#include "colortable.h"

#include <typeinfo>
//#include <QPen>

ColorTable::ColorTable()
{
}

QPen ColorTable::GetPen(int index)
{

    QPen pen = GetPen(index, ncolors);
    return pen;
}
QPen ColorTable::GetPen(int index, int groupsize)
{
    double scale = 255.;
    return GetPen(index, groupsize, scale);
}
QPen ColorTable::GetPen(int index, int groupsize, qreal scale)
{
    int n = index % groupsize;
    int linestyle = index / groupsize;

    int ir, ig, ib;

    if(UseAlternate) {
        ir = altvalues[n].r;
        ig = altvalues[n].g;
        ib = altvalues[n].b;
    }
    else {
        ir = values[n].r;
        ig = values[n].g;
        ib = values[n].b;
    }

    QPen pen = QPen(QColor(ir,ig,ib, scale));

    if (linestyle == 1)
        pen.setStyle(Qt::DashLine);
    else if (linestyle == 2)
        pen.setStyle(Qt::DotLine);
    else if (linestyle == 3)
        pen.setStyle(Qt::DashDotLine);
    else
     pen.setStyle(Qt::SolidLine);

    return pen;
}
