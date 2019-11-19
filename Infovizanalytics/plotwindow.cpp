#include <string.h>
#include "plotwindow.h"
#include "ui_plotwindow.h"
#include "datamanager.h"
#include "axisspecdialog.h"
#include "axisspecdata.h"
#include "varseldialog.h"
#include "dataviewdialog.h"
#include <sstream>
#include <typeinfo>

#include <QDebug>

//#include <QtCharts>
#include <QtWidgets/QGridLayout>

//Used in combobox
#define NUM_IN_COLORTABLE 10

#ifdef MINGW
#pragma GCC diagnostic ignored "-Wformat"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#else
#pragma warning(disable:4018)
#define _CRT_SECURE_NO_WARNINGS
#endif

PlotWindow::PlotWindow(QWidget *parent) :
    QDialog(parent), ui(new Ui::PlotWindow), dm(DataManager::getInstance())

{
    //QGroupBox *formGroupBox;
    ui->setupUi(this);

    currenttype = dm.getDST();
    chartType = T_Line;  // 1 for bar

    if (currenttype == DataManager::T_datagroup_series)
        chartType = T_Line;   //T_Bar;

    if ((currenttype == DataManager::T_datagroup_parallel) || (currenttype == DataManager::T_dataclass_parallel))
        chartType = T_ParallelCoords;

    //
    // ** initialization **
    //
    mainLayout = nullptr;
    dialogUp = false;
    seriesIsSelected = false;
    lastSelName = "";
    lastSelNumber = -1;
    showLegend = true;
    showTitle = true;
    showPoints = false;
    pointsOnly = false;
    numColorGroups = NUM_IN_COLORTABLE;
    UseAltColors = false;
    colorTable.SetUseAlternate(UseAltColors);

    useBigFonts = true;

    // enabledualcharts flags what the layout should looklike ie. buttons to select line are bar
    // dualcharts flags the layout when the charts are actually displayed
    enabledualcharts = false;
    dualCharts = false;

    title = dm.getDsname();

    axDataX = new AxisSpecData();
    axDataY = new AxisSpecData();
    barAxDataX = new AxisSpecData();
    barAxDataY = new AxisSpecData();

    axDataX->SetDataX();
    barAxDataX->SetDataX();
    if (chartType == T_ParallelCoords) {
        axDataY->SetDataYParallel();
    } else {
        axDataY->SetDataY();
    }

    barAxDataY->SetDataY();

    // show it
    //Display();

}

PlotWindow::~PlotWindow()
{
    delete ui;
}

void PlotWindow::Display()
{

    if (mainLayout != nullptr){
        clearLayout(mainLayout);
        delete mainLayout;
    }

    if ((currenttype == DataManager::T_standard) || (currenttype == DataManager::T_StandardPairs) || (currenttype == DataManager::T_AugmentedPairs))
        CreateButtonBox(true, true);
    else if ((currenttype == DataManager::T_datagroup_series) && (!enabledualcharts))
            CreateButtonBox(false,true);
        else
            CreateButtonBox(false, false);


    if (chartType == T_Line) {
        customPlotXY = CreateXYPlot();     //new QCustomPlot();
        if (dualCharts)
            customPlotBar = CreateBarPlot();
    }
    if (chartType == T_Bar) {
        int numVars = dm.getNumVars();
        if (numVars == 4) {
            customPlotBar = CreateStackedBarPlot();
        } else {
            customPlotBar = CreateBarPlot();
        }
    }
    if (chartType == T_ParallelCoords)
        customPlotXY = CreateParallelCoordsPlot();

    mainLayout = new QVBoxLayout;

    gridLayout = new QGridLayout;
    //QChartView *chartView = new QChartView(customPlotXY);
    if (!dualCharts) {
        if (chartType == T_Line)
            gridLayout->addWidget(customPlotXY, 0, 0);
        else if(chartType == T_ParallelCoords) {
            gridLayout->addWidget(customPlotXY, 0, 0);

        }
        else
            gridLayout->addWidget(customPlotBar, 0, 0);

    } else {
        gridLayout->addWidget(customPlotXY, 0, 0);
        gridLayout->addWidget(customPlotBar, 0, 1);
    }

    gridGroupBox = new QGroupBox(tr(""));
    gridGroupBox->setLayout(gridLayout);


    horizontalGroupBox->setFixedHeight(50);
    //if (currenttype != DataManager::T_standard)

    mainLayout->addWidget(horizontalGroupBox);   // buttons
    mainLayout->addWidget(gridGroupBox);

    setLayout(mainLayout);

}
// this is only called for those plots with group capability
// -- never dual charts
// -- never bar chart
void PlotWindow::DisplayPlot()
{
    delete customPlotXY;

    if (chartType == T_Line) {
        customPlotXY = CreateXYPlot();     //new QCustomPlot()
    } else if (chartType == T_ParallelCoords) {
        customPlotXY = CreateParallelCoordsPlot();
    }

    if (chartType == T_Line)
        gridLayout->addWidget(customPlotXY, 0, 0);
    else if(chartType == T_ParallelCoords) {
        gridLayout->addWidget(customPlotXY, 0, 0);

    }

}

