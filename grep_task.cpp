#include "grep_task.h"

#define FILE_OPEN_FAILURE_ENABLED
#define FGREP_BATCH_SIZE 20
#define MAX_LINE_LEN_ALLOWED 32
#define MAX_QUEUE 300000
#define MAX_RES_SIZE 20
#define MAX_LOG_SIZE 20

grep_task::grep_task(QString path, QString substr,
                     thread_pool& tp) noexcept
    : task(tp)
    , path_(std::move(path))
    , substr_(std::move(substr))
    , total_(0)
    , found_all_(false)
    , cancel_(false)
    , complete_(0)
    , critical_(0)
    , self_pointer(nullptr)
{}

std::shared_ptr<grep_task> grep_task::create(QString path, QString substr, thread_pool& tp) noexcept {
    auto p = new grep_task(path, substr, tp);
    std::shared_ptr<grep_task> ptr(nullptr);
    try {
        ptr = std::shared_ptr<grep_task>(new grep_task(path, substr, tp));
    } catch (...) {
        delete p;
        throw;
    }
    ptr->set_self_ptr_(ptr);
    return ptr;
}

void grep_task::set_self_ptr_(const std::shared_ptr<grep_task> &ptr) {
    self_pointer = ptr;
}

template <typename F>
void grep_task::iterate_over_directory_(F&& f) {
    QDirIterator it(path_,
                    QDir::Files
                  | QDir::NoSymLinks
                  | QDir::NoDotAndDotDot
                  | QDir::Readable
                  , QDirIterator::Subdirectories);

    while (it.hasNext()) {
        if (is_cancelled()) {
            return;
        }
        it.next();
        f(it.filePath());
    }
}

void grep_task::enqueue_(std::shared_ptr<task> const& tsk) {
    tp_.enqueue(tsk);
}

void grep_task::run() noexcept {
    try {
        QString path = path_;
        QDir dir(path_);
        if (dir.exists()) {
            iterate_over_directory_([this](QString const& s) {
                while (tp_.queue_size() >= MAX_QUEUE && !is_cancelled());
                if (is_cancelled()) {
                    return;
                }
                enqueue_(std::make_shared<file_grep_subtask>(s, self_pointer, tp_));
                ++total_;
            });
        } else {
            while (tp_.queue_size() > MAX_QUEUE && !is_cancelled());
            if (is_cancelled()) {
                return;
            }
            enqueue_(std::make_shared<file_grep_subtask>(path_, self_pointer, tp_));
            ++total_;
        }
    } catch (...) {
        std::unique_lock<std::mutex> lg(fm_);
        try {
            fail_.emplace_back("AN UNKNOWN ERROR OCCURED\n");
        } catch (...) {
            ++critical_;
            // pass
        }

        // pass
    }

    found_all_ = true;
}

void grep_task::cancel() {
    cancel_.store(true);
}

bool grep_task::is_cancelled() const noexcept {
    return cancel_.load();
}

QString grep_task::search_path() const noexcept {
    return path_;
}

QString grep_task::substring() const noexcept {
    return substr_;
}

int grep_task::total_files() const noexcept {
    return total_;
}

int grep_task::completed_files() const noexcept {
    return complete_.load();
}

bool grep_task::found_all() const noexcept {
    return found_all_;
}

bool grep_task::finished() const noexcept {
    return found_all() && completed_files() == total_files();
}

void grep_task::prepare() noexcept {
    cancel_.store(false);
    total_ = complete_ = 0;
    found_all_ = false;
    critical_.store(0);
    result_.clear();
    fail_.clear();
}

std::vector<QString> grep_task::get_result() const {
    std::unique_lock<std::mutex> lg(rm_);
    return result_;
}

std::vector<QString> grep_task::get_result(size_t count) const {
    std::unique_lock<std::mutex> lg(rm_);
    return std::vector<QString>(result_.begin(), count > result_.size()
                                ? result_.end() : result_.begin() + (long)count);
}

void grep_task::clear_result() noexcept {
    std::unique_lock<std::mutex> lg(rm_);
    result_.clear();
}

std::vector<QString> grep_task::get_failure_logs() const {
    std::unique_lock<std::mutex> lg(fm_);
    return fail_;
}

std::vector<QString> grep_task::get_failure_logs(size_t count) const {
    std::unique_lock<std::mutex> lg(fm_);
    return std::vector<QString>(fail_.begin(), count > fail_.size()
                                ? fail_.end() : fail_.begin() + (long)count);
}

void grep_task::clear_failure_logs() noexcept {
    std::unique_lock<std::mutex> lg(fm_);
    fail_.clear();
}

