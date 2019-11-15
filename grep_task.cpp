#include "grep_task.h"

#define FILE_OPEN_FAILURE_ENABLED
#define FGREP_BATCH_SIZE 256
#define MAX_LINE_LEN_ALLOWED 32

grep_task::grep_task(QString path, QString substr, thread_pool& tp)
    : task(tp)
    , path_(std::move(path))
    , substr_(std::move(substr))
    , total_(0)
    , found_all_(false)
    , cancel_(false)
    , complete_(0)
{}

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

void grep_task::run() {
    QString path = path_;
    QDir dir(path_);
    if (dir.exists()) {
        iterate_over_directory_([this](QString const& s) {
            enqueue_(std::make_shared<file_grep_subtask>(s, this, tp_));
            ++total_;
        });
    } else {
        enqueue_(std::make_shared<file_grep_subtask>(path_, this, tp_));
        ++total_;
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

void grep_task::prepare() {
    cancel_.store(false);
    total_ = complete_ = 0;
    found_all_ = false;
    res_.clear();
}

std::vector<QString> grep_task::get_result() const {
    std::unique_lock<std::mutex> lg(m_);
    return res_;
}

std::vector<QString> grep_task::get_result(size_t count) const {
    std::unique_lock<std::mutex> lg(m_);
    return std::vector<QString>(res_.begin(), count > res_.size() ? res_.end() : res_.begin() + count);
}

void grep_task::clear_result() {
    std::unique_lock<std::mutex> lg(m_);
    res_.clear();
}

file_grep_subtask::file_grep_subtask(QString path, grep_task* parent, thread_pool& tp)
    : task(tp)
    , path_(std::move(path))
    , parent_(parent)
{}

void file_grep_subtask::patch_result() {
    size_t i = 0;
    while (i < result_.size()) {
        size_t sz = std::min(result_.size() - i, (size_t)FGREP_BATCH_SIZE);
        {
            std::unique_lock<std::mutex> lg(parent_->m_);
            parent_->res_.reserve(parent_->res_.size() + sz);
            parent_->res_.insert(parent_->res_.end(),
                                    result_.begin() + i,
                                    result_.begin() + i + sz);
        }
        i += sz;
    }
    result_.clear();
}

QString file_grep_subtask::construct_result(QString const& line, int index, int lineid) {
    return path_ + "::" + QString::number(index) + "::" + QString::number(lineid) + "::" + line + "\n";
//    return QString::number(index) + "::" + QString::number(lineid) + "::<pre>" + line + "</pre><br>";
}

void file_grep_subtask::add_result(QString line, int index, int lineid) {
    QString res;
    if (line.size() < MAX_LINE_LEN_ALLOWED) {
        if (parent_->substr_.size()) {
//            line.insert(index + parent_->substr_.size(), QString("</font><pre>"));
//            line.insert(index, QString("</pre><font color=\"Red\">"));
        }
        line = line.left(line.size() - 1);
        res = construct_result(line, index, lineid);
    } else if (parent_->substr_.size() > MAX_LINE_LEN_ALLOWED) {
//        res = construct_result(QString("</pre><font color=\"Red\">")
//                             + line.mid(index, MAX_LINE_LEN_ALLOWED)
//                             + QString("</font><pre>"), index, lineid);

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
            pstr = push.toString();
        } else {
            pstr = push.toString();
        }
//        pstr = push.toString();
        if (parent_->substr_.size()) {
//            pstr.insert(index + parent_->substr_.size(), QString("</font><pre>"));
//            pstr.insert(index, QString("</pre><font color=\"Red\">"));
        }
        res = construct_result(pstr, def_ind, lineid);
    }
    result_.push_back(res);
}

void file_grep_subtask::run() {
    QFile file(path_);
    if (file.exists() && file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream in(&file);
        int ln = 1;
        while (!in.atEnd()) {
            if (parent_->is_cancelled()) {
                break;
            }
            QString line = in.readLine();
            if (in.status() == QTextStream::Status::ReadCorruptData) {
                break;
            }
            int index = line.indexOf(parent_->substr_);
            if (index != -1) {
                add_result(line, index, ln);
            }
            ++ln;
        }
        file.close();
    } else {
#ifdef FILE_OPEN_FAILURE_ENABLED
        result_.push_back(path_ + "::" + "FAILED TO OPEN FILE\n");
//        result_.push_back(path_ + "::" + "<font color=\"Blue\">FAILED TO OPEN FILE</font><br>");
#endif
    }
    patch_result();
    ++parent_->complete_;
}
