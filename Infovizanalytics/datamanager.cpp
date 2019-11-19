#include "DataManager.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef USING_QT
#include <QDebug>
#endif

/// deafault constructor;
DataManager::DataManager()
{
    numVars = 0;
    numPairs = 0;
    numGroups = 0;
    numPoints = 0;
    omitvar = nullptr;
    NaN = nan("");
    numMetaCollumns = 0;
    currentType = T_unknown;
}
//-----------------------------------------------------------------------------
DataManager::~DataManager()
{
}
#ifdef MINGW
#pragma GCC diagnostic ignored "-Wformat"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#else
#pragma warning(disable:4018)
#define _CRT_SECURE_NO_WARNINGS
#endif
//-----------------------------------------------------------------------------
//  Returns -1, unable to open, 0 no data or unexpected data, 1 on success
int DataManager::ReadFile(std::string filename)
{
    if ((numGroups > 0) || (numVars > 0)) {
        Reset();
    }
	int status = 0;
//	int len = 0;
    currentType = T_unknown;				// set default

	std::string line;
	std::vector<std::string> slist;

    std::ifstream infile(filename);

	if (!infile.is_open()) {
		status = -1;
		return status;
	}
	
	char * str;
	const char *entry;
	int isequal = -99;

	std::getline(infile, line);
	str = const_cast<char*>(line.c_str());
    //myfile << " line: " << line << "\n";
    //myfile.close();


    currentType = T_unknown;
	int nread = Split(slist, str);
	for (int i = 0; i < nread; i++) {
		entry = const_cast<char*>(slist[i].c_str());
		isequal = strncmp("DSN", entry, 3);
		if (isequal == 0) {
			if (i <= nread) {
                dsname = slist[i + 1]; // save dsname
                qdsname = QString::fromStdString(dsname);
				continue;	//next iteration will be nothing
			}
		}
		
		isequal = strncmp("DST", entry, 3);

		if (isequal == 0) {
			entry = const_cast<char*>(slist[i+1].c_str());
            isequal = strcmpi("standard", entry);  // case insensitive
			if (isequal == 0) {
				currentType = T_standard;
                continue;
			}
            //entry = const_cast<char*>(slist[i+1].c_str());
            isequal = _strcmpi("standardpairs", entry);  // case insensitive
            if (isequal == 0) {
                currentType = T_StandardPairs;
                continue;;
            }
            isequal = _strcmpi("augmentedpairs", entry);  // case insensitive
            if (isequal == 0) {
                currentType = T_AugmentedPairs;
                continue;;
            }
            isequal = _strcmpi("datagroupseries", entry);
			if (isequal == 0) {
				currentType = T_datagroup_series;
				break;
			}
            isequal = _strcmpi("datagroupparallel", entry);
            if (isequal == 0) {
                currentType = T_datagroup_parallel;
                continue;
            }
            isequal = _strcmpi("dataclassparallel", entry);
            if (isequal == 0) {
                currentType = T_dataclass_parallel; //T_dataclass_parallel
                continue;
            }
            isequal = _strcmpi("surface", entry);
            if (isequal == 0) {
                currentType = T_surface;
                continue;
            }
            if (currentType == T_unknown)
                return 0;

		}
        //isequal = strncmp("PLOT", entry, 4);
        //if (isequal == 0) {
        //}
        isequal = strcmpi("MARKS",entry);
        if (isequal == 0) {
            entry = const_cast<char*>(slist[i+1].c_str());
            std::string mfilename(entry);
            status = ReadMarksData(mfilename);
        }
        isequal = strcmpi("METADATA",entry);
        if (isequal == 0) {
            entry = const_cast<char*>(slist[i+1].c_str());
            std::string mfilename(entry);
            numMetaCollumns = std::stoi(entry);
            //integer number of metadata colums
        }
	}
	// 
	// Now type and name are known
	// 
	status = -1;
    if ((currentType == T_datagroup_series) || (currentType == T_datagroup_parallel) || (currentType == T_dataclass_parallel))  {

		status = ReadDataGroupSeries(infile);
        //return status;
	}
    if ((currentType == T_standard)) {
        status = ReadMultiSeries(infile);
        if (numVars < 2) {
            qDebug() << "error on input -- too few variables\n";
            status = -1;
            return status;
        }
    }
    if ((currentType == T_AugmentedPairs) || (currentType == T_StandardPairs)) {
        status = ReadPairsSeries(infile);

    }
    if (currentType == T_surface) {
        status = ReadSurface(infile);
        if (numVars < 2) {
            qDebug() << "error on input -- too few variables\n";
            status = -1;
            return status;
        }
    }

    //if (currentType != T_surface)
        ComputeNormalData();

    omitvar = new bool[numVars];
    for (int i = 0;i<numVars;i++)
        omitvar[i] = false;
	

	return status;
}