//-------------------------------------------------------------------------------------
QCustomPlot* PlotWindow::CreateXYPlot() {

    QVector<QString> ticlabels;

    customPlotXY = new QCustomPlot();

    customPlotXY->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
                                    QCP::iSelectLegend | QCP::iSelectPlottables);

    colorTable.SetUseAlternate(UseAltColors);

    rubberBand = new QRubberBand(QRubberBand::Rectangle, customPlotXY);
    int numVars = dm.getNumVars();                                                  // This is the number of rows in inputdata
    unsigned int numGroups = static_cast<unsigned>(dm.getNumGroups());              // This is the number of columns


    //std::vector<std::vector<double>>& data = dm.getData();
    QVector<QVector<double>>& qdata = dm.getQdata();

    if (showTitle) {
        customPlotXY->plotLayout()->insertRow(0);
        if (useBigFonts)
            titleitem =  new QCPTextElement(customPlotXY, title, QFont("sans", 14, QFont::Normal));
        else
            titleitem =  new QCPTextElement(customPlotXY, title, QFont("sans", 11, QFont::Normal));
        customPlotXY->plotLayout()->addElement(0, 0, titleitem);
        //customPlotXY->plotLayout()->addElement(0, 0, new QCPTextElement(customPlotXY, title, QFont("sans", 11, QFont::Normal)));
    }

    isrubberband = false;
    showDataIsOn = false;

   // QVector<double> y(numGroups);
    if (currenttype == DataManager::T_datagroup_series) {

         int ng = numGroups;
         QVector<double> x(ng);

        for (int j = 1;j<=ng;j++) {
            x[j-1] = static_cast<double>(j);
            ticlabels.push_back(axDataX->categoryaxistics[j-1]);
        }
        int indx = 0;
        for (int n = 0;n<numVars;n++) {
            if (dm.getOmitVar(n))
                continue;
            QVector<double> y(ng);
            // do two things here, set index values for x, then set labels for tics
            for (int j = 0;j<ng;j++) {
                y[j] = qdata[n][j];
            }

            customPlotXY->addGraph();
            customPlotXY->graph(indx)->setData(x, y);

            QPen pen = colorTable.GetPen(n, numColorGroups);
            pen.setWidth(2);
            customPlotXY->graph(indx)->setPen(pen);
            //std::string temp = dm.getVarName(n); //??
            //QString str = QString::fromStdString(temp);
            customPlotXY->graph(indx)->setName(dm.getQVarName(n));
            indx++;
        }
        //
        //    Axes lables
        //
        // give the axes some labels:
        customPlotXY->xAxis->setLabel(axDataX->axisLabel);
        QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
        textTicker->addTicks(x,ticlabels);
        customPlotXY->xAxis->setTicker(textTicker);

        // set axes ranges, so we see all data:
        customPlotXY->xAxis->setRange(0, numGroups+1);
        customPlotXY->xAxis->setTickLabelRotation(60);

        customPlotXY->yAxis->setLabel(axDataY->axisLabel);
        QSharedPointer<QCPAxisTickerFixed> ticker(new QCPAxisTickerFixed);
        int len = axDataY->axislen;
        //double dbv = axDataY->deltav;
        ticker->setTickCount(len);
        ticker->setTickStep(axDataY->deltav);
        customPlotXY->yAxis->setTicker(ticker);

        double dbfv = axDataY->firstv;
        QString dbgstring = axDataY->axisLabel;
        customPlotXY->yAxis->setLabel(dbgstring);
        customPlotXY->yAxis->setRange(dbfv, axDataY->range);

        if (useBigFonts) {
            QFont pfont("Newyork",6);
            pfont.setStyleHint(QFont::SansSerif);
            pfont.setPointSize(12);
            customPlotXY->xAxis->setTickLabelFont(pfont);
            customPlotXY->xAxis->setLabelFont(pfont);
            customPlotXY->yAxis->setTickLabelFont(pfont);
            customPlotXY->yAxis->setLabelFont(pfont);
        }

    }
    // here we have a line per variable
    // This is the only plot where rubberband zooming works
    else if ((currenttype == DataManager::T_standard) || (currenttype == DataManager::T_StandardPairs) || (currenttype == DataManager::T_AugmentedPairs)) {

        int inp = dm.getNumPoints();
        unsigned int np = static_cast<unsigned>(inp);
        QVector<double> x(inp);
        QVector<double> y(inp);
        int indx = 0;

        unsigned int iz = 0;
        if (currenttype == DataManager::T_standard) {
            for (unsigned int j = 1;j<np;j++) {
                x[j] = qdata[iz][j];
            }

            for (int n = 1;n<numVars;n++) {
                if (dm.getOmitVar(n))
                    continue;

                for(int j = 0;j<np;j++) {
                    y[j] =  qdata[n][j];
                }
                customPlotXY->addGraph();
                customPlotXY->graph(indx)->setData(x, y);
                customPlotXY->graph(indx)->setName(dm.getQVarName(n));

                if ((showPoints) || (pointsOnly)) {
                    if (showPoints)
                        customPlotXY->graph(indx)->setLineStyle(static_cast<QCPGraph::LineStyle>(1));
                    if(pointsOnly)
                        customPlotXY->graph(indx)->setLineStyle((QCPGraph::LineStyle)0);
                    customPlotXY->graph(indx)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 3));
                }

                QPen pen = colorTable.GetPen(n-1,numColorGroups);
                pen.setWidth(1);
                customPlotXY->graph(indx)->setPen(pen);
                indx++;
            }

        } else if (currenttype == DataManager::T_StandardPairs) {

            int ndx = 0;
            int numPairs = dm.getNumPairs();
            QVector<QVector<pairdata>>& pdata = dm.getPdata();

            for (int n = 0;n<numPairs;n++) {

                if (dm.getOmitVar(ndx))
                   continue;

                for(int j = 0;j<np;j++) {
                    x[j] = pdata[n][j].x;
                    y[j] = pdata[n][j].y;
                }
                customPlotXY->addGraph();
                customPlotXY->graph(indx)->setData(x, y);
                customPlotXY->graph(indx)->setName(dm.getQVarName(ndx));
                if ((showPoints) || (pointsOnly)) {
                    if (showPoints){
                        customPlotXY->graph(indx)->setLineStyle((QCPGraph::LineStyle)1);
                        customPlotXY->graph(indx)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 3));
                    } else {
                        customPlotXY->graph(indx)->setLineStyle((QCPGraph::LineStyle)0);
                        customPlotXY->graph(indx)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 6));
                    }
                }

                QPen pen = colorTable.GetPen(indx,numColorGroups);
                pen.setWidth(1);
                customPlotXY->graph(indx)->setPen(pen);
                ndx++;
                indx++;
            }
        } else if (currenttype == DataManager::T_AugmentedPairs) {
            // ///////////////////////////////////////////////  pair stuff here
            int ndx = 0;
            int ndxs = 0;
            int npen = 0;
            QVector<QVector<pairdata>>& pdata = dm.getPdata();
            QVector<double> xs(inp);
            QVector<double> ys(inp);
            int numPairs = dm.getNumPairs();
            for (int n = 0;n<numPairs;n++) {
                //if (dm.getOmitVar(ndx))
                //    continue;
 //
                ndx  = 0;
                ndxs = 0;
                for(int j = 0;j<np;j++) {
                    if (pdata[n][j].indx >= 200) {
                        x[ndx] = pdata[n][j].x;
                        y[ndx] = pdata[n][j].y;
                        ndx++;
                    }
                }
                customPlotXY->addGraph();
                customPlotXY->graph(indx)->setData(x, y);
                QString dbname = dm.getQVarName(n);
                customPlotXY->graph(indx)->setName(dm.getQVarName(n));
                if ((showPoints) || (pointsOnly)) {
                    if (showPoints){
                        customPlotXY->graph(indx)->setLineStyle((QCPGraph::LineStyle)1);
                        customPlotXY->graph(indx)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 3));
                    } else {
                        customPlotXY->graph(indx)->setLineStyle((QCPGraph::LineStyle)0);
                        customPlotXY->graph(indx)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 6));
                    }
                }
                QPen pen = colorTable.GetPen(npen,numColorGroups);
