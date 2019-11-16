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
    void enqueue_(std::shared_ptr<task> const& tsk);

    QString path_;
    QString substr_;

    int total_;
    bool found_all_;
    std::atomic_bool cancel_;
    std::atomic_int complete_;
    std::atomic_int critical_;

    mutable std::mutex rm_;
    mutable std::mutex fm_;
    std::vector<QString> result_;
    std::vector<QString> fail_;

public:
    grep_task(QString path, QString substr, thread_pool& tp) noexcept;

    std::vector<QString> get_result() const;
    std::vector<QString> get_result(size_t count) const;
    void clear_result() noexcept;

    std::vector<QString> get_failure_logs() const;
    std::vector<QString> get_failure_logs(size_t count) const;
    void clear_failure_logs() noexcept;

    int critical_errors() const noexcept;
    void reset_critical() noexcept;

    void cancel();
    bool is_cancelled() const noexcept;

    QString search_path() const noexcept;
    QString substring() const noexcept;

    int total_files() const noexcept;
    int completed_files() const noexcept;
    bool found_all() const noexcept;
    bool finished() const noexcept;

    void run() noexcept override;
    void prepare() noexcept override;
};

class file_grep_subtask : public task {
public:
    file_grep_subtask(QString path, grep_task* parent, thread_pool& tp) noexcept;
    void run() noexcept override;
    void prepare() noexcept override;

private:
    void try_add_fail(QString const& str) noexcept;
    static void patch(std::vector<QString>&, std::vector<QString>&, std::mutex&);
    void add_result(QString line, int index, int lineid);
    QString construct_result(QString const& line, int index, int lineid);

    QString path_;
    grep_task* parent_;
    std::vector<QString> result_;
    std::vector<QString> fail_;
};

#endif // GREP_TASK_H
