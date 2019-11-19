#ifndef COLORTABLE_H
#define COLORTABLE_H
#include <string>
//class QPen;
#include <qpen.h>

class ColorTable
{
public:
    ColorTable();

    struct comp {
        int r;
        int g;
        int b;
        int a;

        comp(int ir, int ig, int ib) {r = ir; g = ig; b = ib; a = 255;}
    };
    struct namecomp {
        int r;
        int g;
        int b;
        int a;

        namecomp(std::string name, int ir, int ig, int ib) {r = ir; g = ig; b = ib; a = 255;}
    };

    int tableindex;
    bool UseAlternate;
    void SetUseAlternate(bool val) {UseAlternate = val;}

//    enum colors {
//         _red,
//        _green,
//        _blue,
//        _fuschia,
//        _grape,
//        _orange,
//        _purple,
//        _babyblue,
//        _mustard,
//        _brown

//    };
//    enum altcolors {
//        _a_fuschia,
//         _a_ElectricUltramarine,
//        _a_orange,
//        _a_red,
//        _a_green,
//        _a_violet,
//        _a_purple,
//        _a_babyblue,
//        _a_mustard,
//        _a_brown
//  };
    int ncolors = 10;

    namecomp values[10] = { {"red",255, 0,0},      //r
                        {"green",51,255,51},     //g
                        {"fuschia",255,0,255},     //f
                        {"blue",51, 153,251},   //b
                        {"grape",116,52,199},    //g
                        {"orange",255,178,102},   //o
                        {"purple",178,102,255},   //p
                        {"babyblue",0,255,255},     //bb
                        {"mustard",204,204,0},     //m
                        {"brown",153,76,0}       // brown
                      };
    namecomp altvalues[10] = {
                        {"fuschia",255,0,255},
                        {"ElectricUltramarine",43, 0,255},
                        {"orange",255,178,102},
                        {"red",255, 0,0},
                        {"green",51,255,51},
                        {"violet",128,0,255},
                        {"purple",178,102,255},
                        {"babyblue",0,255,255},
                        {"mustard",204,204,0},
                        {"brown",153,76,0}
                      };
public:
    QPen GetPen(int index);
    QPen GetPen(int index, int groupsize);
    QPen GetPen(int index, int groupsize, qreal scale);
};

#endif // COLORTABLE_H
