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
    static QList<APoint> Work1::grbLoader(const QString& grbFileName);
};

#endif // WORK1_H
