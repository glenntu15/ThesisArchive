#include "axisspecdialog.h"
#include "ui_axisspecdialog.h"

AxisSpecDialog::AxisSpecDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AxisSpecDialog)
{
    ui->setupUi(this);
    //setModal(false);

    connect(ui->buttonBox, SIGNAL(clicked(int)), this, SLOT(done(int)));

    //ui->firstValueLineEdit->setText("mytext2");
    //ui->axisLabelLineEdit->setText("mytext");
}

AxisSpecDialog::~AxisSpecDialog()
{
    delete ui;
}

void AxisSpecDialog::SetData(AxisSpecData * d)
{
    data = d;
    axislabel =data->axisLabel;
    firstv = data->firstv;
    deltav = data->deltav;
    axislen = data->axislen;
    ui->axisLabelLineEdit->setText(axislabel);
    ui->firstValueLineEdit->setText(QString::number(firstv));
    ui->deltaValueLineEdit->setText(QString::number(deltav));
    ui->numValueLineEdit->setText(QString::number(axislen));
    if (data->isParallelCoords)
        ui->label_warning->setText("Range (deltav * axlen) should not exceed 1.0");

}
void AxisSpecDialog::done(int r)
{
    //bool success = true;
    QString value;
    //int dbgv = QDialog::Accepted;
#define OK_PRESSED 1
#define CANCEL_PRESSED 0
    if(OK_PRESSED == r)  // ok was pressed
    {
        if (ui->axisLabelLineEdit->text().size() > 0) {
            data->axisLabel = ui->axisLabelLineEdit->text();
        }
        //int size = ui->firstValueLineEdit->text().size();
        if(ui->firstValueLineEdit->text().size() > 0) {  // validate the data somehow
            value = ui->firstValueLineEdit->text();
            data->firstv = value.toDouble();
        }
        //size = ui->deltaValueLineEdit->text().size();
        if(ui->deltaValueLineEdit->text().size() > 0) {  // validate the data somehow
            value = ui->deltaValueLineEdit->text();
            data->deltav = value.toDouble();
        }
        //size = ui->numValueLineEdit->text().size();
        if(ui->numValueLineEdit->text().size() > 0) {  // validate the data somehow
            value = ui->numValueLineEdit->text();
            data->axislen = value.toInt();
        }

        data->range = (static_cast<double>(data->axislen) * data->deltav) + data->firstv;
        emit QDialog::finished(r);
        close();
        return;
    }
    else    // cancel, close or exc was pressed
    {
        emit QDialog::finished(r);
        QDialog::done(r);
        return;
    }
}
