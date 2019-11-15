#include "grep_task.h"

grep_task::grep_task(QString path, QString substr, thread_pool& tp)
    : task(tp)
    , path_(path)
    , substr_(substr)
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
            std::cerr << "I AM CANCELLED" << std::endl;
            return;
        }
        it.next();
        f(it.filePath());
    }
}

void grep_task::enqueue_(std::shared_ptr<task> tsk) {
    tp_.enqueue(tsk);
}

void grep_task::run() {
    QString path = path_;
    QDir dir(path_);
    if (dir.exists()) {
        std::cerr << "STARTED ITERATING" << std::endl;
        iterate_over_directory_([this](QString const& s) {
            enqueue_(std::make_shared<file_grep_subtask>(s, this, tp_));
            ++total_;
        });
        std::cerr << "FINISHED ITERATING" << std::endl;
    } else {
        enqueue_(std::make_shared<file_grep_subtask>(path_, this, tp_));
        ++total_;
    }
    found_all_ = true;
    std::cerr << "EXITING GREP RUN" << std::endl;
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

void grep_task::push_result(QString line) {
    std::unique_lock<std::mutex> lg(m_);
    res_.push_back(line);
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
    return std::vector(res_.begin(), count > res_.size() ? res_.end() : res_.begin() + count);
}

void grep_task::clear_result() {
    std::unique_lock<std::mutex> lg(m_);
    res_.clear();
}

file_grep_subtask::file_grep_subtask(QString path, grep_task* parent, thread_pool& tp)
    : task(tp)
    , path_(path)
    , parent_(parent)
{}

//bool is_binary(QByteArray const& ba) {
//    for (auto const& c : ba) {
//        if (!QChar(c).isPrint()) {
//            return true;
//        }
//    }
//    return false;
//}

void file_grep_subtask::run() {
    QFile file(path_);
    if (file.exists() && file.open(QFile::ReadOnly | QFile::Text)) {
        while (!file.atEnd()) {
            if (parent_->is_cancelled()) {
                std::cerr << "FILE GREP CANCELLED" << std::endl;
                break;
            }

            QByteArray line = file.readLine();
            int index = line.indexOf(parent_->substr_);

//            if (index != -1) {
//                line.insert(index + parent_->substr_.size(), QString("</font><br>"));
//                line.insert(index, QString("<font color=\"Red\">"));
//                parent_->push_result(path_ + ":" + line);
//            }
        }
        file.close();
    } else {
        parent_->push_result(path_ + ":" + "<font color=\"Blue\">FAILED TO OPEN FILE</font><br>");
    }
    ++parent_->complete_;
    std::cerr << "TOTALLY EXITED RUN" << std::endl;
}
