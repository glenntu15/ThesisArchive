#ifndef DATAVIEWDIALOG_H
#define DATAVIEWDIALOG_H

#include <QDialog>

namespace Ui {
class DataViewDialog;
}

class DataViewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DataViewDialog(QWidget *parent = nullptr);
    ~DataViewDialog();

    void SetData(QString name,QString metadata,double x0,double y0,double xp,double yp, double xn, double yn);

private:
    Ui::DataViewDialog *ui;
};

#endif // DATAVIEWDIALOG_H
