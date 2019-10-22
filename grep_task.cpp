#include "grep_task.h"

grep_task::grep_task(QString path, QString substr,
                     std::function<void(std::shared_ptr<task>)> const& enqueue)
    : path_(path)
    , substr_(substr)
    , enq_(enqueue)
{}

grep_subtask::grep_subtask(QString path, grep_task* parent)
    : path_(path)
    , parent(parent)
{}

void run_grep(QString path, grep_task* gt)
{
    QDir dir(path);
    if (dir.exists())
    {
        QDirIterator it(path,
                        QDir::Dirs
                      | QDir::AllEntries
                      | QDir::Files
                      | QDir::NoSymLinks
                      | QDir::NoDotAndDotDot
                      | QDir::Readable,
                        QDirIterator::Subdirectories);

        while (it.hasNext())
        {
            if (gt->is_cancelled())
            {
                return;
            }
            it.next();

            std::unique_lock<std::mutex> lg(gt->m_);
            if (gt->used_.find(it.filePath()) != gt->used_.end())
            {
                continue;
            }
            gt->used_.insert(it.filePath());
            gt->enq_(std::make_shared<grep_subtask>(it.filePath(), gt));
        }
        return;
    }

    QFile file(path);
    if (file.exists())
    {
        file.open(QFile::ReadOnly | QFile::Text);
        while (!file.atEnd())
        {
            if (gt->is_cancelled())
            {
                return;
            }
            QByteArray line = file.readLine();

            if (line.indexOf(gt->substr_) != -1)
            {
                std::unique_lock<std::mutex> lg(gt->m_);
                gt->res_.push_back(path + ":" + line);
            }
        }
    }
}

void grep_task::run()
{
    run_grep(path_, this);
}

void grep_task::prepare()
{
    used_.clear();
    res_.clear();
    canc_.store(false);
}

void grep_subtask::run()
{
    run_grep(path_, parent);
}

void grep_subtask::prepare()
{}

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
