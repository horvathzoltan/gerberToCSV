#include "work1.h"
#include "common/logger/log.h"

Work1Params Work1::params;

Work1::Work1()
{

}

int Work1::doWork()
{
    //auto a1 = params.inFile;
    auto m = new char[40];
    m=new char[2];
    zInfo(QStringLiteral("params: %1, %2, %3").arg(params.inFile).arg(params.outFile).arg(params.isBackup));
    zInfo(QStringLiteral("Work1 done"));
    return 1;
}


