#ifndef WORK1_H
#define WORK1_H

#include <QString>
#include "apoint.h"

struct Work1Params{
public:
    QString inFile;
    QString outFile;
    QString insoleSize;
    bool isBackup;
};

class Work1
{
public:
    Work1();
    static int doWork();
    static Work1Params params;

    static int savemap(const QString& fn, const QList<APoint> &list);
    static int savemapjb(const QString& fn, const QList<APoint> &list, const QList<APoint> &listb);
    static QList<APoint> grbLoader(const QString& grbFileName);
    static QMap<int, QList<APoint>> pointListToMap(const QList<APoint> &list);
    static QMap<int, APoint> pointMapWeight(const QMap<int, QList<APoint>> &map);
    static QList<APoint> pointMapToList(const QMap<int, APoint> &map);
    static QString drawPointList(const QList<APoint>& list, double VX, double VY, double dx, double dy);
    static double round(double val);
    static void pointMapRadius(const QMap<int, QList<APoint>> &map,QMap<int, APoint>* );
    static QMap<int, APoint> pointMapPercent(const QMap<int, APoint>& );
    static QMap<int, APoint> pointMapPercent_mirrored(const QMap<int, APoint>& );

    static QMap<int, APoint> pointMapShift(const QMap<int, APoint>& , double x, double y);
    static QMap<int, APoint> pointMapMirror(const QMap<int, APoint> &map);
private:
    static char chToChar(int i);
};

#endif // WORK1_H
