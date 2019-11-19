#include "dataviewdialog.h"
#include "ui_dataviewdialog.h"

DataViewDialog::DataViewDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DataViewDialog)
{
    ui->setupUi(this);

    connect(ui->closePushButton,SIGNAL(clicked()),this,SLOT(close()));
}

DataViewDialog::~DataViewDialog()
{
    delete ui;
}
void DataViewDialog::SetData(QString name,QString metadata, double x0,double y0,double xp,double yp, double xn, double yn)
{
    ui->nameLineEdit->setText(name);
    ui->metadataLineEdit->setText(metadata);
    ui->ptBeforeXLineEdit->setText(QString::number(xp));
    ui->ptBeforeYLineEdit->setText(QString::number(yp));
    ui->ptSelectedXLineEdit->setText(QString::number(x0));
    ui->ptSelectedYLineEdit->setText(QString::number(y0));
    ui->ptAfterXLineEdit->setText(QString::number(xn));
    ui->ptAfterYLineEdit->setText(QString::number(yn));
}
