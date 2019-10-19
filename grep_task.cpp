#include "grep_task.h"

grep_task::grep_task(QString path, QString substr,
                     std::function<void(std::shared_ptr<task>)> const& enqueue,
                     grep_task* parent)
    : path_(path)
    , substr_(substr)
    , parent_(parent ? parent : this)
    , enq_(enqueue)
{}

void grep_task::run()
{
    if (path_ == QString())
    {
        return;
    }
    {
        std::unique_lock<std::mutex> lg(parent_->m_);
        if (parent_->used_.find(path_) != parent_->used_.end())
        {
            return;
        }
        parent_->used_.insert(path_);
    }

    QDir dir(path_);
    if (dir.exists())
    {
        QDirIterator it(path_,
                        QDir::Dirs
                      | QDir::Files
                      | QDir::NoSymLinks
                      | QDir::NoDotAndDotDot
                      | QDir::Readable,
                        QDirIterator::Subdirectories);

        while (it.hasNext())
        {
            if (is_cancelled())
            {
                return;
            }
            it.next();

            enq_(std::make_shared<grep_task>(it.filePath(), substr_, enq_,
                                             parent_ == this ? this : parent_));
        }
        return;
    }

    QFile file(path_);
    file.open(QFile::ReadOnly | QFile::Text);
    while (!file.atEnd())
    {
        if (is_cancelled())
        {
            return;
        }
        QByteArray line = file.readLine();

        if (line.indexOf(substr_) != -1)
        {
            std::unique_lock<std::mutex> lg(parent_->m_);
            parent_->res_.push_back(path_ + ":" + line);
        }
    }
}

std::vector<QString> grep_task::get_result() const
{
    std::unique_lock<std::mutex> lg(m_);
    return res_;
}

void grep_task::clear_result()
{
    std::unique_lock<std::mutex> lg(m_);
    res_.clear();
}
