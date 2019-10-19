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
public:
    grep_task(QString path, QString substr,
              std::function<void(std::shared_ptr<task>)> const& enqueue,
              grep_task* parent = nullptr);

    std::vector<QString> get_result() const;
    void clear_result();

private:
    void run() override;

    QString path_;
    QString substr_;
    grep_task* parent_;

    mutable std::mutex m_;
    std::vector<QString> res_;
    std::set<QString> used_;
    std::function<void(std::shared_ptr<task>)> enq_;
};

#endif // GREP_TASK_H
