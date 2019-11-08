#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <iostream>
#include <algorithm>

#include <QMainWindow>
#include <QTimer>
#include <QCommonStyle>
#include <QFileDialog>
#include <QColorDialog>

#include "thread_pool.h"
#include "grep_task.h"

QT_BEGIN_NAMESPACE
namespace Ui { class main_window; }
QT_END_NAMESPACE

class main_window : public QMainWindow {
    Q_OBJECT

public:
    main_window(QWidget* parent = nullptr);
    ~main_window();

private:
    enum STATUS {
        READY,
        SFILES,
        SEARCH
    } status;

    void update_ui();

    Ui::main_window* ui;
    QTimer timer;

    std::shared_ptr<grep_task> gtask;
    thread_pool tpool;
};
#endif // MAIN_WINDOW_H
