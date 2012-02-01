#include "widget.h"
#include "ui_widget.h"

#include <QDir>
#include <QTextStream>
#include <QStringList>

QStringList stat()
{
    QDir dir("/proc/");
    dir.setFilter(QDir::Dirs | QDir::NoSymLinks);
    dir.setSorting(QDir::Size | QDir::Reversed);

    QFileInfoList list = dir.entryInfoList();
    QStringList result;

    for (int i = 0; i < list.size(); ++i)
    {
        QFileInfo dirInfo = list.at(i);
        QString path = QString("%1/stat").arg(dirInfo.filePath());

        QFile file(path);

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            continue;

        QTextStream in(&file);
        QString data = in.readAll();

        QRegExp re("(\\d+)\\s+\\((.+)\\)\\s+([RSDZTW])\\s+(-?\\d+\\s+){10}(\\d+)");

        if (re.indexIn(data) != -1)
        {
            result << QString("PID: %1    CMD: %2    STATE: %3    UTIME: %4").arg(re.cap(1)).arg(re.cap(2)).arg(re.cap(3)).arg(re.cap(5)).toStdString().c_str();
        }
    }

    return result;
}

void Widget::updateUI()
{
    QStringList a = stat();

    ui->listWidget->clear();
    ui->listWidget->setSortingEnabled(true);

    for (int i = 0; i < a.size(); i++)
    {
        ui->listWidget->addItem(new QListWidgetItem(a.at(i)));
    }
}

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    updateUI();
}

Widget::~Widget()
{
    delete ui;
}

void Widget::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void Widget::on_pushButton_clicked()
{
    updateUI();
}
