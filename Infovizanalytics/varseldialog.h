#ifndef VARSELDIALOG_H
#define VARSELDIALOG_H

#include <QDialog>
#include "datamanager.h"
class QCustomPlot;

class DataManager;

namespace Ui {
class VarSelDialog;


}struct catentry {
    int i1;
    int i2;
    double corrcoef;
    double abcorrcoef;
    catentry(int i, int j) {i1 = i; i2 = j;}
    catentry(int i, int j, double r) { i1 = i, i2 = j, corrcoef = r; abcorrcoef = fabs(corrcoef);}
    bool operator < (const catentry& other) const
    {
        return (abcorrcoef > other.abcorrcoef);
    }
};


class VarSelDialog : public QDialog
{
    Q_OBJECT

public:
    explicit VarSelDialog(QWidget *parent = nullptr);
    ~VarSelDialog();

    void Display(int flag) {layoutFlag = flag; Display(); }

private slots:
    void oKButton_clicked();
    void resetButton_clicked();
    void nextButton_clicked();

private:
    Ui::VarSelDialog *ui;

    DataManager &dm;

    bool ShowScatterMatrix;
    int layoutFlag;

    int numVars;
    unsigned int numLittlePlots;
    QTableWidget* pTableWidget;

    QVBoxLayout *mainLayout;
    QGroupBox *horizontalGroupBox;
    QGroupBox *gridGroupBox;
    QGridLayout *gridLayout;

    QPushButton *oKButton;
    QPushButton *resetButton;
    QPushButton *nextButton;
    QPushButton *prevButton;


    std::size_t startdisplayindex;
    std::vector<catentry> varcatalog;

    void Display();
    void BuildVarCatalog();
    void clearLayout(QLayout *layout);
    void CreateButtonBox(); 
    QGridLayout *BuildThumbnailGrid();
    QCustomPlot* CreateLittlePlot(unsigned int ivarx, unsigned int ivary, double r);
    double GetCorrCoef(unsigned int ivarx, unsigned int ivary);

};

#endif // VARSELDIALOG_H
