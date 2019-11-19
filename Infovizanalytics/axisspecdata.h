#ifndef AXISSPECDATA_H
#define AXISSPECDATA_H
#include <qstring.h>
#include <qvector>
#include "datamanager.h"


class AxisSpecData
{
public:
    explicit AxisSpecData();
    ~AxisSpecData();

    void SetDataX();
    void SetDataY();
    void SetDataYParallel();
    void OverRideData(double firstv, double lastv);

    QVector<QString> categoryaxistics;
    QVector<QString> catagorymaxlabels;
private:

    DataManager &dm;
    DataManager::dstypes currenttype;

    double getRoundNumber(double v);



public:

    bool isCategoryAxis;
    bool isoveride;
    double firstv;
    double deltav;
    int axislen;
    QString axisLabel;
    double range;

    bool isParallelCoords;
    bool isXAxis;
    bool isYAxis;

};

#endif // AXISSPECDATA_H
