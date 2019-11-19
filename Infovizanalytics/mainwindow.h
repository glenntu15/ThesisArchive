#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDialog>
#include <string>

QT_FORWARD_DECLARE_CLASS(QMenu)
class QVBoxLayout;
class QGridLayout;
class QGroupBox;
class AxisSpecData;
class QLineEdit;
class QLabel;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    void SetOpenFileName(std::string filename) { Openfilename = QString::fromUtf8(filename.c_str());}
    ~MainWindow();

private slots:
    void on_actionFileOpen_triggered();
    void on_plotButton_clicked();
    void on_barPlotButton_clicked();
    void on_bothPlotButton_clicked();
    void on_3DButton_clicked();


private:
    Ui::MainWindow *ui;

    QPushButton *plot3DButton;
    QPushButton *bothPlotButton;
    QPushButton *barPlotButton;
    QPushButton *plotButton;

    QLineEdit *filenamebox;
    QLabel *filenamelabel;
    QString Openfilename;
    QVBoxLayout *mainLayout;
    QGroupBox *horizontalGroupBox;
    QGroupBox *gridGroupBox;
    QGridLayout *gridLayout;

    bool enabledualcharts;


    void setupMenuBar();

public:
    void SetUpButtonsandPlottype();
};

#endif // MAINWINDOW_H
