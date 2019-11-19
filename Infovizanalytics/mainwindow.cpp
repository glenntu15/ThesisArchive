#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "plotwindow.h"
#include "datamanager.h"
#include "threedview.h"
#include "dataviewdialog.h"

#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QTextEdit>
//#include <QtCharts/QChartView>
//#include <QtCharts/QLineSeries>
//#include <QtCharts/QCategoryAxis>

static const char message[] =
    "<p><b>Qt Main Window Example</b></p>"

    "<p>This is a demonstration of the QMainWindow, QToolBar and "
    "QDockWidget classes.</p>"

    "<p>The tool bar and dock widgets can be dragged around and rearranged "
    "using the mouse or via the menu.</p>"

    "<p>Each dock widget contains a colored frame and a context "
    "(right-click) menu.</p>"

#ifdef Q_OS_MAC
    "<p>On OS X, the \"Black\" dock widget has been created as a "
    "<em>Drawer</em>, which is a special kind of QDockWidget.</p>"
#endif
    ;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //ui->setupUi(this);
    Q_UNUSED(message);
    setObjectName("MainWindow");
    setWindowTitle("Qt Main Window Example");

    setupMenuBar();
    enabledualcharts = false;

    //mainLayout = new QVBoxLayout;
    QHBoxLayout *layout = new QHBoxLayout(this);

    // layouts

    gridLayout = new QGridLayout;
    gridGroupBox = new QGroupBox(tr(""));
    gridGroupBox->setLayout(gridLayout);
    gridGroupBox->setMinimumHeight(250);

    horizontalGroupBox = new QGroupBox(tr("Chart Type Selection"));
    horizontalGroupBox->setLayout(layout);

    //filenamebox->setReadOnly(true);
   // filenamebox->setAlignment(Qt::AlignLeft);
   // filenamebox->setMaxLength(370);
    filenamelabel = new QLabel(Openfilename);
    gridLayout->addWidget(filenamelabel);

    plotButton = new QPushButton();
    plotButton->setText("Plot");
    layout->addWidget(plotButton);
    plotButton->setEnabled(false);

    barPlotButton = new QPushButton();
    barPlotButton->setText("BarPlot");
    layout->addWidget(barPlotButton);
    barPlotButton->setEnabled(false);

    bothPlotButton = new QPushButton();
    bothPlotButton->setText("Line+Bar");
    layout->addWidget(bothPlotButton);
    bothPlotButton->setEnabled(false);

    plot3DButton = new QPushButton();
    plot3DButton->setText("3D Plot");
    layout->addWidget(plot3DButton);
    plot3DButton->setEnabled(false);


    connect(plotButton,SIGNAL(clicked()),this,SLOT(on_plotButton_clicked()));
    connect(barPlotButton,SIGNAL(clicked()),this,SLOT(on_barPlotButton_clicked()));
    connect(bothPlotButton,SIGNAL(clicked()),this,SLOT(on_bothPlotButton_clicked()));
    connect(plot3DButton,SIGNAL(clicked()),this,SLOT(on_3DButton_clicked()));

    horizontalGroupBox->setFixedHeight(100);
    horizontalGroupBox->setFixedWidth(500);

    gridLayout->addWidget(horizontalGroupBox,1,0);
   // mainLayout->addWidget(centralWidget);

   // //setLayout(mainLayout);
   setCentralWidget(gridGroupBox);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupMenuBar()
{
    QMenu *menu = menuBar()->addMenu(tr("&File"));
    menu->addAction(tr("Open"), this, &MainWindow::on_actionFileOpen_triggered);
}

void MainWindow::on_actionFileOpen_triggered()
{
    std::string currentFile;
    DataManager& dm = DataManager::getInstance();

    QString fileName = QFileDialog::getOpenFileName(this, "Open the file");
    //QFile file(fileName);
    Openfilename = fileName;
    filenamelabel->setText(Openfilename);
    this->filenamelabel->repaint();
    setWindowTitle(fileName);

     currentFile = fileName.toLocal8Bit().constData();
     int success = dm.ReadFile(currentFile);
     if (success == 0) {
         QMessageBox::warning(this, "Error", "Cannot read this file: ");
         return;
     }
     if (success < 0) {
         QMessageBox::warning(this, "Error", "Unable to open file: ");
         return;
     }

     SetUpButtonsandPlottype();

}
void MainWindow::on_plotButton_clicked()
{
    DataManager& dm = DataManager::getInstance();
    DataManager::dstypes currentType = dm.getDST();

    if (currentType != DataManager::T_surface) {
        PlotWindow *pwin = new PlotWindow();
        pwin->Display();
        pwin->show();
    } else {
        //ThreeDView* viewshower = new ThreeDView();
       // viewshower->Display(false);
    }
}
//----------------------------------------------------------------------
void MainWindow::on_barPlotButton_clicked()
{
    DataManager& dm = DataManager::getInstance();
    DataManager::dstypes currentType = dm.getDST();

    if (currentType != DataManager::T_surface) {
        PlotWindow *pwin = new PlotWindow();
        pwin->SetChartType(PlotWindow::T_Bar);
        pwin->Display();
        pwin->show();
    } else {
        //ThreeDView* viewshower = new ThreeDView();
       // viewshower->Display(false);
    }
}
//----------------------------------------------------------------------
void MainWindow::on_bothPlotButton_clicked()
{
    DataManager& dm = DataManager::getInstance();
    DataManager::dstypes currentType = dm.getDST();

    if (currentType != DataManager::T_surface) {
        PlotWindow *pwin = new PlotWindow();
        pwin->SetDualEnabled(enabledualcharts);
        pwin->SetDualCharts();
        pwin->Display();
        pwin->show();
    } else {
        //ThreeDView* viewshower = new ThreeDView();
       // viewshower->Display(false);
    }
}
void MainWindow::on_3DButton_clicked()
{
    QString name = "Name";
    ThreeDView* viewshower = new ThreeDView();
    viewshower->Display(true);

}
void MainWindow::SetUpButtonsandPlottype()
{
    DataManager& dm = DataManager::getInstance();
    DataManager::dstypes currenttype;
    PlotWindow::ChartTypes chartType;

    currenttype = dm.getDST();
    chartType = PlotWindow::T_Line;  // 1 for bar

    plotButton->setEnabled(false);
    barPlotButton->setEnabled(false);
    bothPlotButton->setEnabled(false);

    filenamelabel->setText(Openfilename);
    this->filenamelabel->repaint();

    if (currenttype == DataManager::T_datagroup_series) {

        chartType = PlotWindow::T_Line;   //T_Bar;
        plotButton->setEnabled(true);
        int nvars = dm.getNumVars();
        if (nvars <=5) {
            enabledualcharts = true;
            barPlotButton->setEnabled(true);
            bothPlotButton->setEnabled(true);
        }
    } else if ((currenttype == DataManager::T_datagroup_parallel) || (currenttype == DataManager::T_dataclass_parallel)) {

        chartType = PlotWindow::T_ParallelCoords;
        plotButton->setEnabled(true);
    }
    else if (currenttype != DataManager::T_unknown){
        plotButton->setEnabled(true);
    }
}
