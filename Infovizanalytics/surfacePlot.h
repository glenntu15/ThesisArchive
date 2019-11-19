#ifndef SURFACE_H
#define SURFACE_H

#include <QtDataVisualization/Q3DSurface>
#include <QtDataVisualization/QSurfaceDataProxy>
#include <QtDataVisualization/QHeightMapSurfaceDataProxy>
#include <QtDataVisualization/QSurface3DSeries>
#include <QtWidgets/QSlider>

class DataManager;

using namespace QtDataVisualization;


class SurfacePlot : public QObject
{
     Q_OBJECT

public:
    SurfacePlot(Q3DSurface *surface);
    ~SurfacePlot();

    void EnableModel(bool demo);
    void FillData();
    void fillSqrtSinProxy();

private:

    Q3DSurface *m_graph;

    DataManager &dm;

    int sampleCountX;
    int sampleCountZ;
    float sampleMin;
    float sampleMax;
    float minx;
    float maxx;
    float miny;
    float maxy;
    float minz;
    float maxz;


    QSurfaceDataProxy *m_sqrtSinProxy;
    QSurface3DSeries *m_sqrtSinSeries;

    QSurfaceDataProxy *dataProxy;
    QSurface3DSeries *dataSeries;



};

#endif // SURFACE_H
