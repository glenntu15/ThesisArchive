#include "datainfodialog.h"
#include "ui_datainfodialog.h"

#ifdef MINGW
#pragma GCC diagnostic ignored "-Wformat"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#else
#pragma warning(disable:4018)
#pragma warning(disable:4100)
#define _CRT_SECURE_NO_WARNINGS
#endif

DataInfoDialog::DataInfoDialog(QWidget *parent) :
    QDialog(parent), ui(new Ui::DataInfoDialog), dm(DataManager::getInstance())
{
    ui->setupUi(this);

    QHBoxLayout *layout = new QHBoxLayout(this);

    gridLayout = new QGridLayout;
    gridGroupBox = new QGroupBox(tr(""));
    gridGroupBox->setLayout(gridLayout);
    gridGroupBox->setMinimumHeight(250);

    horizontalGroupBox = new QGroupBox(tr("Chart Type Selection"));
    horizontalGroupBox->setLayout(layout);


    linenamebox = new QLineEdit();
    linenamebox->setReadOnly(true);
    linenamebox->setAlignment(Qt::AlignLeft);
    linenamebox->setMaxLength(70);

    gridLayout->addWidget(linenamebox,0,0);
    QPushButton *btn = new QPushButton();
    btn->setText("Done");
    gridLayout->addWidget(btn,1,0);

    setLayout(gridLayout);

    connect(btn,SIGNAL(clicked()),this,SLOT(theend()));
}

DataInfoDialog::~DataInfoDialog()
{
    delete ui;
}

void DataInfoDialog::SetData(QString name, double x, double y)
{
    linenamebox->setText(name);
}
void DataInfoDialog::theend()
{
    emit QDialog::finished(r);
    QDialog::done(r);
    return;
}