// not repeat for other ...
                for(int j = 0;j<np;j++) {
                    if (pdata[n][j].indx < 200) {
                        xs[ndxs] = pdata[n][j].x;
                        ys[ndxs] = pdata[n][j].y;
                        ndxs++;
                    }
                }
                customPlotXY->graph(indx)->setPen(pen);
                customPlotXY->addGraph();
                indx++;
                customPlotXY->graph(indx)->setData(xs, ys);
                //customPlotXY->graph(indx)->setName(dm.getQVarName(n+1));
                if ((showPoints) || (pointsOnly)) {
                    if (showPoints){
                        customPlotXY->graph(indx)->setLineStyle((QCPGraph::LineStyle)1);
                        customPlotXY->graph(indx)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 3));
                    } else {
                        customPlotXY->graph(indx)->setLineStyle((QCPGraph::LineStyle)0);
                        customPlotXY->graph(indx)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 6));
                    }
                }
                QPen pens = colorTable.GetPen(npen,numColorGroups,100-15);

                pen.setWidth(1);
                customPlotXY->graph(indx)->removeFromLegend();
                customPlotXY->graph(indx)->setPen(pens);
                //customPlotXY->addGraph();
                indx++;
                npen++;
            }
        } // end of for

        if (dm.getNUmMarks() > 0){
            Addmarks(indx);
        }

        //
        //    Axes set up
        //
        //  y-axis
        customPlotXY->yAxis->setLabel(axDataY->axisLabel);
        QSharedPointer<QCPAxisTickerFixed> ticker(new QCPAxisTickerFixed);
        int len = axDataY->axislen;
        //double dbv = axDataY->deltav;
        ticker->setTickCount(len);
        ticker->setTickStep(axDataY->deltav);
        customPlotXY->yAxis->setTicker(ticker);

        //double dbfv = axDataY->firstv;
        //double dbr = axDataY->range;
        QString dbgstring = axDataY->axisLabel;
        customPlotXY->yAxis->setLabel(dbgstring);
        customPlotXY->yAxis->setRange(axDataY->firstv, axDataY->range);

        customPlotXY->yAxis->setLabel(axDataY->axisLabel);
        //double dbfvx = axDataX->firstv;
        //double dbrx = axDataX->range;
        //QString dbgstring = axDataY->axisLabel;
        customPlotXY->xAxis->setLabel(axDataX->axisLabel);
        customPlotXY->xAxis->setRange(axDataX->firstv, axDataX->range);

        if (useBigFonts) {
            QFont pfont("Newyork",6);
            QFont tfont("Newyork",6);
            pfont.setStyleHint(QFont::SansSerif);
            pfont.setPointSize(14);
            tfont.setPointSize(15);
            customPlotXY->xAxis->setTickLabelFont(pfont);
            customPlotXY->xAxis->setLabelFont(pfont);
            customPlotXY->yAxis->setTickLabelFont(tfont);
            customPlotXY->yAxis->setLabelFont(pfont);
        }

    }

    if (showLegend){

        customPlotXY->legend->setVisible(true);
        if (useBigFonts) {
            QFont lfont = customPlotXY->legend->font();
            lfont.setPointSize(13);
            customPlotXY->legend->setFont(lfont);
        }
    }


    connect(customPlotXY, SIGNAL(plottableClick(QCPAbstractPlottable*,int,QMouseEvent*)), this, SLOT(graphClicked(QCPAbstractPlottable*,int)));

    connect(customPlotXY, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChangedXY()));
    if (showTitle)
         connect(titleitem, SIGNAL(doubleClicked(QMouseEvent*)), this, SLOT(titleDoubleClick(QMouseEvent*)));

    connect(customPlotXY, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress(QMouseEvent*)));
    connect(customPlotXY, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(mouseMove(QMouseEvent*)));
    connect(customPlotXY, SIGNAL(mouseRelease(QMouseEvent*)), this, SLOT(mouseRelease(QMouseEvent*)));

    return customPlotXY;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void PlotWindow::Addmarks(int indx){

    int numMarks = dm.getNUmMarks();
    int ndx, np;

    QString out_string;
    std::stringstream ss;

    QVector<QVector<double>>& marksdata = dm.getMarksdata();

    int count =0;
    // even enpty marks have one elements -- this keeps the associated with a variable
    for (int n = 0;n<numMarks;n+=2) {
        ndx = n + 1;
        np = marksdata[n].length();
        if (np > 1) {
            QVector<double> x(np);
            QVector<double> y(np);

            for(int j = 0;j<np;j++) {
                x[j] = marksdata[n][j];
                y[j] = marksdata[ndx][j];
            }
            customPlotXY->addGraph();
            customPlotXY->graph(indx)->setData(x, y);
            ss << count;
            out_string = QString::fromStdString(ss.str());
            customPlotXY->graph(indx)->setName(out_string);
            QPen pen = colorTable.GetPen(indx,numColorGroups);
            pen.setWidth(1);
            customPlotXY->graph(indx)->setPen(pen);
            customPlotXY->graph(indx)->setLineStyle((QCPGraph::LineStyle)0);
            customPlotXY->graph(indx)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCrossCircle, 9));
            indx++;
        }
        count++;
        ss.str("");
    }

}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
QCustomPlot* PlotWindow::CreateBarPlot()
{

    QVector<QString> ticlabels;

    customPlotBar = new QCustomPlot();
    customPlotBar->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
                                    QCP::iSelectLegend | QCP::iSelectPlottables);

    if (showTitle) {
        customPlotBar->plotLayout()->insertRow(0);
        if (useBigFonts)
            titleitem =  new QCPTextElement(customPlotBar, title, QFont("sans", 13, QFont::Normal));
        else
            titleitem =  new QCPTextElement(customPlotBar, title, QFont("sans", 11, QFont::Normal));
        customPlotBar->plotLayout()->addElement(0, 0, titleitem);
        //customPlotXY->plotLayout()->addElement(0, 0, new QCPTextElement(customPlotXY, title, QFont("sans", 11, QFont::Normal)));
    }

    isrubberband = false;
    showDataIsOn = false;

    // clear class member array of bar*, (QPCustomPlot does not have a list of bars)
    for (int i = 0;i<MAX_BARS;i++)
        bars[i] = nullptr;

    int numVars = dm.getNumVars();                                                  // This is the number of rows in inputdata
    if (numVars > MAX_BARS)
        numVars = MAX_BARS;
    unsigned int numGroups = static_cast<unsigned>(dm.getNumGroups());              // This is the number of columns

    QCPBarsGroup *group1 = new QCPBarsGroup(customPlotBar);

    QVector<double> x(numGroups+1);
    QVector<QVector<double>>& qdata = dm.getQdata();

    if (currenttype == DataManager::T_datagroup_series) {
       for (unsigned int j = 1;j<=numGroups;j++) {
           x[j-1] = static_cast<double>(j);
           ticlabels.push_back(axDataX->categoryaxistics[j-1]);
       }
       int indx = 0;
       for (int n = 0;n<numVars;n++) {
           if (dm.getOmitVar(n))
               continue;
           QVector<double> y(numGroups+1);
           for (unsigned int j = 0;j<numGroups;j++) {
               y[j] = qdata[n][j];

           }
           //std::string temp = dm.getVarName(n);
           QString str = dm.getQVarName(n);   //QString::fromStdString(temp);

           bars[indx] = new QCPBars(customPlotBar->xAxis, customPlotBar->yAxis);
           bars[indx]->setData(x, y);

           bars[indx]->setBrush(QColor(colorTable.values[n].r,colorTable.values[n].g,colorTable.values[n].b, 100));
           bars[indx]->setPen(QColor(colorTable.values[n].r,colorTable.values[n].g,colorTable.values[n].b));

           bars[indx]->setWidth(0.1);
           bars[indx]->setBarsGroup(group1);
           bars[indx]->setName(str);
           indx++;
        }
    }

    customPlotBar->xAxis->setLabel(barAxDataX->axisLabel);
    QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
    textTicker->addTicks(x,ticlabels);
    customPlotBar->xAxis->setTicker(textTicker);
    customPlotBar->xAxis->setRange(0, numGroups+1);
    customPlotBar->xAxis->setTickLabelRotation(60);

    customPlotBar->yAxis->setLabel(barAxDataY->axisLabel);
    int len = barAxDataY->axislen;
   // double dbv = barAxDataY->deltav;
    QSharedPointer<QCPAxisTickerFixed> ticker(new QCPAxisTickerFixed);
    ticker->setTickCount(len);
    double dbdelv = barAxDataY->deltav;
    ticker->setTickStep(dbdelv);
    customPlotBar->yAxis->setTicker(ticker);

    double dbfv = barAxDataY->firstv;
    QString dbgstring = barAxDataY->axisLabel;

    customPlotBar->yAxis->setRange(dbfv, barAxDataY->range);
    if (useBigFonts) {
        QFont pfont("Newyork",6);
        pfont.setStyleHint(QFont::SansSerif);
        pfont.setPointSize(14);
        customPlotBar->xAxis->setTickLabelFont(pfont);
        customPlotBar->xAxis->setLabelFont(pfont);
        customPlotBar->yAxis->setTickLabelFont(pfont);
        customPlotBar->yAxis->setLabelFont(pfont);
    }

    //customPlotBar->legend->setVisible(true);
    if (showLegend){

        customPlotBar->legend->setVisible(true);
        if (useBigFonts) {
            QFont lfont = customPlotBar->legend->font();
            lfont.setPointSize(13);
            customPlotBar->legend->setFont(lfont);
        }
    }

    connect(customPlotBar, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChangedBar()));
    connect(titleitem, SIGNAL(doubleClicked(QMouseEvent*)), this, SLOT(titleDoubleClick(QMouseEvent*)));

    return customPlotBar;

}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
QCustomPlot* PlotWindow::CreateStackedBarPlot()
{
// *****************************************************************************************
//                    NOTE:  THIS ONLY WORKS FOR 4 GROUPS -- FOUR BARS PER TIC
//                           STACKED CANNOT HAVE OMITED VARABLES
// *****************************************************************************************
    QVector<QString> ticlabels;

    customPlotBar = new QCustomPlot();
    customPlotBar->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
                                    QCP::iSelectLegend | QCP::iSelectPlottables);

    if (showTitle) {
        customPlotBar->plotLayout()->insertRow(0);
        if (useBigFonts)
            titleitem =  new QCPTextElement(customPlotBar, title, QFont("sans", 14, QFont::Normal));
        else
            titleitem =  new QCPTextElement(customPlotBar, title, QFont("sans", 11, QFont::Normal));
        customPlotBar->plotLayout()->addElement(0, 0, titleitem);
        //customPlotXY->plotLayout()->addElement(0, 0, new QCPTextElement(customPlotXY, title, QFont("sans", 11, QFont::Normal)));
    }
    isrubberband = false;
    showDataIsOn = false;

    // clear class member array of bar*, (QPCustomPlot does not have a list of bars)
    for (int i = 0;i<MAX_BARS;i++)
        bars[i] = nullptr;

    int numVars = dm.getNumVars();                                                  // This is the number of rows in inputdata
    if (numVars > MAX_BARS)
        numVars = MAX_BARS;
    unsigned int numGroups = static_cast<unsigned>(dm.getNumGroups());              // This is the number of columns
