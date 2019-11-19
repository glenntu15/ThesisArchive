#include "varseldialog.h"
#include "ui_varseldialog.h"
#include "datamanager.h"
#include "qcustomplot.h"

#include <QDebug>


VarSelDialog::VarSelDialog(QWidget *parent) :
   QDialog(parent),  ui(new Ui::VarSelDialog), dm(DataManager::getInstance())

{
    ui->setupUi(this);
    mainLayout = nullptr;
    startdisplayindex = 0;
    Display(0);

}

VarSelDialog::~VarSelDialog()
{
    delete ui;
}

void VarSelDialog::clearLayout(QLayout *layout)
{
    QLayoutItem *item;
    while((item = layout->takeAt(0))) {
        if (item->layout()) {
            clearLayout(item->layout());
            delete item->layout();
        }
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }
   // startdisplayindex = 0;
}
void VarSelDialog::Display()
{

    if (mainLayout != nullptr){
        clearLayout(mainLayout);
        delete mainLayout;
    }

    numVars = dm.getNumVars();

    mainLayout = new QVBoxLayout;

    gridLayout = new QGridLayout;

    pTableWidget = new QTableWidget();
    pTableWidget->setColumnCount(2);
    pTableWidget->setRowCount(numVars);

    numLittlePlots = dm.getNumPairs();
    if (numLittlePlots == 0)
        numLittlePlots = dm.getNumVars();
    for (int i = 0;i<numLittlePlots;i++)
    {
        //std::string temp = dm.getVarName(i);
        QString str = dm.getQVarName(i); //QString::fromStdString(temp);
        QTableWidgetItem *item = new QTableWidgetItem(str);
        pTableWidget->setItem(i, 0, item);
        // now check box
        QTableWidgetItem *checkBoxItem = new QTableWidgetItem();
        checkBoxItem->data(Qt::CheckStateRole);
        if (dm.getOmitVar(i))
            checkBoxItem->setCheckState(Qt::Checked);
        else
            checkBoxItem->setCheckState(Qt::Unchecked);
        //qDebug() << "initial " << checkBoxItem->checkState() << endl;

        pTableWidget->setItem(i, 1, checkBoxItem);
        //QCheckBox *checkBox = new QCheckBox();
        //pTableWidget->setCellWidget(i,1, checkBox);
        checkBoxItem = pTableWidget->item(i,1);
        //qDebug() << "afterset " << checkBoxItem->checkState() << endl;
    }

    pTableWidget->resizeColumnsToContents();

    gridLayout->addWidget(pTableWidget, 0, 2);
    gridLayout->setColumnStretch(1, 50);
    gridLayout->setColumnStretch(2, 12);

    //OGlThumbnail * pThumbnails = new OGlThumbnail();

    // flag = 0 default
    if (layoutFlag == 0) {
        //
        // **  Create thumnnail plots of all vars against the others  **
        //
        BuildVarCatalog();

        QGridLayout *thumbnails = BuildThumbnailGrid();

        QGroupBox *thumbnailbox = new QGroupBox(tr("thumbnail group box"));

        thumbnailbox->setLayout(thumbnails);
        gridLayout->addWidget(thumbnailbox, 0, 1);



    }

    gridGroupBox = new QGroupBox(tr("grid group box"));
    gridGroupBox->setLayout(gridLayout);

    // ** now add buttons

    CreateButtonBox();

    mainLayout->addWidget(gridGroupBox);
    //mainLayout->addWidget(gridGroupBox);

    horizontalGroupBox->setFixedHeight(100);

    mainLayout->addWidget(horizontalGroupBox);
    setLayout(mainLayout);

}
//-------------------------------------------------------------------------
void VarSelDialog::BuildVarCatalog()
{
    double corrcoef;

    varcatalog.clear();

    //int numvars = dm.getNumVars();
    unsigned int unv = static_cast<unsigned>(numLittlePlots);
    for(unsigned int i = 0;i<unv;i++) {
        for (unsigned int j = i+1;j<numLittlePlots;j++) {
            corrcoef = GetCorrCoef(i,j);
            catentry xe(i,j, corrcoef);
            varcatalog.push_back(xe);
        }
    }
    std::sort(varcatalog.begin(), varcatalog.end());

}
//-------------------------------------------------------------------------
double VarSelDialog::GetCorrCoef(unsigned int ivarx, unsigned int ivary)
{
    std::vector<std::vector<double>>& data = dm.getData();
    std::size_t n = data[ivarx].size();
    //QVector<double> x(len);
    //QVector<double> y(len);
    double sum_x = 0., sum_y = 0., sum_xy = 0., sum_xs = 0., sum_ys = 0.;
    double x,y, rt, rb, r;

    for (std::size_t i = 0;i<n;i++) {
        x = data[ivarx][i];
        y = data[ivary][i];
        sum_x = sum_x + x;
        sum_y = sum_y + y;
        sum_xs = sum_xs + (x * x);
        sum_ys = sum_ys + (y * y);
        sum_xy = sum_xy + (x * y);

    }
    double dn = static_cast<double>(n);
    rt = dn * sum_xy - (sum_x * sum_y);
    rb = (dn*sum_xs - (sum_x * sum_x)) * (dn*sum_ys - (sum_y*sum_y));
    r = rt / sqrt(rb);
    return r;
}
//-------------------------------------------------------------------------
#define NUMBOXES 3
QGridLayout* VarSelDialog::BuildThumbnailGrid()
{
    int iv,jv;
    double r;
    QGridLayout *thumbnails = new QGridLayout();
    std::size_t index = startdisplayindex;
    std::size_t l = varcatalog.size();

    QCustomPlot *customPlotXY;

    int irow = 0;
    int icol = 0;
    while (index < l) {
        iv = varcatalog[index].i1;
        jv = varcatalog[index].i2;
        if ( (dm.getOmitVar(iv)) || (dm.getOmitVar(jv)) ) {
            index++;
            continue;
        }
        r = varcatalog[index].corrcoef;
        customPlotXY = CreateLittlePlot(iv,jv, r);
        thumbnails->addWidget(customPlotXY,irow,icol);
        index++;
        icol++;
        if (icol >= NUMBOXES) {
            icol = 0;
            irow++;
            if(irow >= NUMBOXES)
                break;
        }
    }
    return thumbnails;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
QCustomPlot* VarSelDialog::CreateLittlePlot(unsigned int ivarx, unsigned int ivary, double r)
{

    bool dummy = false;
    QCustomPlot *customPlotXY = new QCustomPlot();

    if ((ivarx < 0) || (ivary < 0))
        dummy = true;

    QString textx = "X-axis";
    QString texty = "Y-axis";
    QString title = "No Plot";
    if (!dummy) {
        title = "r = " + QString::number(r);
    }

    customPlotXY->plotLayout()->insertRow(0);
    customPlotXY->plotLayout()->addElement(0, 0, new QCPTextElement(customPlotXY, title, QFont("sans", 10, QFont::Normal)));

    std::string temp;

    if (!dummy) {
        //temp = dm.getVarName(ivarx);
        //textx = QString::fromStdString(temp);
        textx = dm.getQVarName(ivarx);
        //temp = dm.getVarName(ivary);
        //texty = QString::fromStdString(temp);
        texty = dm.getQVarName(ivary);
    }


    QVector<QVector<double>>& qnormaldata = dm.getQNormaldata();

    int len = qnormaldata[ivarx].size();
    QVector<double> x(len);
    QVector<double> y(len);

    if (!dummy) {

        for (int i = 0;i<len;i++) {
            x[i] = qnormaldata[ivarx][i];
            y[i] = qnormaldata[ivary][i];
        }
    }
    else {
        //int len = 1;
        x[0] = 0.;
        y[0] = 0.;

    }


    customPlotXY->addGraph();
    customPlotXY->graph(0)->setData(x, y);
    customPlotXY->graph(0)->setLineStyle((QCPGraph::LineStyle)0);
    customPlotXY->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 2));

    customPlotXY->xAxis->setLabel(textx);
    customPlotXY->xAxis->setRange(0, 1.0);

    customPlotXY->yAxis->setLabel(texty);
    customPlotXY->yAxis->setRange(0, 1.0);

    return customPlotXY;

}
//-------------------------------------------------------------------------
void VarSelDialog::CreateButtonBox()
{

    horizontalGroupBox = new QGroupBox(tr("Action"));
    QHBoxLayout *layout = new QHBoxLayout;

    prevButton = new QPushButton();
    prevButton->setText("Previous");
    layout->addWidget(prevButton);
    prevButton->setEnabled(false);

    nextButton = new QPushButton();
    nextButton->setText("Next");
    layout->addWidget(nextButton);
    //nextButton->setEnabled(false);

    oKButton = new QPushButton();
    oKButton->setText("OK ");
    layout->addWidget(oKButton);
    resetButton = new QPushButton();
    resetButton->setText("Reset");
    layout->addWidget(resetButton);


    horizontalGroupBox->setLayout(layout);

    connect(oKButton,SIGNAL(clicked()),this,SLOT(oKButton_clicked()));
    connect(resetButton,SIGNAL(clicked()),this,SLOT(resetButton_clicked()));
    connect(nextButton,SIGNAL(clicked()),this,SLOT(nextButton_clicked()));
    //connect(bothButton,SIGNAL(clicked()),this,SLOT(bothRadioButton_clicked()));
}

void VarSelDialog::oKButton_clicked()
{
    //int count = 0;

    //qDebug() << "going through remaining rows";
    for (int i = 0;i<numLittlePlots;i++) {
       // QWidget *pWidget = pTableWidget->cellWidget(i,1);
       // QCheckBox *checkbox = pWidget->findChild<QCheckBox *>();
      //  if (checkbox->isChecked())
      //      count++;
        QPoint h(i,0);
        //QTableWidgetItem *item = pTableWidget->item(i,0);
        //qDebug() << item->text();
        QPoint h2(i,1);
        QTableWidgetItem *checkBoxItem = pTableWidget->item(i,1);
        if (checkBoxItem->checkState() == Qt::Checked) {
            //count++;
            dm.setOmitVar(i);
        }
        else {
            dm.clearOmitVar(i);
        }

        //qDebug() << " checkstate " << checkBoxItem->checkState() << endl
    }
    //qDebug() <<  " count 1 " << count << endl;

    emit QDialog::finished(0);
    close();


}
void VarSelDialog::resetButton_clicked()
{
    dm.ClearOmited();

    close();
}
void VarSelDialog::nextButton_clicked()
{
    startdisplayindex = startdisplayindex + 9;
    Display();
}
