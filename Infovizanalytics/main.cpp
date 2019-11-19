#include "mainwindow.h"
#include <QApplication>
#include "datamanager.h"
#include <string>

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    //QApplication::setAttribute(Qt::AA_UseDesktopOpenGL);

    QApplication a(argc, argv);

    //std::string filename = "irisData.csv";
    std::string filename = "metadatatest.csv";
    //std::string filename = "RSE29_ScatterNormIntenseCycle_Data.csv";
    //std::string filename = "RSE29_ScatterNormWmetaCycle_Data.csv";
    DataManager &dm = DataManager::getInstance();
    dm.ReadFile(filename);
    MainWindow w;
    w.SetOpenFileName(filename);
    w.SetUpButtonsandPlottype();
    w.show();

    return a.exec();
}
