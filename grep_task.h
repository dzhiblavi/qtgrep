#ifndef GREP_TASK_H
#define GREP_TASK_H

#include <set>
#include <iostream>
#include <stack>

#include <QString>
#include <QFile>
#include <QDirIterator>
#include <QDir>
#include <QTextStream>

#include "task.h"

class grep_task : public task {
    friend class file_grep_subtask;

public:
    grep_task(QString path, QString substr,
              std::function<void(std::shared_ptr<task>)> const& enqueue);

    std::vector<QString> get_result() const;
    std::vector<QString> get_result(size_t count) const;
    void clear_result();

    int total_files() const;
    int completed_files() const;
    bool found_all() const;

private:
    void run() override;
    void prepare() override;

    template <typename F>
    void iterate_over_directory_(F&& f);

    QString path_;
    QString substr_;

    int total_;
    std::atomic_int complete_;
    bool found_all_;

    mutable std::mutex m_;

    std::vector<QString> res_;
    std::function<void(std::shared_ptr<task>)> enq_;
};

class file_grep_subtask : public task {
public:
    file_grep_subtask(QString path, grep_task* parent);

private:
    void run() override;

    QString path_;
    grep_task* parent_;
};

#endif // GREP_TASK_H
