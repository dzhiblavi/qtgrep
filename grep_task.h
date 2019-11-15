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
#include "thread_pool.h"

class grep_task : public task {
    friend class file_grep_subtask;

    template <typename F>
    void iterate_over_directory_(F&& f);
    void enqueue_(std::shared_ptr<task> tsk);

    QString path_;
    QString substr_;

    int total_;
    bool found_all_;
    std::atomic_bool cancel_;
    std::atomic_int complete_;

    mutable std::mutex m_;
    std::vector<QString> res_;

public:
    grep_task(QString path, QString substr, thread_pool& tp);

    std::vector<QString> get_result() const;
    std::vector<QString> get_result(size_t count) const;
    void clear_result();

    void cancel();
    bool is_cancelled() const noexcept;

    QString search_path() const noexcept;
    QString substring() const noexcept;

    void push_result(QString line);

    int total_files() const noexcept;
    int completed_files() const noexcept;
    bool found_all() const noexcept;

    void run() override;
    void prepare() override;
};

class file_grep_subtask : public task {
public:
    file_grep_subtask(QString path, grep_task* parent, thread_pool& tp);

    void run() override;

private:
    QString path_;
    grep_task* parent_;
};

#endif // GREP_TASK_H
