#ifndef AXISSPECDIALOG_H
#define AXISSPECDIALOG_H

#include "axisspecdata.h"
#include <QDialog>

namespace Ui {
class AxisSpecDialog;
}

class AxisSpecDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AxisSpecDialog(QWidget *parent = nullptr);
    ~AxisSpecDialog();

    void SetDataSource(AxisSpecData * d) {data = d;}
    void SetData(AxisSpecData *d);

    void setFirstv(int v) {firstv = v;}
    void setDeltav(double v) {deltav = v; }

    double getFirstv() {return firstv;}
    double getDeltav() {return deltav;}

private slots:
    void done(int r);

//signals:
 //   void valueChanged();

private:
    Ui::AxisSpecDialog *ui;
    AxisSpecData* data;

    double firstv;
    double deltav;
    int axislen;
    QString axislabel;


};

#endif // AXISSPECDIALOG_H