//-----------------------------------------------------------------------------
// Note This function should only be called after data is already read and new
//      data will be replacing it.
//
void DataManager::Reset()
{
    //unsigned int len;
    int lim = qdata.size() - 1;
    for (unsigned int n = 0;n<numVars;n++) {
        data[n].clear();

        //len = data[n].size();
#ifdef USING_QT
        if (n >= lim){
            qdata[n].clear();
            qnormaldata[n].clear();
        }


       // qnormaldata[n].clear();
#endif 
    }

    for (int n=0;n<numPairs;n++) {
        pdata[n].clear();
        metadata[n].clear();
    }
    if (numPairs > 0) {
        numPairs = 0;
        pdata.clear();
    }

    for (unsigned int n = 0;n < static_cast<unsigned>(numMarks); n ++) {
        marksdata[n].clear();
    }
    data.clear();
    marksdata.clear();
    varMax.clear();
    varMin.clear();
    surfXMax.clear();
    surfXMin.clear();
    metadata.clear();

    surfaceZVals.clear();
    surfacexdata.clear();
    surfaceydata.clear();
#ifdef USING_QT
    qdata.clear();

    qvarnames.clear();

    categoryMax.clear();
    qcategoryMaxLabels.clear();
    qcategorydata.clear();
    qnormaldata.clear();

    qsurfaceZVals.clear();
    qsurfacexdata.clear();
    qsurfaceydata.clear();
#endif
	categoryLabels.clear();
    numGroups = 0;
    numVars = 0;
    numMetaCollumns= 0;
    numMarks = 0;
    dsname = "";
    allMax = -1.e15;
    allMin = 1.e15;
    if (omitvar != nullptr) {
        delete [] omitvar;
        omitvar = nullptr;
    }
}

void DataManager::ClearOmited()
{
    if (omitvar == nullptr)
        return;

    for (int i = 0;i < numVars;i++) {
        omitvar[i] = false;
    }
}

