#ifndef GREP_TASK_H
#define GREP_TASK_H

#include <set>
#include <iostream>

#include <QString>
#include <QFile>
#include <QDirIterator>
#include <QDir>

#include "task.h"

class grep_task : public task
{
    friend void run_grep(QString path, grep_task* gt);

public:
    grep_task(QString path, QString substr,
              std::function<void(std::shared_ptr<task>)> const& enqueue);

    std::vector<QString> get_result() const;
    void clear_result();

private:
    void run() override;
    void prepare() override;

    QString path_;
    QString substr_;

    mutable std::mutex m_;

    std::vector<QString> res_;
    std::set<QString> used_;
    std::function<void(std::shared_ptr<task>)> enq_;
};

class grep_subtask : public task
{
public:
    grep_subtask(QString path, grep_task* parent);

private:
    void run() override;
    void prepare() override;

    QString path_;
    grep_task* parent;
};

#endif // GREP_TASK_H
