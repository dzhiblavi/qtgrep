#include "main_window.h"
#include "ui_main_window.h"

main_window::main_window(QWidget *parent)
    : QMainWindow(parent)
    , status(READY)
    , ui(new Ui::main_window)
    , gtask(nullptr)
    , tpool(8) {
    ui->setupUi(this);

    QCommonStyle style;
    ui->actionScanDirectory->setIcon(style.standardIcon(QCommonStyle::SP_DialogOpenButton));
    update_view();

    connect(ui->searchlineEdit, &QLineEdit::textChanged, this, [this](QString new_dir) {
        if (new_dir.startsWith("~/")) {
            new_dir.replace(0, 1, QDir::homePath());
            ui->searchlineEdit->setText(new_dir);
        }

        QPalette palette = ui->searchlineEdit->palette();
        if (QDir(new_dir).exists() || QFile(new_dir).exists()) {
            palette.setColor(ui->searchlineEdit->foregroundRole(), Qt::black);
        } else {
            palette.setColor(ui->searchlineEdit->foregroundRole(), Qt::red);
        }
        ui->searchlineEdit->setPalette(palette);
    });

    connect(ui->actionScanDirectory, &QAction::triggered, this, [this]() {
        QString path = QFileDialog::getExistingDirectory(this, "Select Directory",
                                                         QString(),
                                                         QFileDialog::ShowDirsOnly
                                                       | QFileDialog::DontResolveSymlinks);
        ui->searchlineEdit->setText(path);
    });

    connect(ui->substringClearButton, &QPushButton::clicked, this, [this] {
        ui->substringLineEdit->clear();
    });

    connect(ui->searchClearButton, &QPushButton::clicked, this, [this] {
        ui->searchlineEdit->clear();
    });

    connect(ui->resultClearButton, &QPushButton::clicked, this, [this] {
        ui->resultTextBrowser->clear();
    });

    connect(ui->cancelButton, &QPushButton::clicked, this, [this] {
        if (gtask) {
            tpool.abort();
            gtask->cancel();
            update_view();
        }
    });

    connect(ui->searchButton, &QPushButton::clicked, this, [this] {
        QString substr = ui->substringLineEdit->text();
        QString dir = ui->searchlineEdit->text();

        if (QDir(dir).exists() || QFile(dir).exists()) {
            if (gtask) {
                tpool.abort();
                gtask->cancel();
            }
            ui->resultTextBrowser->clear();

            gtask = grep_task::create(dir, substr, tpool);
            tpool.enqueue(gtask);
        } else {
            ui->logTextBrowser->append("No such file or directory");
        }
    });

    timer.setInterval(50);
    timer.start();
    connect(&timer, &QTimer::timeout, this, [this] {
        update_ui();
        update_view();
    });
}

void main_window::update_view() {
    switch (status) {
        case READY:
            ui->progressBar->hide();
            ui->searchLabel->hide();
            ui->readyLabel->show();
            break;
        case SFILES:
            ui->progressBar->hide();
            ui->readyLabel->hide();
            ui->searchLabel->show();
            break;
        case SEARCH:
            ui->readyLabel->hide();
            ui->searchLabel->hide();
            ui->progressBar->show();
            break;
    }
}

void main_window::update_ui() {
    if (!gtask || gtask->is_cancelled() || gtask->finished()) {
        status = READY;
        ui->poolLoadN->display(0);
    } else {
        auto res = gtask->get_result(21);
        gtask->clear_result();
        auto fail = gtask->get_failure_logs(21);
        gtask->clear_failure_logs();

        ui->poolLoadN->display((int)tpool.queue_size());
        if (gtask->found_all()) {
            ui->progressBar->setRange(0, (int)gtask->total_files());
            ui->progressBar->setValue((int)gtask->completed_files());
            status = SEARCH;
        } else {
            status = SFILES;
            ui->progressBar->setValue(0);
        }

        if (res.empty()) {
            return;
        }

        QString rappended;
        for (size_t i = 0; i < std::min((size_t)20, res.size()); ++i) {
            rappended += res[i];
        }
        if (res.size() > 20) {
            rappended += "...\n";
        }
        ui->resultTextBrowser->insertPlainText(rappended);
        QScrollBar *sb = ui->resultTextBrowser->verticalScrollBar();
        sb->setValue(sb->maximum());

        if (fail.empty()) {
            return;
        }

        QString fappended;
        for (size_t i = 0; i < std::min((size_t)20, fail.size()); ++i) {
            fappended += fail[i] + "\n";
        }
        if (fail.size() > 20) {
            fappended += "...\n";
        }

        ui->logTextBrowser->insertPlainText(fappended);
        if (gtask->critical_errors()) {
            ui->logTextBrowser->insertPlainText("CRITICAL : "
                                           + QString::number(gtask->critical_errors()) + "\n");
            gtask->reset_critical();
        }
        QScrollBar *sb_log = ui->logTextBrowser->verticalScrollBar();
        sb_log->setValue(sb_log->maximum());
    }
}

main_window::~main_window() {
    delete ui;
}
