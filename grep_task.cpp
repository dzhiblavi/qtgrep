#include "grep_task.h"

grep_task::grep_task(QString path, QString substr,
                     std::function<void(std::shared_ptr<task>)> const& enqueue)
    : path_(path)
    , substr_(substr)
    , enq_(enqueue)
{}

void grep_task::run() {
    QString path = path_;

    QDir dir(path_);
    if (dir.exists()) {
        QDirIterator it(path_,
                        QDir::Files
                      | QDir::AllEntries
                      | QDir::NoSymLinks
                      | QDir::NoDotAndDotDot
                      | QDir::Readable
                      , QDirIterator::Subdirectories);

        while (it.hasNext()) {
            if (is_cancelled())
                return;

            it.next();
            enq_(std::make_shared<file_grep_subtask>(it.filePath(), this));
            ++total_;
        }
    }
}

int grep_task::total_files() const {
    return total_;
}

int grep_task::completed_files() const {
    return complete_.load();
}

void grep_task::prepare() {
    total_ = complete_ = 0;
    res_.clear();
    canc_.store(false);
}

file_grep_subtask::file_grep_subtask(QString path, grep_task* parent)
    : path_(path)
    , parent_(parent)
{}

void file_grep_subtask::run() {
    QFile file(path_);
    if (file.exists()) {
        file.open(QFile::ReadOnly
                | QFile::Text);
        while (!file.atEnd()) {
            if (parent_->is_cancelled())
                return;

            QByteArray line = file.readLine();
            if (line.indexOf(parent_->substr_) != -1) {
                std::unique_lock<std::mutex> lg(parent_->m_);
                parent_->res_.push_back(path_ + ":" + line);
            }
        }
    }
    ++parent_->complete_;
}

std::vector<QString> grep_task::get_result() const {
    std::unique_lock<std::mutex> lg(m_);
    return res_;
}

void grep_task::clear_result() {
    std::unique_lock<std::mutex> lg(m_);
    res_.clear();
}
