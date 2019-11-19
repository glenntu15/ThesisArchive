#ifndef DATAINFODIALOG_H
#define DATAINFODIALOG_H

#include "datamanager.h"
#include <QDialog>

namespace Ui {
class DataInfoDialog;
}

class DataInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DataInfoDialog(QWidget *parent = nullptr);
    ~DataInfoDialog();

private slots:
    void theend();

private:
    Ui::DataInfoDialog *ui;

    DataManager &dm;

    //QString Openfilename;

    QLineEdit *linenamebox;
    QVBoxLayout *mainLayout;
    QGroupBox *horizontalGroupBox;
    QGroupBox *gridGroupBox;
    QGridLayout *gridLayout;

public:
    void SetData(QString name, double x, double y);
};

#endif // DATAINFODIALOG_H
