#include "main_window.h"
#include "ui_main_window.h"

main_window::main_window(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::main_window)
    , gtask(nullptr)
    , ftask(nullptr)
    , tpool(8)
{
    ui->setupUi(this);

    QCommonStyle style;
    ui->actionScanDirectory->setIcon(style.standardIcon(QCommonStyle::SP_DialogOpenButton));

    connect(ui->pushButton, &QPushButton::clicked, this, [this]
    {
        if (gtask)
            gtask->cancel();
    });

    connect(ui->pushButton_2, &QPushButton::clicked, this, [this]
    {
        ui->textEdit->clear();
    });

    connect(ui->actionScanDirectory, &QAction::triggered, this, [this]()
    {
        QString path = QFileDialog::getExistingDirectory(this, "Select Directory for grepping",
                                                         QString(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

        QString substr = ui->lineEdit->text();

        if (gtask)
            gtask->cancel();

        if (path == QString())
        {
            gtask = nullptr;
            return;
        }

        ui->textEdit->clear();
        gtask = std::shared_ptr<grep_task>(new grep_task(path, substr,
                                           [&](std::shared_ptr<task> t) { tpool.enqueue(t); }));
        tpool.enqueue(gtask);
    });

    connect(ui->lineEdit_2, &QLineEdit::textChanged, this, [this](QString const& new_text)
    {
        if (ftask)
            ftask->cancel();

        uint64_t numm = new_text.toULongLong();

        ftask = std::shared_ptr<factorization_task>(new factorization_task(numm));
        tpool.enqueue(ftask);
    });

    timer.setInterval(250);
    timer.start();
    connect(&timer, &QTimer::timeout, this, [this]
    {
        show_result_();

        if (!ftask)
            return;

        auto res = ftask->get_result();

        QString text;
        for (auto x : res.second)
        {
            text += QString("%1 ").arg(x);
        }

        if (res.first)
        {
            text += "...";
        }
        text += "\n";

        ui->lineEdit_3->setText(text);

        if (!gtask)
            return;
    });
}

void main_window::show_result_() {
    if (!gtask)
        return;

    auto res = gtask->get_result();
    gtask->clear_result();

    ui->lcdNumber_2->display((int)tpool.queue_size());

    if (res.empty())
    {
        return;
    }

    QString appended;
    for (size_t i = 0; i < std::min(res.size(), size_t(1000)); ++i)
    {
        appended += res[i];
    }
    if (res.size() > 1000)
        appended += "...";
    ui->textEdit->append(appended);
}

main_window::~main_window()
{
    delete ui;
}

