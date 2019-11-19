#include "surfacePlot.h"

#include <QtDataVisualization/QValue3DAxis>
#include <QtDataVisualization/Q3DTheme>
#include <QtGui/QImage>
#include <QtCore/qmath.h>

#include "datamanager.h"

// temp....
using namespace QtDataVisualization;


// end temp...

SurfacePlot::SurfacePlot(Q3DSurface *surface)
                 : m_graph(surface), dm(DataManager::getInstance())
{

    sampleCountX = 50;
    sampleCountZ = 50;

    sampleMin = -8.0f;
    sampleMax = 8.0f;

    m_graph->setAxisX(new QValue3DAxis);
    m_graph->setAxisY(new QValue3DAxis);
    m_graph->setAxisZ(new QValue3DAxis);


    //fillSqrtSinProxy();

}
SurfacePlot::~SurfacePlot()
{
    QSurfaceDataArray *dataArray = new QSurfaceDataArray;
    dataArray->reserve(sampleCountZ);  // may be small axis
}

void SurfacePlot::fillSqrtSinProxy()
{
    m_sqrtSinProxy = new QSurfaceDataProxy();
    m_sqrtSinSeries = new QSurface3DSeries(m_sqrtSinProxy);

    float stepX = (sampleMax - sampleMin) / float(sampleCountX - 1);
    float stepZ = (sampleMax - sampleMin) / float(sampleCountZ - 1);

    int dbgnz = sampleCountZ;
    int dbgnx = sampleCountX;

    QSurfaceDataArray *dataArray = new QSurfaceDataArray;
    dataArray->reserve(sampleCountZ);
    for (int i = 0 ; i < sampleCountZ ; i++) {
        QSurfaceDataRow *newRow = new QSurfaceDataRow(sampleCountX);
        // Keep values within range bounds, since just adding step can cause minor drift due
        // to the rounding errors.
        float z = qMin(sampleMax, (i * stepZ + sampleMin));
        int index = 0;
        for (int j = 0; j < sampleCountX; j++) {
            float x = qMin(sampleMax, (j * stepX + sampleMin));
            float R = qSqrt(z * z + x * x) + 0.01f;
            float y = (qSin(R) / R + 0.24f) * 1.61f;
            (*newRow)[index++].setPosition(QVector3D(x, y, z));
        }
        *dataArray << newRow;
    }
    minx = sampleMin;
    maxx = sampleMax;
    minz = sampleMin;
    maxz = sampleMax;
    miny = 0.0f;
    maxy = 2.0f;
    m_sqrtSinProxy->resetArray(dataArray);
}

void SurfacePlot::FillData()
{
    dataProxy = new QSurfaceDataProxy();
    dataSeries = new QSurface3DSeries(dataProxy);

    sampleCountX = dm.getNumPoints();
    sampleCountZ = dm.getNumGroups();

    maxz = static_cast<float>(dm.getMaxZ());
    minz = static_cast<float>(dm.getMinZ());
    minx = static_cast<float>(dm.getAllsurfXMin());
    maxx = static_cast<float>(dm.getAllsurfXMax());
    miny = 0.0f;
    maxy = 500.f;

    int ng = dm.getNumGroups();
    int np = dm.getNumPoints();

    QSurfaceDataArray *dataArray = new QSurfaceDataArray;
    dataArray->reserve(sampleCountZ);
    float x, y, z;

    for (int n = 0;n<ng;n++) {
        int index = 0;
        QSurfaceDataRow *newRow = new QSurfaceDataRow(np);
        z = static_cast<float>(dm.getqSurfaceZVal(n));
        for (int i = 0;i<np;i++) {
            x = static_cast<float>(dm.getQSurfaceXval(n,i));
            y = static_cast<float>(dm.getQSurfaceYval(n,i));
            (*newRow)[index++].setPosition(QVector3D(x, y, z));
        }
        *dataArray << newRow;
    }

    dataProxy->resetArray(dataArray);
}
void SurfacePlot::EnableModel(bool demo)
{
    if (demo) {
        m_sqrtSinSeries->setDrawMode(QSurface3DSeries::DrawSurfaceAndWireframe);
        m_sqrtSinSeries->setFlatShadingEnabled(true);
    } else {
        dataSeries->setDrawMode(QSurface3DSeries::DrawSurfaceAndWireframe);
        dataSeries->setFlatShadingEnabled(true);
    }


    m_graph->axisX()->setLabelFormat("%.1f");
    m_graph->axisZ()->setLabelFormat("%.1f");
    m_graph->axisX()->setRange(minx, maxx);
    m_graph->axisY()->setRange(miny, maxy);
    m_graph->axisZ()->setRange(minz, maxz);
    m_graph->axisX()->setLabelAutoRotation(30);
    m_graph->axisY()->setLabelAutoRotation(90);
    m_graph->axisZ()->setLabelAutoRotation(30);
    if (demo) {
        m_graph->addSeries(m_sqrtSinSeries);
    } else {
        m_graph->addSeries(dataSeries);
    }
}

