#ifndef THREEDVIEW_H
#define THREEDVIEW_H

#include "datamanager.h"
#include <QDialog>

class SurfacePlot;

namespace Ui {
class ThreeDView;
}

class ThreeDView
{

public:
    explicit ThreeDView();
    ~ThreeDView();
    void Display(bool demo);

private:

    //DataManager &dm;

    QWidget *container;
    SurfacePlot *psurface;

    QVBoxLayout *mainLayout;
    QGroupBox *horizontalGroupBox;
    QGroupBox *gridGroupBox;
    QGridLayout *gridLayout;


    enum chartTypes {T_Line, T_Scatter, T_Bar, T_ParallelCoords, T_Spectograph, T_Surface };
    chartTypes chartType;
    DataManager::dstypes currenttype;

};

#endif // THREEDVIEW_H
