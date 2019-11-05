#include "main_window.h"
#include "ui_main_window.h"

main_window::main_window(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::main_window)
    , gtask(nullptr)
    , tpool(8) {
    ui->setupUi(this);

    QCommonStyle style;
    ui->actionScanDirectory->setIcon(style.standardIcon(QCommonStyle::SP_DialogOpenButton));
    ui->progressBar->setValue(0);
    ui->progressBar->setTextVisible(true);

    connect(ui->pushButton, &QPushButton::clicked, this, [this] {
        if (gtask)
            gtask->cancel();
    });

    connect(ui->pushButton_2, &QPushButton::clicked, this, [this] {
        ui->textEdit->clear();
    });

    connect(ui->actionScanDirectory, &QAction::triggered, this, [this]() {
        QString path = QFileDialog::getExistingDirectory(this, "Select Directory for grepping",
                                                         QString(), QFileDialog::DontResolveSymlinks);

        QString substr = ui->lineEdit->text();

        if (gtask)
            gtask->cancel();

        if (path == QString()) {
            gtask = nullptr;
            return;
        }

        ui->textEdit->clear();
        gtask = std::make_shared<grep_task>(path, substr, [&](std::shared_ptr<task> t) { tpool.enqueue(t); });
        tpool.enqueue(gtask);
    });

    timer.setInterval(250);
    timer.start();
    connect(&timer, &QTimer::timeout, this, [this] {
        if (!gtask)
            return;

        auto res = gtask->get_result();
        gtask->clear_result();

        ui->lcdNumber_2->display((int)tpool.queue_size());
        if (gtask->total_files() != gtask->completed_files()) {
            ui->progressBar->setRange(0, (int)gtask->total_files());
            ui->progressBar->setValue((int)gtask->completed_files());
        } else {
            ui->progressBar->setValue(0);
        }

        if (res.empty())
            return;

        QString appended;
        for (size_t i = 0; i < std::min(res.size(), size_t(1000)); ++i) {
            appended += res[i];
        }
        if (res.size() > 1000)
            appended += "...";

        ui->textEdit->append(appended);
    });
}

main_window::~main_window() {
    delete ui;
}