//
//  Check for 4 vars, check for omitted
//
    bool flaginvalid = false;
    if ((numVars % 2) != 0) {
        flaginvalid = true;
    }
    for (int n = 0;n<numVars;n++) {
        if (dm.getOmitVar(n))
          flaginvalid = true;
    }
    if (flaginvalid) {
        printf(" NOT ABLE TO USE STACKED PLOTS");
        qDebug() << "NOT ABLE TO USE STACKED PLOTS";
    }
    QVector<double> x(numGroups+1);
    QVector<double> xbar1(numGroups+1);
    QVector<double> xbar2(numGroups+1);
    QVector<QVector<double>>& qdata = dm.getQdata();

    if (currenttype == DataManager::T_datagroup_series) {
        for (unsigned int j = 1;j<=numGroups;j++) {
           x[j-1] = static_cast<double>(j);
           xbar1[j-1] = x[j-1] -.125;
           xbar2[j-1] = x[j-1] +.125;

           ticlabels.push_back(axDataX->categoryaxistics[j-1]);
        }
        int indx = 0;
        //int nstackgroup = 0;
        for (unsigned int n = 0;n<numVars;n++) {

            QVector<double> y(numGroups+1);
            for (int j = 0;j<numGroups;j++) {
               y[j] = qdata[n][j];

            }
            //std::string temp = dm.getVarName(n);
            QString str = dm.getQVarName(n);   //QString::fromStdString(temp);

            bars[indx] = new QCPBars(customPlotBar->xAxis, customPlotBar->yAxis);
            if (indx > (numVars/2 - 1)) {
                bars[indx]->setData(xbar2, y);
            } else {
                bars[indx]->setData(xbar1, y);
            }

            bars[indx]->setBrush(QColor(colorTable.values[n].r,colorTable.values[n].g,colorTable.values[n].b, 100));
            //QPen pen = QPen(QColor(colorTable.values[n].r,colorTable.values[n].g,colorTable.values[n].b), 130);
            bars[indx]->setPen(QColor(colorTable.values[n].r,colorTable.values[n].g,colorTable.values[n].b));

            bars[indx]->setWidth(0.1);

            bars[indx]->setStackingGap(0);
            if ((indx % 2) == 1)
                bars[indx]->moveAbove(bars[indx-1]);
            bars[indx]->setAntialiased(false);
            //bars[indx]->setBarsGroup(0);

            bars[indx]->setName(str);
            indx++;
        }
    }

    customPlotBar->xAxis->setLabel(barAxDataX->axisLabel);
    QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
    textTicker->addTicks(x,ticlabels);
    customPlotBar->xAxis->setTicker(textTicker);
    customPlotBar->xAxis->setRange(0, numGroups+1);
    customPlotBar->xAxis->setTickLabelRotation(60);

    customPlotBar->yAxis->setLabel(barAxDataY->axisLabel);
    int len = barAxDataY->axislen;
   // double dbv = barAxDataY->deltav;
    QSharedPointer<QCPAxisTickerFixed> ticker(new QCPAxisTickerFixed);
    ticker->setTickCount(len);
    double dbdelv = barAxDataY->deltav;
    ticker->setTickStep(dbdelv);
    customPlotBar->yAxis->setTicker(ticker);

    double dbfv = barAxDataY->firstv;
    QString dbgstring = barAxDataY->axisLabel;

    customPlotBar->yAxis->setRange(dbfv, barAxDataY->range);
    if (useBigFonts) {
        QFont pfont("Newyork",6);
        pfont.setStyleHint(QFont::SansSerif);
        pfont.setPointSize(15);
        customPlotBar->xAxis->setTickLabelFont(pfont);
        customPlotBar->xAxis->setLabelFont(pfont);
        customPlotBar->yAxis->setTickLabelFont(pfont);
        customPlotBar->yAxis->setLabelFont(pfont);
    }

    //customPlotBar->legend->setVisible(true);
    if (showLegend){

        customPlotBar->legend->setVisible(true);
        if (useBigFonts) {
            QFont lfont = customPlotBar->legend->font();
            lfont.setPointSize(13);
            customPlotBar->legend->setFont(lfont);
        }
    }

    connect(customPlotBar, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChangedBar()));

    return customPlotBar;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