int grep_task::critical_errors() const noexcept {
    return critical_.load();
}

void grep_task::reset_critical() noexcept {
    critical_.store(0);
}

file_grep_subtask::file_grep_subtask(QString path, std::shared_ptr<grep_task> parent, thread_pool& tp) noexcept
    : task(tp)
    , path_(std::move(path))
    , parent_(parent)
{}

void file_grep_subtask::patch(std::vector<QString>& src, std::vector<QString>& dst, std::mutex& m) {
    auto it = src.begin();
    while (it < src.end()) {
        if (parent_->is_cancelled()) {
            return;
        }
        size_t sz = (size_t)std::min(src.end() - it, (long)FGREP_BATCH_SIZE);
        {
            std::unique_lock<std::mutex> lg(m);
            dst.reserve(dst.size() + sz);
            dst.insert(dst.end(), it, it + (long)sz);
        }
        it += (long)sz;
    }
    src.clear();
}

QString file_grep_subtask::construct_result(QString const& line, int index, int lineid) {
    return path_ + "::" + QString::number(index) + "::" + QString::number(lineid) + "::" + line + "\n";
}

void file_grep_subtask::add_result(QString line, int index, int lineid) {
    QString res;
    if (line.size() < MAX_LINE_LEN_ALLOWED) {
        if (line.size() && line.back() == '\n') {
            line = line.left(line.size() - 1);
        }
        res = construct_result(line, index, lineid);
    } else if (parent_->substr_.size() > MAX_LINE_LEN_ALLOWED) {
        res = construct_result(line.mid(index, MAX_LINE_LEN_ALLOWED), index, lineid);
    } else {
        int def_ind = index;
        QString pstr;
        QStringRef push(&line);
        if (index > MAX_LINE_LEN_ALLOWED / 2) {
            push = push.right(push.size() - index + MAX_LINE_LEN_ALLOWED / 2);
            index = MAX_LINE_LEN_ALLOWED / 2;
        }
        if (push.size() - index - parent_->substr_.size() > MAX_LINE_LEN_ALLOWED / 2) {
            assert(index <= MAX_LINE_LEN_ALLOWED / 2);
            push = push.left(index + parent_->substr_.size() + MAX_LINE_LEN_ALLOWED / 2);
        }
        pstr = push.toString();
        res = construct_result(pstr, def_ind, lineid);
    }
    result_.push_back(res);
}

void file_grep_subtask::try_add_fail(QString const& str) noexcept {
    if (fail_.size() == MAX_LOG_SIZE) {
        fail_.push_back(path_ + "::TOO MANY ERRORS IN THIS FILE");
        return;
    } else if (fail_.size() > MAX_LOG_SIZE) {
        return;
    }
    try {
        fail_.push_back(path_ + "::" + str);
    } catch (...) {
        ++parent_->critical_;
        // pass
    }
}

bool bad(QString const& line) {
    for (auto it = line.begin(); it != line.end(); ++it) {
        if (!it->isPrint()) {
            return true;
        }
    }
    return false;
}

void file_grep_subtask::run() noexcept {
    try {
        QFile file(path_);
        if (file.exists() && file.open(QFile::ReadOnly | QFile::Text)) {
            QTextStream in(&file);
            int ln = 1;
            while (!in.atEnd() && result_.size() < MAX_RES_SIZE) {
                if (parent_->is_cancelled()) {
                    break;
                }
                QString line = in.readLine();

                if (bad(line) || in.status() == QTextStream::Status::ReadCorruptData) {
                    try_add_fail("Corrupt data or unknown encoding\n");
                    continue;
                }

                int index = line.indexOf(parent_->substr_);
                if (index != -1) {
                    add_result(line, index, ln);
                }
                ++ln;
            }
            if (result_.size() >= MAX_RES_SIZE) {
                try_add_fail("Too large result\n");
            }
        } else {
#ifdef FILE_OPEN_FAILURE_ENABLED
            throw std::runtime_error("Failed to open file\n");
#endif
        }
    } catch (std::runtime_error const& e) {
        try_add_fail(e.what());
        // pass
    } catch (...) {
        try_add_fail("An unknown error occured\n");
    }

    ++parent_->complete_;
    if (parent_->is_cancelled()) {
        return;
    }

    try {
        patch(result_, parent_->result_, parent_->rm_);
    } catch (...) {
        try_add_fail("Failed to push results\n");
        // pass
    }
    try {
        patch(fail_, parent_->fail_, parent_->fm_);
    } catch (...) {
        ++parent_->critical_;
        // pass (
    }
}

void file_grep_subtask::prepare() noexcept {
    result_.clear();
}
