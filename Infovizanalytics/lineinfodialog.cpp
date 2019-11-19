#include "lineinfodialog.h"
#include "ui_lineinfodialog.h"

LineInfoDialog::LineInfoDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LineInfoDialog)
{
    ui->setupUi(this);
}

LineInfoDialog::~LineInfoDialog()
{
    delete ui;
}