QCustomPlot* PlotWindow::CreateParallelCoordsPlot()
{
    customPlotXY = new QCustomPlot();

    customPlotXY->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
                                    QCP::iSelectLegend | QCP::iSelectPlottables);

    if (showTitle) {
        customPlotXY->plotLayout()->insertRow(0);
        if (useBigFonts)
            customPlotXY->plotLayout()->addElement(0, 0, new QCPTextElement(customPlotXY, title, QFont("sans", 14, QFont::Normal)));
        else
            customPlotXY->plotLayout()->addElement(0, 0, new QCPTextElement(customPlotXY, title, QFont("sans", 11, QFont::Normal)));
    }

    rubberBand = new QRubberBand(QRubberBand::Rectangle, customPlotXY);

    isrubberband = false;
    showDataIsOn = false;

    numVars = dm.getNumVars();              // This is the number of rows in inputdata
    numGroups = dm.getNumGroups();          // This is the number of columns


    QVector<QVector<double>>& qdata = dm.getQCategorydata();  // NOTE:: its not qdata but qcategorydata
    //QVector<QVector<double>>& qdata = dm.getQdata();  // NOTE:: its not qdata but qcategorydata

    QVector<double> x(numGroups);
    QVector<QString> ticlabels;
    QVector<QString> ticlabels2;  // this goes at top axis
   // QVector<double> y(numGroups);
    // for use with class data
    QString lastvar = "";
    QString currentvar;
    int isequal;
    int classnum = -1;
    for (int j = 0;j<numGroups;j++) {
        x[j] = static_cast<double>(j);
        //QString sdbg = axDataX->categoryaxistics[j];
        ticlabels.push_back(axDataX->categoryaxistics[j]);
        ticlabels2.push_back(axDataX->catagorymaxlabels[j]);
    }
    int indx = 0;
    for (int n = 0;n<numVars;n++) {
        if (dm.getOmitVar(n))
            continue;

        QVector<double> y(numGroups);
        // do two things here, set index values for x, then set labels for tics
        for (int j = 0;j<numGroups;j++) {
            y[j] = qdata[n][j];
        }

        customPlotXY->addGraph();
        customPlotXY->graph(indx)->setData(x, y);
        if (currenttype == DataManager::T_dataclass_parallel) {
            currentvar = dm.getQVarName(n);
            isequal = QString::compare(currentvar, lastvar);
            if (isequal != 0) {
                customPlotXY->graph(indx)->setName(dm.getQVarName(n));
                classnum++;
                lastvar = currentvar;
            }
            QPen pen = colorTable.GetPen(classnum, numColorGroups);
            pen.setWidth(2);
            customPlotXY->graph(indx)->setPen(pen);
            if (isequal == 0)
               customPlotXY->graph(indx)->removeFromLegend();
            customPlotXY->graph(indx)->setName(dm.getQVarName(n));

        } else {                    // datagroup

            QPen pen = colorTable.GetPen(n, numColorGroups);
            pen.setWidth(2);
            customPlotXY->graph(indx)->setPen(pen);

            customPlotXY->graph(indx)->setName(dm.getQVarName(n));
        }

        indx++;
    }

    // give the axes some labels:
    QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
    textTicker->addTicks(x,ticlabels);
    customPlotXY->xAxis->setTicker(textTicker);
    customPlotXY->xAxis->setTickLabelRotation(60);

    QSharedPointer<QCPAxisTickerText> textTicker2(new QCPAxisTickerText);
    textTicker2->addTicks(x,ticlabels2);
    customPlotXY->xAxis2->setVisible(true);
    customPlotXY->xAxis->setRange(0, numGroups);
    //customPlotXY->yAxis2->setRange(0, numGroups+1);
    customPlotXY->xAxis2->setTickLabels(true);
    customPlotXY->xAxis2->setTicker(textTicker2);
    customPlotXY->xAxis2->setRange(0, numGroups);


    QCPGrid* gp = customPlotXY->xAxis->grid();
    gp->setPen(QPen(QColor(0,0,0), 2, Qt::SolidLine));

    QString dbstr = axDataY->axisLabel;
    customPlotXY->yAxis->setLabel(axDataY->axisLabel);
    ///=****customPlotXY->yAxis->setVisible(false);
    // set axes ranges, so we see all data:

    QSharedPointer<QCPAxisTickerFixed> ticker(new QCPAxisTickerFixed);
    int len = axDataY->axislen;
    //double dbv = axDataY->deltav;
    ticker->setTickCount(len);
    ticker->setTickStep(axDataY->deltav);
    customPlotXY->yAxis->setTicker(ticker);

    double dbfv = axDataY->firstv;
    QString dbgstring = axDataY->axisLabel;
    customPlotXY->yAxis->setLabel(dbgstring);
    customPlotXY->yAxis->setRange(dbfv, axDataY->range);

    if (useBigFonts) {
        QFont pfont("Newyork",6);
        QFont lfont("Newyork",6);
        pfont.setStyleHint(QFont::SansSerif);
        pfont.setPointSize(16);
        lfont.setPointSize(14);
        customPlotXY->xAxis->setTickLabelFont(pfont);
        customPlotXY->xAxis->setLabelFont(lfont);
        customPlotXY->xAxis2->setLabelFont(lfont);
         customPlotXY->xAxis2->setTickLabelFont(pfont);
        customPlotXY->yAxis->setTickLabelFont(pfont);
        customPlotXY->yAxis->setLabelFont(lfont);
    }

    connect(customPlotXY, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChangedXY()));

    connect(customPlotXY, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress(QMouseEvent*)));
    connect(customPlotXY, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(mouseMove(QMouseEvent*)));
    connect(customPlotXY, SIGNAL(mouseRelease(QMouseEvent*)), this, SLOT(mouseRelease(QMouseEvent*)));

    //customPlotXY->legend->setVisible(true);
    if (showLegend){
        customPlotXY->legend->setVisible(true);
        if (useBigFonts) {
            QFont lfont = customPlotXY->legend->font();
            lfont.setPointSize(13);
            customPlotXY->legend->setFont(lfont);
        }
    }

    return customPlotXY;
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
void PlotWindow::selectionChangedXY()
{
  /*
   normally, axis base line, axis tick labels and axis labels are selectable separately, but we want
   the user only to be able to select the axis as a whole, so we tie the selected states of the tick labels
   and the axis base line together. However, the axis label shall be selectable individually.

   The selection state of the left and right axes shall be synchronized as well as the state of the
   bottom and top axes.

   Further, we want to synchronize the selection of the graphs with the selection state of the respective
   legend item belonging to that graph. So the user can select a graph by either clicking on the graph itself
   or on its legend item.
  */

  // make top and bottom axes be selected synchronously, and handle axis and tick labels as one selectable object:
  if (customPlotXY->xAxis->selectedParts().testFlag(QCPAxis::spAxis) || customPlotXY->xAxis->selectedParts().testFlag(QCPAxis::spTickLabels))
  {

    customPlotXY->xAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    if (!dialogUp) {
        QPointer< AxisSpecDialog>  axisspec = new AxisSpecDialog();

        QObject::connect(axisspec, &QDialog::finished, this, &PlotWindow::UserDialogClosed);

        axisspec->SetData(axDataX);
        axisspec->show();
        return;
    }
  }
  // make left and right axes be selected synchronously, and handle axis and tick labels as one selectable object:
    if (customPlotXY->yAxis->selectedParts().testFlag(QCPAxis::spAxis) || customPlotXY->yAxis->selectedParts().testFlag(QCPAxis::spTickLabels)) {
        if (!dialogUp) {
            customPlotXY->yAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
            QPointer< AxisSpecDialog>  axisspec = new AxisSpecDialog();
            QObject::connect(axisspec, &QDialog::finished, this, &PlotWindow::UserDialogClosed);

            axisspec->SetData(axDataY);
            axisspec->show();
            dialogUp = true;
        }
  }

    if (showDataIsOn)
        return;

  // synchronize selection of graphs with selection of corresponding legend items:
    if (currenttype != DataManager::T_dataclass_parallel) {

        for (int i=0; i<customPlotXY->graphCount(); ++i)
        {
            QCPGraph *graph = customPlotXY->graph(i);
            QCPPlottableLegendItem *item = customPlotXY->legend->itemWithPlottable(graph);
            if (item == nullptr)
                continue;
            if (item->selected() || graph->selected()) {
                graph->setSelectionDecorator(nullptr);
                QString selectedname = graph->name();
                //bool selectedvalue = graph->selected();

                qreal alpha = SetAlphaAndLastSelName(selectedname);
                SetLineSelectedByName(selectedname, alpha);
                if (dualCharts) {

                    SetBarSelectedByName(selectedname, alpha);
                    customPlotBar->replot();
                }
               // item->setSelected(true);
            }
        }
    } else {
       //qDebug() << " count = " << customPlotXY->graphCount() << "\n";
       int lineNoSelected = -1;
       for (int i=0; i<customPlotXY->graphCount(); ++i) {
           QCPGraph *graph = customPlotXY->graph(i);
           bool selectedvalue = graph->selected();
           QCPPlottableLegendItem *item = customPlotXY->legend->itemWithPlottable(graph);
           if (selectedvalue) {
               lineNoSelected  = i;
               graph->setSelectionDecorator(nullptr);
           }
            //qDebug() << "line selection" << selectedvalue << "line name " <<graph->name() << "\n";

           if (item != nullptr) {
               if (item->selected()) {      // a legend was selected not a line
                   graph->setSelectionDecorator(nullptr);
                   QString selectedname = graph->name();
                   SetAllLinesSelectedByname(selectedname);

                   item->setSelected(true);
               }
               if (selectedvalue)
                   item->setSelected(true);

           }
        }
        // if line was selectted just hightlight that line
       if (lineNoSelected > -1) {
           SetLineSelectedByIndex(lineNoSelected );
       }
    }
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void PlotWindow::selectionChangedBar()
{
  /*
   normally, axis base line, axis tick labels and axis labels are selectable separately, but we want
   the user only to be able to select the axis as a whole, so we tie the selected states of the tick labels
   and the axis base line together. However, the axis label shall be selectable individually.

   The selection state of the left and right axes shall be synchronized as well as the state of the
   bottom and top axes.

   Further, we want to synchronize the selection of the graphs with the selection state of the respective
   legend item belonging to that graph. So the user can select a graph by either clicking on the graph itself
   or on its legend item.
  */

  // make top and bottom axes be selected synchronously, and handle axis and tick labels as one selectable object:
    if (customPlotBar->xAxis->selectedParts().testFlag(QCPAxis::spAxis) || customPlotBar->xAxis->selectedParts().testFlag(QCPAxis::spTickLabels)) {
        if (!dialogUp) {
            customPlotBar->xAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
            QPointer< AxisSpecDialog>  axisspec = new AxisSpecDialog();
            QObject::connect(axisspec, &QDialog::finished, this, &PlotWindow::UserDialogClosed);
            axisspec->SetData(barAxDataX);
            axisspec->show();
            dialogUp = true;
        }

    }
      // make left and right axes be selected synchronously, and handle axis and tick labels as one selectable object:
    if (customPlotBar->yAxis->selectedParts().testFlag(QCPAxis::spAxis) || customPlotBar->yAxis->selectedParts().testFlag(QCPAxis::spTickLabels)) {
        customPlotBar->yAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
        if (!dialogUp) {
            QPointer< AxisSpecDialog>  axisspec = new AxisSpecDialog();
            QObject::connect(axisspec, &QDialog::finished, this, &PlotWindow::UserDialogClosed);
            axisspec->SetData(barAxDataY);
            axisspec->show();
            dialogUp = true;
        }
    }

    QList<QCPAbstractPlottable*> selectedList = customPlotBar->selectedPlottables();

    // synchronize selection of graphs with selection of corresponding legend items:
    int numselected = selectedList.length();
    if (numselected > 0) {
        QCPAbstractPlottable* thing = selectedList[0];

        QCPPlottableLegendItem *item = customPlotBar->legend->itemWithPlottable(thing);
        std::string thingname = typeid(*thing).name();
        QCPBars * sbar = dynamic_cast<QCPBars*>(thing);
        QString selectedname = sbar->name();

        sbar->setSelectionDecorator(nullptr);

        qreal alpha = SetAlphaAndLastSelName(selectedname);
        SetBarSelectedByName(selectedname, alpha);
        if (dualCharts) {
            SetLineSelectedByName(selectedname, alpha);
            customPlotXY->replot();
        }

        item->setSelected(true);
    }

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void PlotWindow::SetAllLinesSelectedByname(QString selname)
{
    QColor color;
    QPen pen1;
    qreal alpha = 1.0;
    qreal altalpha = .05;

    // if same as last line selected then we light up all the lines
    int iseq = QString::compare(selname, lastSelName);
    if (iseq == 0) {
        // unselect;
        seriesIsSelected = false;
        lastSelName = "";
        altalpha = 1.0;
    }

    for (int i=0; i<customPlotXY->graphCount(); i++) {
        //QCPGraph *graph = customPlotXY->graph(i);
        QString name = customPlotXY->graph(i)->name();
        int iseq = QString::compare(selname, name);

        pen1 = customPlotXY->graph(i)->pen();
        if (iseq == 0) {
            color = pen1.color();
            color.setAlphaF(alpha);
            pen1.setColor(color);
        } else {
            color = pen1.color();
            color.setAlphaF(altalpha);
            pen1.setColor(color);
        }
        customPlotXY->graph(i)->setPen(pen1);
    }
    customPlotXY->replot();
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void PlotWindow::SetLineSelectedByIndex(int ndx)
{
    QColor color;
    QPen pen1;
    qreal alpha = 1.0;
    qreal altalpha = .05;

    // if same as last line selected then we light up all the lines
    if ( ndx == lastSelNumber) {
        altalpha = 1.0;
        lastSelNumber = -1;
    } else {
        lastSelNumber = ndx;
    }
    //qDebug() << " count = " << customPlotXY->graphCount() << "\n";
    for (int i=0; i<customPlotXY->graphCount(); i++) {
        QCPGraph *graph = customPlotXY->graph(i);
        bool selectedvalue = graph->selected();

        pen1 = customPlotXY->graph(i)->pen();
        if (selectedvalue) {
            color = pen1.color();
            color.setAlphaF(alpha);
            pen1.setColor(color);
        } else {
            color = pen1.color();
            color.setAlphaF(altalpha);
            pen1.setColor(color);
        }
        customPlotXY->graph(i)->setPen(pen1);
    }

}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void PlotWindow::SetLineListSelectedByIndex(std::vector<int>linelist)
{
    QColor color;
    QPen pen1;
    qreal alpha = 1.0;
    qreal altalpha = .03;

    for (int i=0; i<customPlotXY->graphCount(); i++) {

        //QCPGraph *graph = customPlotXY->graph(i);
        pen1 = customPlotXY->graph(i)->pen();

        if (std::find(linelist.begin(), linelist.end(), i) != linelist.end()) {
            color = pen1.color();
            color.setAlphaF(alpha);
            pen1.setColor(color);
        } else {
            color = pen1.color();
            color.setAlphaF(altalpha);
            pen1.setColor(color);
        }
        customPlotXY->graph(i)->setPen(pen1);
    }
    //SetLineSelectedByIndex(1);
    customPlotXY->replot();

}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void PlotWindow::SetLineSelectedByName(QString selname, qreal alpha)
{
    QColor color;
    QPen pen1;

    QCPGraph *selected = nullptr;

    for (int i=0; i<customPlotXY->graphCount(); i++) {
        QCPGraph *graph = customPlotXY->graph(i);
        pen1 = customPlotXY->graph(i)->pen();
        int isequal = QString::compare(selname, graph->name());
        if (isequal == 0) {
            selected = customPlotXY->graph(i);
        }
        color = pen1.color();
        color.setAlphaF(alpha);
        pen1.setColor(color);
        customPlotXY->graph(i)->setPen(pen1);
    }
    pen1 = selected->pen();
    color = pen1.color();
    color.setAlphaF(1.0);
    pen1.setColor(color);
    selected->setPen(pen1);

}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
qreal PlotWindow::SetAlphaAndLastSelName(QString selname){
    qreal alpha = 1.0;

    if (seriesIsSelected) {
        // are we reselecting the same one...
        int x = QString::compare(selname, lastSelName);
        if (x == 0) {
            // unselect;
            seriesIsSelected = false;
            lastSelName = "";
            alpha = 1.0;
        } else {
            // new series selection
            seriesIsSelected = true; // still true
            lastSelName = selname;
            alpha = .05;
        }
    } else {   // here when nothing was selected
        seriesIsSelected = true;
        lastSelName = selname;
        alpha = .05;
    }
    //lastSelName = selname;

    return alpha;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void PlotWindow::SetBarSelectedByName(QString selname, qreal alpha)
{
    QColor color;
    QPen pen1;
    QCPBars* selected = nullptr;


    for (int i=0; i<MAX_BARS; ++i) {
        if (bars[i] != nullptr) {
            int x = QString::compare(selname, bars[i]->name());
            if (x ==0) {
                selected = bars[i];
            }
            pen1 = bars[i]->pen();
            color = pen1.color();

            //color = colorlist[i];
            color.setAlphaF(alpha);
            bars[i]->setPen(color);

            color.setAlpha(100);
            bars[i]->setBrush(color);

        }

    }
    if (selected) {
        pen1 = selected->pen();
        color = pen1.color();
        color.setAlphaF(1.0);
        selected->setPen(color);
        color.setAlpha(100);
        selected->setBrush(color);
    }

}
//--------------------------------------------------------------------------------
void PlotWindow::graphClicked(QCPAbstractPlottable *plottable, int dataIndex)
{
    if (showDataIsOn) {
        // since we know we only have QCPGraphs in the plot, we can immediately access interface1D()
        // usually it's better to first check whether interface1D() returns non-zero, and only then use it.
        double dataValue = plottable->interface1D()->dataMainValue(dataIndex);
        double datakey = plottable->interface1D()->dataMainKey(dataIndex);
        QString name = plottable->name();
        double x0 = datakey;
        double y0 = dataValue;
        //QString message = QString("Clicked on graph '%1' at data point #%2 with key %3 value %4.").arg(plottable->name()).arg(dataIndex).arg(datakey).arg(dataValue);
        //qDebug() << message << "\n";
        if (dataIndex >= 0){
            double yp = plottable->interface1D()->dataMainValue(dataIndex-1);
            double xp = plottable->interface1D()->dataMainKey(dataIndex-1);
            //QString message = QString("prev point: key %1 value %2.").arg(datakey).arg(dataValue);
            //qDebug() << message << "\n";

            double yn = plottable->interface1D()->dataMainValue(dataIndex+1);
            double xn = plottable->interface1D()->dataMainKey(dataIndex+1);

            DataViewDialog* dlg = new DataViewDialog();
            QString metadata = dm.getMetadata(name,dataIndex);
            //QString metadata("1425322307:r:46.06:g:11.53:y:32.09");
            dlg->SetData(name,metadata,x0, y0, xp, yp, xn, yn);
            dlg->show();
        }
    }

}
//--------------------------------------------------------------------------------
void PlotWindow::mousePress( QMouseEvent *mevent )
{
    rubberOrigin = mevent->pos();

    if(mevent->button() == Qt::RightButton)
    {
        isrubberband = true;
        rubberBand->setGeometry(QRect(rubberOrigin, QSize()));
        rubberBand->show();
    }
}
//--------------------------------------------------------------------------------
void PlotWindow::mouseMove( QMouseEvent *mevent )
{
    if (isrubberband)
        rubberBand->setGeometry(QRect(rubberOrigin, mevent->pos()).normalized());
}
//--------------------------------------------------------------------------------
void PlotWindow::mouseRelease( QMouseEvent *mevent )
{
    rubberOrigin = mevent->pos();
    if (isrubberband) {
        const QRect & zoomRect = rubberBand->geometry();
        int xp1, yp1, xp2, yp2;
        zoomRect.getCoords(&xp1, &yp1, &xp2, &yp2);
        double x1 = customPlotXY->xAxis->pixelToCoord(xp1);
        double x2 = customPlotXY->xAxis->pixelToCoord(xp2);
        double y1 = customPlotXY->yAxis->pixelToCoord(yp1);
        double y2 = customPlotXY->yAxis->pixelToCoord(yp2);

        rubberBand->hide();
        if(chartType == T_ParallelCoords) {
            HighlightSelectedLines(x1, x2, y1, y2);

        } else {
            axDataX->OverRideData(x1,x2);
            axDataY->OverRideData(y1,y2);
        }

        isrubberband = false;
    }

}
//--------------------------------------------------------------------------------
void PlotWindow::HighlightSelectedLines(double x1, double x2, double y1, double y2)
{
    // find first line.
    //std::vector<int>::size_type j = static_cast<size_t>(x1 + .5);
    int j = static_cast<int>(x1 + .5);
    std::vector<int> linelist;
    std::vector<int>::size_type n;
    QVector<QVector<double>>& qdata = dm.getQCategorydata();  // NOTE:: its not qdata but qcategorydata
    for (n = 0;n<numVars;n++) {
       //qDebug() << "line check: " << qdata[n][j] << "\n";
       if ((qdata[n][j] >= y2) && (qdata[n][j] <= y1)) {
           linelist.push_back(n);
       }
    }
    SetLineListSelectedByIndex(linelist);

    //SetLineSelectedByIndex(linelist[0]);
}
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
void PlotWindow::clearLayout(QLayout *layout)
{
    QLayoutItem *item;
    while((item = layout->takeAt(0))) {
        if (item->layout()) {
            clearLayout(item->layout());
            delete item->layout();
        }
        if (item->widget()) {
            //QWidget *itis = item->widget();
            delete item->widget();
        }
        delete item;
    }
}


void PlotWindow::CreateButtonBox(bool pointsOption, bool showdataoption)
{

    horizontalGroupBox = new QGroupBox(tr("Selection"));
    //QGridLayout *buttonlayout = new QGridLayout;
    QHBoxLayout *buttonlayout = new QHBoxLayout;
    buttonlayout->setAlignment(Qt::AlignHCenter);

    QPushButton *removeVarButton = new QPushButton();
    removeVarButton->setText("Remove vars");
    const QSize btnSize = QSize(80, 25);   // w, h
    QFont bfont = removeVarButton->font();
    bfont.setPointSize(10);
    removeVarButton->setFont(bfont);
    removeVarButton->setFixedSize(btnSize);
    buttonlayout->addWidget(removeVarButton);

    QCheckBox* legendSelect = new QCheckBox();
    QFont chfont = legendSelect->font();
    chfont.setPointSize(11);
    legendSelect->setFont(chfont);
    legendSelect->setText("Legend");
    if (showLegend)
        legendSelect->setCheckState(Qt::Checked);
    buttonlayout->addWidget(legendSelect);

    QCheckBox* titleSelect = new QCheckBox();
    titleSelect->setFont(chfont);
    titleSelect->setText("Title");
    if (showTitle)
        titleSelect->setCheckState(Qt::Checked);
    buttonlayout->addWidget(titleSelect);

    QCheckBox* pointsSelect = new QCheckBox();
    if (pointsOption) {
        pointsSelect->setFont(chfont);
        pointsSelect->setText("Show Points");
        if (showPoints)
            pointsSelect->setCheckState(Qt::Checked);
        buttonlayout->addWidget(pointsSelect);
        QCheckBox* pointsOnlySelect = new QCheckBox();
        pointsOnlySelect->setText("Points Only");
        pointsOnlySelect->setFont(chfont);
        if (pointsOnly)
            pointsOnlySelect->setCheckState(Qt::Checked);
        buttonlayout->addWidget(pointsOnlySelect);

        connect(pointsOnlySelect,SIGNAL(stateChanged(int)),this,SLOT(PointsOnlySelect_StateChange(int)));
    }

    QCheckBox* altColorSelect = new QCheckBox();
    altColorSelect->setText("Alternate Colors");
    altColorSelect->setFont(chfont);
    if (UseAltColors)
        altColorSelect->setCheckState(Qt::Checked);
    buttonlayout->addWidget(altColorSelect);


    QLabel *combolabel = new QLabel("Groups");
    buttonlayout->addWidget(combolabel);

    // needs to be tied to number in colortable
    QComboBox *groupselect = new QComboBox();
    QString sel[NUM_IN_COLORTABLE] = {"1","2","3","4","5","6","7","8","9","10"};
    for (int i = 0;i<NUM_IN_COLORTABLE;i++) {
        groupselect->addItem(sel[i]);
    }
    QFont cbfont = groupselect->font();
    cbfont.setPointSize(10);
    groupselect->setFont(cbfont);
    groupselect->setCurrentIndex(numColorGroups-1);
    buttonlayout->addWidget(groupselect);

    if (showdataoption) {
        showDataButton = new QPushButton();
        showDataButton->setText("Show Data");

        showDataButton->setStyleSheet("QPushButton {background : rgb(200, 200, 200);}");

        const QSize btnSizev = QSize(80, 25);   // w, h
        showDataButton->setFixedSize(btnSizev);
        QFont bfont = showDataButton->font();
        bfont.setPointSize(10);
        showDataButton->setFont(bfont);
        buttonlayout->addWidget(showDataButton);
    }

    horizontalGroupBox->setLayout(buttonlayout);

    connect(removeVarButton,SIGNAL(clicked()),this,SLOT(removeVarButton_clicked()));
    connect(legendSelect,SIGNAL(stateChanged(int)),this,SLOT(LegendSelect_StateChange(int)));
    connect(titleSelect,SIGNAL(stateChanged(int)),this,SLOT(TitleSelect_StateChange(int)));
    connect(altColorSelect,SIGNAL(stateChanged(int)),this,SLOT(AltColorSelect_StateChange(int)));
    if (pointsOption)
        connect(pointsSelect,SIGNAL(stateChanged(int)),this,SLOT(PointsSelect_StateChange(int)));
    if (showdataoption)
       connect(showDataButton,SIGNAL(clicked()),this,SLOT(showDataSelect()));

    connect(groupselect,SIGNAL(currentIndexChanged(int)),this,SLOT(ComboBox_IndexChange(int)));

}
//--------------------------------------------------------------------------------
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void PlotWindow::titleDoubleClick(QMouseEvent* event)
{
 // Q_UNUSED(event)
                                                             //QPointer< AxisSpecDialog>  axisspec = new AxisSpecDialog();
  QPointer< QInputDialog > inputdlg= new QInputDialog();
  //QInputDialog *inputdlg = new QInputDialog();
  QObject::connect(inputdlg, &QInputDialog::finished, this, &PlotWindow::UserDialogClosed);
  //QObject::connect(inputdlg, &QInputDialog::accepted, this, &PlotWindow::UserDialogClosed);

  if (QCPTextElement *titleitem = qobject_cast<QCPTextElement*>(sender()))
  {
    // Set the plot title by double clicking on it
    bool ok;
    //QString newTitle = QInputDialog::getText(this, "QCustomPlot example", "New plot title:", QLineEdit::Normal, title->text(), &ok);
    QString newTitle = inputdlg->getText(this, "QCustomPlot example", "New plot title:", QLineEdit::Normal, titleitem->text(), &ok);
    if (ok)
    {
      title = newTitle;


    }
    //Display();
  }

}

void PlotWindow::UserDialogClosed(int r)
{
    dialogUp = false;
    Display();
}
void PlotWindow::removeVarButton_clicked()
{
    VarSelDialog* varwindow = new VarSelDialog();
    if (currenttype == DataManager::T_StandardPairs)
        varwindow->Display(1);
    else
        varwindow->Display(0);

    QObject::connect(varwindow, &QDialog::finished, this, &PlotWindow::UserDialogClosed);

    varwindow->show();
}
void PlotWindow::showDataSelect()
{
    if (showDataIsOn){
        showDataIsOn = false;
        showDataButton->setStyleSheet("QPushButton {background : rgb(200, 200, 200);}");
    } else {
        showDataIsOn = true;
        showDataButton->setStyleSheet("QPushButton {background : rgb(253, 70, 131);}");
    }
}
void PlotWindow::LegendSelect_StateChange(int state)
{
    if (state == 0)
        showLegend = false;
    else {
        showLegend = true;
    }
    Display();
}
void PlotWindow::TitleSelect_StateChange(int state)
{
    if (state == 0)
        showTitle = false;
    else {
        showTitle = true;
    }
    Display();
}
void PlotWindow::AltColorSelect_StateChange(int state)
{
    if (state == 0)
        UseAltColors = false;
    else {
        UseAltColors = true;
    }
    Display();
}
void PlotWindow::PointsSelect_StateChange(int state)
{
    if (state == 0)
        showPoints = false;
    else {
        showPoints = true;
    }
    Display();
}
void PlotWindow::PointsOnlySelect_StateChange(int state)
{
    if (state == 0)
        pointsOnly = false;
    else {
        pointsOnly = true;
    }
    Display();
}
void PlotWindow::ComboBox_IndexChange(int index)
{
    //int ngroups = index + 1;
    numColorGroups = index + 1;
    //qDebug() << "INDEX CHANGED " << index << "\n";
    DisplayPlot();
}
