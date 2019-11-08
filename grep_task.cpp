#include "grep_task.h"

grep_task::grep_task(QString path, QString substr,
                     std::function<void(std::shared_ptr<task>)> const& enqueue)
    : path_(path)
    , substr_(substr)
    , total_(0)
    , complete_(0)
    , found_all_(false)
    , enq_(enqueue)
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

void grep_task::run() {
    QString path = path_;
    QDir dir(path_);
    if (dir.exists()) {
        iterate_over_directory_([this](QString const& s) {
            enq_(std::make_shared<file_grep_subtask>(s, this));
            ++total_;
        });
    } else {
        enq_(std::make_shared<file_grep_subtask>(path_, this));
        ++total_;
    }
    found_all_ = true;
}

int grep_task::total_files() const {
    return total_;
}

int grep_task::completed_files() const {
    return complete_.load();
}

bool grep_task::found_all() const {
    return found_all_;
}

void grep_task::prepare() {
    total_ = complete_ = 0;
    found_all_ = false;
    res_.clear();
    canc_.store(false);
}

file_grep_subtask::file_grep_subtask(QString path, grep_task* parent)
    : path_(path)
    , parent_(parent)
{}

void file_grep_subtask::run() {
    QFile file(path_);
    if (file.exists() && file.open(QFile::ReadOnly | QFile::Text)) {
        while (!file.atEnd()) {
            if (parent_->is_cancelled()) {
                return;
            }

            QByteArray line = file.readLine();
            int index = line.indexOf(parent_->substr_);
            if (index != -1) {
                line.insert(index + parent_->substr_.size(), QString("</font><br>"));
                std::unique_lock<std::mutex> lg(parent_->m_);
                line.insert(index, QString("<font color=\"Red\">"));
                parent_->res_.push_back(path_ + ":" + line);
            }
        }
    } else {
        std::unique_lock<std::mutex> lg(parent_->m_);
        parent_->res_.push_back(path_ + ":"
                                + "<font color=\"Blue\">FAILED TO OPEN FILE</font><br>");
    }
    ++parent_->complete_;
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