//-----------------------------------------------------------------------------
/// This function not only finds max and min of each variable but finds
/// max and min for each category.  For "datagroup_parallel" it generates text
/// for tics above the plot area
///
//-----------------------------------------------------------------------------
void DataManager::ComputeNormalData()
{
    unsigned int len;
    std::string temp; //debug
    if (currentType != T_surface) {
        if (numVars > 0){

            for(unsigned int n = 0;n<numVars;n++) {
                len = data[n].size();
                double var = data[n][0];
                varMax.push_back(var);
                varMin.push_back(var);
                //temp = getVarName(n);
                //QString texttemp = QString::fromStdString(temp);
                //qDebug() << texttemp << "\n";
                for (unsigned int i = 1;i<len;i++) {
                    //qDebug() << data[n][i] << endl;
                    if (data[n][i] != NaN) {
                        if (data[n][i] > varMax[n])
                            varMax[n] = data[n][i];
                        if (data[n][i] < varMin[n])
                            varMin[n] = data[n][i];
                    }

                }
                //qDebug() << "varmax " << n << " " << varMax[n] << " varmin " << varMin[n] << endl;
            }
        }
        // The first is the "x" axis variable
        int istart = 0;
        if (currentType == T_standard) {
            allMax =  varMax[1];
            istart = 1;
        }
        else {
            allMax =  varMax[0];
        }

        allMin = allMax;
        //
        for(unsigned int n = istart;n<numVars;n++) {

            if (varMax[n] > allMax)
                allMax = varMax[n];
            if (varMin[n] < allMin)
                allMin = varMin[n];
        }
    //
    // surface stuff
    //
    } else {        // process surface data
        for (int n = 0;n<numGroups;n++) {
            double var = qsurfacexdata[n][0];
            surfXMax.push_back(var);  // for 1..n x
            surfXMin.push_back(var);
            var = qsurfaceydata[n][0];
            varMax.push_back(var);  // for 1..n y
            varMin.push_back(var);
            for (int i = 1;i<numPoints;i++) {
                if (qsurfacexdata[n][i] > surfXMax[n])
                    surfXMax[n] = qsurfacexdata[n][i];
                if (qsurfacexdata[n][i] < surfXMin[n])
                    surfXMin[n] = qsurfacexdata[n][i];

                if (qsurfaceydata[n][i] > varMax[n])
                    varMax[n] = qsurfaceydata[n][i];
                if (qsurfaceydata[n][i] < varMin[n])
                    varMin[n] = qsurfaceydata[n][i];
            }

        }
        surfZMax = qsurfaceZVals[0];
        surfZMin = qsurfaceZVals[0];
        allsurfXMax = surfXMax[0];
        allsurfXMin = surfXMax[0];
        for (int n = 1;n<numGroups;n++) {
            if (varMax[n] > allMax)
                allMax = varMax[n];
            if (varMin[n] < allMin)
                allMin = varMin[n];

            if (qsurfaceZVals[n] > surfZMax)
                surfZMax = qsurfaceZVals[n];
            if (qsurfaceZVals[n] < surfZMin)
                surfZMin = qsurfaceZVals[n];

            if (surfXMax[n] > allsurfXMax)
                allsurfXMax = surfXMax[n];
            if (surfXMin[n] < allsurfXMin)
                allsurfXMin = surfXMin[n];
        }

    }


    // for type standard we take max of all variables but the first
    // The first is the "x" axis variable
    int istart = 0;
    if (currentType == T_standard) {
        allMax =  varMax[1];
        istart = 1;
    }
    else {
        allMax =  varMax[0];
    }

    allMin = allMax;
    //
    for(unsigned int n = istart;n<numVars;n++) {

        if (varMax[n] > allMax)
            allMax = varMax[n];
        if (varMin[n] < allMin)
            allMin = varMin[n];
    }

#ifdef USING_QT
//   Compute normal for each varaiable -- only applies to QT
    QVector<double> datalist;
    double x, range;

    for(unsigned int n = 0;n<numVars;n++) {
       //temp = getVarName(n);
       range = varMax[n] - varMin[n];
       //double vm = varMax[n];
       //double vmin = varMin[n];
       len = data[n].size();
       datalist.clear();
       for (unsigned int i = 1;i<len;i++) {

           x = (data[n][i] - varMin[n]) / range;
           datalist.push_back(x);
       }
       qnormaldata.push_back(datalist);
    }
//
// Now Category axis stuff -- only applies to QT
// compute the normal for each category
//
    if ((currentType == T_datagroup_parallel) || (currentType == T_dataclass_parallel)) {
        for (unsigned int ic = 0;ic<numGroups;ic++) {
            categoryMax.push_back(qdata[0][ic]);
            for (int i = 0;i<numVars;i++) {
                if (qdata[i][ic]>categoryMax[ic])
                    categoryMax[ic] = qdata[i][ic];
            }
        }
        QVector<double> datalist;
        for (unsigned int i = 0;i<numVars;i++) {
            datalist.clear();
            for (int ic = 0;ic<numGroups;ic++) {
                double scaled = qdata[i][ic]/categoryMax[ic];
                datalist.push_back(scaled);
            }
            qcategorydata.push_back(datalist);
        }
    // now set labels
        for (int i = 0;i<numGroups;i++) {
            QString label = QString::number(categoryMax[i],'G', 4);
           //qDebug() << label;
            qcategoryMaxLabels.push_back(label);
        }
    }
#endif
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//-----------------------------------------------------------------------------
int DataManager::Split(std::vector<std::string> &slist, char * line)
{
    int len = 0;
    slist.clear();  // always enpty the vector deleting all contents
    char *pch = line;
	pch = strtok(pch, ",");
	len = strlen(pch);
	std::string temp(pch, len);
	slist.push_back(temp);
    do {
        pch = strtok(nullptr, ",");
		if (pch != nullptr) {
			len = strlen(pch);
			std::string temp(pch, len);
			slist.push_back(temp);
		}
    } while (pch != nullptr);
    
    len = static_cast<unsigned>(slist.size());

    return len;
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string trim(const std::string& str,
	const std::string& whitespace = " \t")
{
	const auto strBegin = str.find_first_not_of(whitespace);
	if (strBegin == std::string::npos)
		return ""; // no content

	const auto strEnd = str.find_last_not_of(whitespace);
	const auto strRange = strEnd - strBegin + 1;

	return str.substr(strBegin, strRange);
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  builds the 2D array data, where each element of data is a vector
//  corresponding to a variable
//-----------------------------------------------------------------------------
int DataManager::ReadDataGroupSeries(std::ifstream &infile)
{
	int status = -1;
	std::string line;
	char* str;
	const char *entry;

    qdata.reserve(60);
    data.reserve(100);

	std::vector<std::string> slist;
	int nread;
	// read header line -- this contains the series labels
	std::getline(infile, line);
	str = const_cast<char*>(line.c_str());
	nread = Split(slist, str);
	int count = 0;
    for (int i = 0; i < nread; i++) {
		if (slist[i].length() > 0) {
			serieslables[count] = slist[i];
            count = count + 1;
		}
	}
    // generate a QVector of QStrings for the series labels
    numGroups = count;
    for (int i = 0;i<numGroups;i++) {
        
#ifdef USING_QT
		std::string temp = serieslables[i];
        QString str = QString::fromStdString(temp);
        categoryLabels.push_back(str);
#else
		categoryLabels.push_back(serieslables[i]);
#endif
    }

	status = 0;
	// Now read the data
    double x;
	int nvars = 0;
	while (std::getline(infile, line)) {
		str = const_cast<char*>(line.c_str());
		nread = Split(slist, str);
		varnames[nvars] = slist[0];
#ifdef USING_QT
        QString str = QString::fromStdString(varnames[nvars]);
        qvarnames.push_back(str);
        QVector<double> qdatalist;
#endif 
        std::vector<double> datalist;
        for (unsigned int i = 1; i < nread; i++) {
			entry = const_cast<char*>(slist[i].c_str());
            if (strchr(entry,'?') == nullptr) {
                x = atof(entry);
            } else {
                x = 0.0;  //NaN;
            }
			datalist.push_back(x);
#ifdef USING_QT
            qdatalist.push_back(x);
#endif
		}
        //int dblen = datalist.size();
        //int dbqlen = qdatalist.size();
        //if (dblen < 1)
        //    datalist.push_back(1.0);
        //qDebug() << " about to pushback, len = " << dblen << "qlen = " << dbqlen << " nvars = " << nvars << endl;
        data.push_back(datalist);
#ifdef USING_QT
        qdata.push_back(qdatalist);
#endif
        //nvars++;
	}
	//
    numVars = data.size();
    if ((numGroups > 0) || (numVars > 0)) {
        status = 1;
    }
	return status;
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Surface data is stored in the same way as Datagroupseries.
// A surface is a series of X-Y plots along a Z axis. We categorize surfaces as
// value surfaces (all value axes) or category surfaces Z and maybe X is a category axis
// surface format data is stored as columns, Z, X, Y.  The Z value is reapeted for N XY pairs
int DataManager::ReadSurface(std::ifstream &infile)
{
    double eps = 1.e-9;

    int status = 0;
    int nread;
    std::string line;
    char* str;
    const char *entry;
    std::vector<std::string> slist;
    slist.reserve(10);
    double x, y, z, zlast = 0.;
    // numGroups will be the number of enties along the Z axis
    // numVars will be 2; np is number of points for each Z entry; should be the same
    int np = 0;
    int nplast = -1;
    numPoints = 0;
    numGroups = 0;

    QVector<double> qdatalisty;  // yvals
    QVector<double> qdatalistx;  // x at each y
    std::vector<double> datalistx;
    std::vector<double> datalisty;

    while (std::getline(infile, line)) {

        str = const_cast<char*>(line.c_str());
        nread = Split(slist, str);
        entry = const_cast<char*>(slist[0].c_str());
        z = atof(entry);
        if (numGroups == 0) {   // first entry
            zlast = z;
            surfaceZVals.push_back(z); //qsurfaceZVals
#ifdef USING_QT
            qsurfaceZVals.push_back(z);
#endif
            numGroups = 1;
        } else if (fabs(z-zlast) > eps) {     //(z != zlast) {
            if (nplast == -1) {
                nplast = np;
            }
            zlast = z;
            numVars = 2;
            surfaceZVals.push_back(z);
#ifdef USING_QT
            qsurfaceZVals.push_back(z);
#endif
            if (np != nplast)
                qDebug() << " ERROR NP = " << np << " nplast = " << nplast << endl;
            else {
                nplast = np;
            }
            surfacexdata.push_back(datalistx);
            surfaceydata.push_back(datalisty);
            datalistx.clear();
            datalisty.clear();
#ifdef USING_QT
            qsurfacexdata.push_back(qdatalistx);
            qsurfaceydata.push_back(qdatalisty);
            qdatalistx.clear();
            qdatalisty.clear();
#endif
            numPoints = np;
            np = 0;
            numGroups++;
        }

        //for (unsigned int icol = 0; icol < nread-1; icol++) {
        entry = const_cast<char*>(slist[1].c_str());
        x = atof(entry);
        entry = const_cast<char*>(slist[2].c_str());
        y = atof(entry);
        datalistx.push_back(x);
        datalisty.push_back(y);
#ifdef USING_QT
        qdatalistx.push_back(x);
        qdatalisty.push_back(y);
#endif
        np++;
    }
    // may need one more
    //qDebug() << " end of file np = " << np << "\n";
    if (np > 0) {
#ifdef USING_QT
        qsurfacexdata.push_back(qdatalistx);
        qsurfaceydata.push_back(qdatalisty);
#endif
    }


    if (numVars > 0) {
        status = 1;
    }
    return status;
}
//-----------------------------------------------------------------------------
int DataManager::ReadMultiSeries(std::ifstream &infile)
{
	int success = 0;
	std::string line;
	char* str;
	const char *entry;
	// read variables
	std::vector<std::string> slist;
    unsigned int nread;
	// read header line -- this contains the series labels
	std::getline(infile, line);
	str = const_cast<char*>(line.c_str());
    nread = (unsigned)Split(slist, str);
	int count = 0;
    for (unsigned int i = 0; i < nread; i++) {
		if (slist[i].length() > 0) {
			varnames[count] = slist[i];
			count = count + 1;
            QString str = QString::fromStdString(slist[i]);
            qvarnames.push_back(str);
		}
	}
	numVars = count;
    // so we need numVars vectors to hold the columns
	// create a vector to hold data for each column
	for (int icol = 0; icol < numVars; icol++) {
		std::vector<double> datalist;
		data.push_back(datalist);
#ifdef USING_QT
        QVector<double> qdatalist;
        qdata.push_back(qdatalist);
#endif
	}

    // set up holders for metadata if needed
    int ncols = numVars + numMetaCollumns;
    if (numMetaCollumns > 0) {
        for (int m = 0;m<numMetaCollumns;m++){
            QVector<QString> metalist;
            metadata.push_back(metalist);
        }
    }
    // now read data
    double x;
    int id;

	while (std::getline(infile, line)) {
        //std::vector<double> datalist;
        str = const_cast<char*>(line.c_str());
        nread = Split(slist, str);

        int ncols = numVars;

        for (unsigned int icol = 0; icol < ncols; icol++) {
			entry = const_cast<char*>(slist[icol].c_str());
            if (strchr(entry,'?') == nullptr) {
                x = atof(entry);
            } else {
                x = NaN;
            }
            data[icol].push_back(x);
#ifdef USING_QT
            qdata[icol].push_back(x);
#endif
        } // end of loop on data
        for (int m = 0;m<numMetaCollumns;m++){
            entry = const_cast<char*>(slist[numVars+m].c_str());
            QString ddd = QString("XXX");
            metadata[m].push_back(ddd);
        }
	}
    numPoints = data[0].size();
    if ((numGroups > 0) || (numVars > 0)) {
        success = 1;
    }
	return success;

}
//-----------------------------------------------------------------------------
// if standard pairs we may have metadata, metadata is a string per pair of variables per line
int DataManager::ReadPairsSeries(std::ifstream &infile)
{
    int success = 0;
    std::string line;
    char* str;
    const char *entry;
    // read variables
    std::vector<std::string> slist;
    unsigned int nread;

    std::getline(infile, line);
    str = const_cast<char*>(line.c_str());
    nread = (unsigned)Split(slist, str);
    int count = 0;
    for (unsigned int i = 0; i < nread; i+=2) {
        if (slist[i].length() > 0) {
            varnames[count] = slist[i+1];
            count = count + 1;
            QString str = QString::fromStdString(slist[i+1]);
            qvarnames.push_back(str);
        }
    }
    numVars =  nread;
    numPairs = count;
    // **** THIS IS TEMPORARY  ****
        // so we need numVars vectors to hold the columns
        // create a vector to hold data for each column
        for (int icol = 0; icol < numVars; icol++) {
            std::vector<double> datalist;
            data.push_back(datalist);
        }
    for (int icol = 0;icol <numPairs;icol++) {
        QVector<pairdata> pairlist;
        pdata.push_back(pairlist);
    }
    // If we are reading standard pairs there should be an even number of column headers,
    // If augmented pairs the number should be divisible by three
    //if (currentType == T_StandardPairs){
    //    if (nread % 2 != 0)
    //        qDebug() << "ERROR INPUT NOT VALID FOR PAIRS" << endl;
    //    numVars = nread / 2;
    //} else {            // augmented pairs
    //    numVars = nread / 3;
    //}
    // set up holders for metadata if needed
    int ncols = numVars + numMetaCollumns;
    if (numMetaCollumns > 0) {
        for (int m = 0;m<numMetaCollumns;m++){
            QVector<QString> metalist;
            metadata.push_back(metalist);
        }
    }
    // now read data
    double x, y;
    int id;
    numPoints=0;
    bool missingAbcissa = false;
    while (std::getline(infile, line)) {
        //std::vector<double> datalist;
        str = const_cast<char*>(line.c_str());
        nread = Split(slist, str);

        if ( ((nread % 3) != 0) && (numMetaCollumns > 0) ) {
            qDebug() << "ERROR\n";
        }
        int index = 0;
        int npair = 0;
        unsigned int endcol = nread - numMetaCollumns;
        unsigned int increment = 2;
        if (currentType == T_AugmentedPairs)
            increment = 3;
        for (unsigned int icol = 0; icol < endcol; icol+=increment) {
            entry = const_cast<char*>(slist[icol].c_str());
            if (strchr(entry,'?') == nullptr) {
                x = atof(entry);
            } else {
                x = NaN;
                missingAbcissa = true;
            }
            entry = const_cast<char*>(slist[icol+1].c_str());
            if (strchr(entry,'?') == nullptr) {
                y = atof(entry);
            } else {
                y = NaN;
            }
            if (currentType == T_AugmentedPairs) {
                if (nread >=3) {
                    entry = const_cast<char*>(slist[icol+2].c_str());
                    id = atoi(entry);
                } else {
                    id = 0;
                }

            } else {  // maybe pairs with metadata
                if (numMetaCollumns > 0) {
                    id = numPoints;
                } else {
                    id = 0;
                }
            }

            pairdata pd(x,y,id);
            pdata[npair].push_back(pd);
            //data[icol].push_back(x);

            data[index].push_back(x);
            data[index+1].push_back(y);

            index = index + 2;
            npair++;

        } // end of loop on data columns
        for (int m = 0;m<numMetaCollumns;m++){
            entry = const_cast<char*>(slist[numVars+m].c_str());
            QString ddd = QString::number(numPoints);
            //QString ddd = QString("XXX");
            metadata[m].push_back(ddd);
        }
        numPoints++;
    }

    if (missingAbcissa)
        numMetaCollumns = 0;  // kill metadata
    return success;
}
//-----------------------------------------------------------------------------
int DataManager::ReadMarksData(std::string Mfilename)
{
    int status = 0;  // return 1 for success
    char* str;
    std::string line;
    std::vector<std::string> slist;
    double x;
    int nread;

    std::ifstream infile(Mfilename);

    if (!infile.is_open()) {
        status = -1;
        return status;
    }

    //char * str;
    const char *entry;

    while (std::getline(infile, line)) {
        QVector<double> qdatalist;
        str = const_cast<char*>(line.c_str());
        nread = Split(slist, str);
        for (unsigned int icol = 0; icol < nread; icol++) {
            entry = const_cast<char*>(slist[icol].c_str());
            if (strchr(entry,'?') == nullptr) {
                x = atof(entry);
            } else {
                x = NaN;
            }
            qdatalist.push_back(x);
        }
        //if (qdatalist.length() > 1) {
            numMarks++;
            marksdata.push_back(qdatalist);
       // }

    }
    return status;

}
QString DataManager::getMetadata(QString graphName, int pointid)
{
    QString datastring = "";
    if (numMetaCollumns == 0)
        return datastring;
    int index = -1;
    // get graph index -- metadata only comes with data in pairs
    for (int i = 0;i < numPairs; i++) {
        if(graphName == qvarnames[i]) {
            index = i;
            break;
        }
    }
    datastring = metadata[index][pointid];
    return datastring;
}

int DataManager::getGraphIndex(QString graphName)
{
    int index = 0;

    return index;
}
