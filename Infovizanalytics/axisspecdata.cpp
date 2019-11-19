#include "axisspecdata.h"

AxisSpecData::AxisSpecData() :dm(DataManager::getInstance())
{
    currenttype = dm.getDST();
    isParallelCoords = false;
    //dstypes { T_standard, T_surface, T_datagroup_series, T_datagroup_parallel };
    isXAxis = false;
    isYAxis = false;

}
//=================================================================================
void AxisSpecData::SetDataX()
{
    if ((currenttype == DataManager::T_datagroup_series) || (currenttype == DataManager::T_datagroup_parallel)
            || (currenttype == DataManager::T_dataclass_parallel)) {
        axislen = dm.getNumGroups();
        firstv = 0;
        deltav = 1.0;
        // make a copy -- may want to modify later
        for (int i = 0;i<axislen;i++) {
            categoryaxistics.push_back(dm.getCatagoryLabel(i));
            if ((currenttype == DataManager::T_datagroup_parallel) || (currenttype == DataManager::T_dataclass_parallel) )
                catagorymaxlabels.push_back(dm.getCategoryMaxLabel(i));
        }
    } else if ((currenttype == DataManager::T_standard) || (currenttype == DataManager::T_StandardPairs) || (currenttype == DataManager::T_AugmentedPairs)) {

        double minx = dm.getVarMin(0);
        double maxx = dm.getVarMax(0);
        double minv;
        axislen = 5;
        firstv = 0.0;
        double sign = 1.0;
        if (minx < 0.)
            sign = -1.0;
        minv = fabs(minx);
        if (minv > 1.e-5){
            minv = getRoundNumber(minv) * sign;
            firstv = minv;
        }

        double maxv = getRoundNumber(maxx); // zeroth variable, first column is independent
        deltav = (maxv - firstv) / static_cast<double>(axislen);

    }    else {
        axislen = 5;
        firstv = 0.0;
        deltav = 1.0;
    }
    isXAxis = true;
    range = firstv + deltav * static_cast<double>(axislen);
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void AxisSpecData::SetDataY()
{
    // all max and all min exclude the first variable
    double miny = dm.getAllMin();
    double maxy = dm.getAllMax();
    double minv;

    if ((currenttype == DataManager::T_standard) || (currenttype == DataManager::T_StandardPairs) || (currenttype == DataManager::T_AugmentedPairs)) {
        axislen = 5;
        firstv = 0.0;
        double sign = 1.0;

        if (miny < 0.)
            sign = -1.0;
        minv = fabs(miny);
        if (minv > 1.e-5){
            minv = getRoundNumber(minv);
            firstv = minv * sign;
        }

        double maxv = getRoundNumber(maxy); // zeroth variable, first column is independent
        deltav = (maxv - firstv) / static_cast<double>(axislen);
    } else {
        if (miny < 0.0) {
            firstv = -1.0;
            deltav = 0.2;
            axislen = 5;
        } else {
            firstv = 0.0;
            axislen = 5;
            deltav = (maxy- firstv) / axislen;
        }

    }
    isYAxis = true;
    range = firstv + deltav * static_cast<double>(axislen);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void AxisSpecData::OverRideData(double fv, double lv)
{
    if ((currenttype == DataManager::T_standard) || (currenttype == DataManager::T_StandardPairs) || (currenttype == DataManager::T_AugmentedPairs)) {
        double delv = lv - fv;
        axislen = delv / deltav;
        deltav = delv / axislen;
        range = lv;
        firstv = fv;
    }

}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void AxisSpecData::SetDataYParallel()
{
    isParallelCoords = true;
    axisLabel = "Fraction";
    firstv = 0.0;
    deltav = 0.2;
    axislen = 5;
    range = firstv + deltav * static_cast<double>(axislen);
    isYAxis = true;
}
//=================================================================================
double AxisSpecData::getRoundNumber(double v) {
    double rval, factor;

    if (v <= 0.1) {
        rval = 0.1;
        return rval;
    }
    if (v <= 1.) {
        rval = 1.0;
        return rval;
    }
    if (v < 10.){
        rval = static_cast<double>(int(v + 0.5));
        return rval;
    }
    if (v < 100.) {
        rval = 100.;
        factor = ceil(v / 10.);
        rval = factor * 10.;
        return rval;
    }
    if (v < 1000.) {
        rval = 100.;
        factor = ceil(v / 100.);
        rval = factor * 100.;
        return rval;
    }
    if (v < 10000.) {
        rval = 100.;
        factor = ceil(v / 1000.);
        rval = factor * 1000.;
        return rval;
    }
    return v;
}

