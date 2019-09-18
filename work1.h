#ifndef WORK1_H
#define WORK1_H

#include <QString>
#include "apoint.h"

struct Work1Params{
public:
    QString inFile;
    QString outFile;
    bool isBackup;
};

class Work1
{
public:
    Work1();
    static int doWork();
    static Work1Params params;

    static int savemap(const QString& fn, const QList<APoint> &list);
    static QList<APoint> grbLoader(const QString& grbFileName);
    static QMap<int, QList<APoint>> pointListToMap(const QList<APoint> &list);
    static QMap<int, APoint> pointMapWeight(const QMap<int, QList<APoint>> &map);
    static QList<APoint> pointMapToList(const QMap<int, APoint> &map);
};

#endif // WORK1_H
