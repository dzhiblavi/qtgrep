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
        ui->resultTextEdit->clear();
    });

    connect(ui->cancelButton, &QPushButton::clicked, this, [this] {
        if (gtask) {
            gtask->cancel();
        }
    });

    connect(ui->searchButton, &QPushButton::clicked, this, [this] {
        QString substr = ui->substringLineEdit->text();
        QString dir = ui->searchlineEdit->text();

        if (QDir(dir).exists() || QFile(dir).exists()) {
            if (gtask) {
                gtask->cancel();
            }
            ui->resultTextEdit->clear();
            gtask = std::make_shared<grep_task>(dir, substr, tpool);
            tpool.enqueue(gtask);
        } else {
            ui->logTextEdit->append("No such file or directory");
        }
    });

    timer.setInterval(250);
    timer.start();
    connect(&timer, &QTimer::timeout, this, [this] {
        update_ui();
    });
}

void main_window::update_ui() {
    if (!gtask || gtask->is_cancelled()) {
        status = READY;
        ui->poolLoadN->display(0);
    } else {
        auto res = gtask->get_result(101);
        gtask->clear_result();
        auto fail = gtask->get_failure_logs(101);
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
        for (size_t i = 0; i < std::min((size_t)100, res.size()); ++i) {
            rappended += res[i];
        }
        if (res.size() > 100) {
            rappended += "...";
        }

        QString fappended;
        for (size_t i = 0; i < std::min((size_t)100, fail.size()); ++i) {
            fappended += fail[i];
        }
        if (fail.size() > 100) {
            fappended += "...";
        }

        ui->resultTextEdit->insertPlainText(rappended);
        ui->logTextEdit->insertPlainText(fappended);
    }

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

main_window::~main_window() {
    delete ui;
}

