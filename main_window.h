#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <iostream>

#include <QMainWindow>
#include <QTimer>
#include <QCommonStyle>
#include <QFileDialog>

#include "thread_pool.h"
#include "factorization_task.h"
#include "grep_task.h"

QT_BEGIN_NAMESPACE
namespace Ui { class main_window; }
QT_END_NAMESPACE

class main_window : public QMainWindow
{
    Q_OBJECT

public:
    main_window(QWidget* parent = nullptr);
    ~main_window();

private:
    void show_result_();

    Ui::main_window* ui;
    QTimer timer;

    std::shared_ptr<grep_task> gtask;
    std::shared_ptr<factorization_task> ftask;
    thread_pool tpool;
};
#endif // MAIN_WINDOW_H
