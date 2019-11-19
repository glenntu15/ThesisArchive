#ifndef PLOTWINDOW_H
#define PLOTWINDOW_H

#include <QDialog>
#include "datamanager.h"
#include "qcustomplot.h"
#include "colortable.h"

class QVBoxLayout;
class QGridLayout;
class QGroupBox;
class AxisSpecData;

#define MAX_BARS 10

namespace Ui {

class PlotWindow;
}

class PlotWindow : public QDialog
{
    Q_OBJECT

public:
    explicit PlotWindow(QWidget *parent = nullptr);
    ~PlotWindow();

   enum ChartTypes {T_Line, T_Scatter, T_Bar, T_ParallelCoords, T_Spectograph, T_Surface };
   void SetDualEnabled(bool setting) {enabledualcharts = setting;}
   void SetDualCharts() {chartType = T_Line; dualCharts = true;}
   void SetChartType(ChartTypes ct) {chartType = ct;}

   void Display();
protected:


    void DisplayPlot();
    DataManager::dstypes currenttype;

    ChartTypes chartType;

private slots:

    void selectionChangedXY();
    void selectionChangedBar();

    //void plotButton_clicked();
    //void bothRadioButton_clicked();
    //void barchartButton_clicked();
    void UserDialogClosed(int r);
    void removeVarButton_clicked();
    void showDataSelect();
    void LegendSelect_StateChange(int state);
    void TitleSelect_StateChange(int state);
    void AltColorSelect_StateChange(int state);
    void PointsOnlySelect_StateChange(int state);
    void PointsSelect_StateChange(int state);
    void ComboBox_IndexChange(int index);
    void titleDoubleClick(QMouseEvent*);

    void mousePress(QMouseEvent* mevent);
    void mouseMove(QMouseEvent *mevent);
    void mouseRelease(QMouseEvent *mevent);


    void graphClicked(QCPAbstractPlottable *plottable, int dataIndex);


private:
    Ui::PlotWindow *ui;

    DataManager &dm;

    bool seriesIsSelected;
    QString lastSelName;
    int lastSelNumber;

    int buttonselected;
    bool dualCharts;
    bool enabledualcharts;
    bool showLegend;
    bool showTitle;
    bool showPoints;
    bool pointsOnly;
    int  numColorGroups;
    bool isrubberband;
    bool showDataIsOn;
    bool useBigFonts;

    // These are used by parallel coordinatae plots
    int numVars;
    unsigned int numGroups;

    // Maybe KLUDGE
    QComboBox *groupselect;

    bool dialogUp;

    QCustomPlot *customPlotXY;
    QCustomPlot *customPlotBar;

    AxisSpecData *axDataX;
    AxisSpecData *axDataY;
    AxisSpecData *barAxDataX;
    AxisSpecData *barAxDataY;

    QString title;
    QCPTextElement *titleitem;

    QRubberBand *rubberBand;
    QPoint rubberOrigin;

    QVBoxLayout *mainLayout;
    QGroupBox *horizontalGroupBox;
    QGroupBox *gridGroupBox;
    QGridLayout *gridLayout;
    // Controls
    //QRadioButton *plotButton;
    //QRadioButton *barchartButton;
    //QRadioButton *bothButton;
    QPushButton *showDataButton;

    ColorTable colorTable;
    bool UseAltColors;

    // QCustomPlot does not store a list of bars like it does graphs so we do it here
    QCPBars *bars[MAX_BARS];

// Methods
    QCustomPlot* CreateXYPlot();
    QCustomPlot* CreateBarPlot();
    QCustomPlot* CreateStackedBarPlot();
    QCustomPlot* CreateParallelCoordsPlot();

    void Addmarks(int ndx);

    void clearLayout(QLayout *layout);
   // void CreateButtonBox();
    void CreateButtonBox(bool pointsoption, bool showdataoption);

    qreal SetAlphaAndLastSelName(QString selname);
    void SetLineSelectedByIndex(int ndx);
    void SetLineListSelectedByIndex(std::vector<int>linelist);
    void SetLineSelectedByName(QString selname, qreal alpha);
    void SetBarSelectedByName(QString selname,qreal alpha);
    void SetAllLinesSelectedByname(QString selname);
    void HighlightSelectedLines(double x1, double x2, double y1, double y2);

};

#endif // PLOTWINDOW_H
