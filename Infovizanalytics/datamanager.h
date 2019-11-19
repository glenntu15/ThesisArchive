#pragma once
#define USING_QT

#include <string>
#include <vector>
#ifdef USING_QT
#include <QtCharts>
#endif

struct pairdata {
        double x;
        double y;
        int indx;
        pairdata(double xx, double yy, int i)
        {
            x = xx;
            y = yy;
            indx = i;
        }
        pairdata()
        {
            x =0.;
            y =0.;
            indx =0;
        }
};

#define MAX_VARS 20  // max number of columns
class DataManager
{
public:
	static DataManager& getInstance()
	{
		static DataManager instance;
		return instance;
	}

	~DataManager();

    enum dstypes { T_standard, T_StandardPairs, T_AugmentedPairs, T_surface, T_datagroup_series, T_datagroup_parallel, T_dataclass_parallel, T_unknown };
	
    double NaN;

	int ReadFile(std::string filename);

	std::string getDSN() { return dsname; }
	dstypes getDST() {
		return currentType;
	}

    std::string getSeriesLabel(int i) { return serieslables[i]; }
    std::string getXName() { return varnames[0]; }
    std::string getYName() { return varnames[1]; }
    std::string getVarName(int i) { return varnames[i]; }
    QString getQVarName(int i) { return qvarnames[i]; }
    double getVarMax(int i) {return varMax[i];}
    double getVarMin(int i) {return varMin[i];}
    int getNumVars() { return numVars; }
    int getNumGroups() { return numGroups; }
    int getNumPoints() { return numPoints; }
    int getNUmMarks() {return numMarks;}
    int getNumPairs() {return numPairs;}

    bool getOmitVar(int i) { if (omitvar != nullptr) return omitvar[i];else return false;}
    void setOmitVar(int i) { if (omitvar != nullptr) omitvar[i] = true;}
    void clearOmitVar(int i) { if (omitvar != nullptr) omitvar[i] = false;}


    double getAllMax() {return allMax; }
    double getAllMin() {return allMin; }
    double getAllsurfXMax() { return allsurfXMax;}
    double getAllsurfXMin() { return allsurfXMin;}

    //used for 3D
    float getMaxZ() {return static_cast<float>(surfZMax); }
    float getMinZ() {return surfZMin; }
#ifdef USING_QT
    QVector<QString> categoryLabels;
    QString getCatagoryLabel(int i) { return categoryLabels[i];}
    QString getCategoryMaxLabel(int i) {return qcategoryMaxLabels[i];}

    QString getMetadata(QString graphName, int pointid);
    int getGraphIndex(QString graphName);

    QVector<QVector<double>>& getQdata() {return qdata;}
    QVector<QVector<pairdata>> &getPdata() {return pdata;}
    QVector<QVector<double>>& getMarksdata() {return marksdata;}
    QVector<QVector<double>>& getQCategorydata() {return qcategorydata;}
    QVector<QVector<double>>& getQNormaldata() {return qnormaldata;}
    QVector<QVector<pairdata>> pdata;
    QString getDsname() { return qdsname;}
    double getqSurfaceZVal(int i) { return qsurfaceZVals[i];}
    double getQSurfaceXval(int n, int i) { return qsurfacexdata[n][i];}
    double getQSurfaceYval(int n, int i) { return qsurfaceydata[n][i];}
#else 
	std::vector<std::string> categoryLabels;
	std::string getCatagoryLabel(int i) { return categoryLabels[i]; }

#endif
	
	std::vector<std::vector<double>>& getData() { return data; }

    void ClearOmited();

private:
    //enum dstypes { T_standard, T_surface, T_datagroup_series };

	DataManager();

	int ReadMultiSeries(std::ifstream &infile);
    int ReadPairsSeries(std::ifstream &infile);
	int ReadDataGroupSeries(std::ifstream &infile);
    int ReadSurface(std::ifstream &infile);
    int ReadMarksData(std::string Mfilename);
	int Split(std::vector<std::string> &slist, char * line);
    void Reset();

    void ComputeNormalData();

    int numVars;
    int numPairs;
    int numMetaCollumns;
    /// numGroups is also numcategories
	int numGroups;
    int numPoints;
    int numClasses;
    int numMarks;
	dstypes currentType;
    double allMax;
    double allMin;
    double allsurfXMax;
    double allsurfXMin;
    double surfZMax;
    double surfZMin;

#ifdef USING_QT
    QVector<double> varMax;
    QVector<double> varMin;
    QVector<double> surfXMax;       // for surface plot we have an x array for each y variable
    QVector<double> surfXMin;

    QVector<double> categoryMax;
    QVector<double> categoryMin;
    QString qdsname;
#else
    std::vector<double> varMax;
    std::vector<double> varMin;
    std::vector<double> surfXMax;       // for surface plot we have an x array for each y variable
    std::vector<double> surfXMin;
    double  categoryMax[]
#endif
    bool *omitvar;

	std::string dsname;

	std::string varnames[MAX_VARS];
	std::string serieslables[MAX_VARS];
    std::vector<std::vector<double>> data;

    std::vector<double> surfaceZVals;
    std::vector<std::vector<double>> surfacexdata;
    std::vector<std::vector<double>> surfaceydata;
#ifdef USING_QT
    QVector<QVector<double>> qdata;
    QVector<QString> qvarnames;

    QVector<QVector<double>> marksdata;

    // metadata
    QVector<QVector<QString>> metadata;

    /// This is scaled data for parallel coordinate plots
    QVector<QVector<double>> qcategorydata;
    QVector<QVector<double>> qnormaldata;
    QVector<QString> qcategoryMaxLabels;

    QVector<double> qsurfaceZVals;
    QVector<QVector<double>> qsurfacexdata;
    QVector<QVector<double>> qsurfaceydata;
    //float minz, maxz;
#endif
};



