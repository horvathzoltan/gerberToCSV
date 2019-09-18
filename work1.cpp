#include "work1.h"
#include "common/logger/log.h"
#include "apoint.h"
#include"common/helper/textfilehelper/textfilehelper.h"
#include <QFileInfo>
#include <QRegularExpression>
#include <QtMath>

Work1Params Work1::params;

Work1::Work1()
{

}

int Work1::doWork()
{    
    zInfo(QStringLiteral("params: %1, %2, %3").arg(params.inFile).arg(params.outFile).arg(params.isBackup));
    //calculate(params.inFile, params.outFile);

    if(params.inFile.isEmpty())
        {
            zInfo(QStringLiteral("no input  filename"));
            return 1;
        }

     if(params.outFile.isEmpty())
        {
            zInfo(QStringLiteral("no source filename"));
            return 1;
        }

        //QString lFileName = getMessageFileName(lFilePath, sFileName);//Messages_USERSERVICE_hu-HU.csv

      auto list = grbLoader(params.inFile);
        if(list.isEmpty())
        {
            zInfo(QStringLiteral("list is empty"));
            return 1;
        }

      auto pmap = pointListToMap(list);

      auto wmap = pointMapWeight(pmap);

      auto list2 = pointMapToList(wmap);

      savemap(params.outFile, list);

      savemap("2"+params.outFile, list2);



    zInfo(QStringLiteral("Work1 done"));
    return 0;
}

QList<APoint> Work1::pointMapToList(const QMap<int, APoint> &map)
{
    QList<APoint> e;

    zforeach(m, map){
        auto p = *m;
        p.id = m.key();
        e.append(p);
    }

    return e;
}

// TODO radiuszt ki kell számolni
QMap<int, APoint> Work1::pointMapWeight(const QMap<int, QList<APoint>> &map){
    QMap<int, APoint> e;

    zforeach(m, map)
    {
        double x=0,y=0,j=0;
        auto v = m.value();
        int ix = 1;
        for(auto a = v.begin(); a != v.end()-ix; ++a){
            j++;
            x+=a->X;
            y+=a->Y;
        }
        e.insert(m.key(), APoint(x/j, y/j, 0, 0));
    }

    return e;
}

// by id
QMap<int, QList<APoint>> Work1::pointListToMap(const QList<APoint> &list){
    QMap<int, QList<APoint>> e;

    zforeach(p, list){
        if(e.contains(p->id)){
            e[p->id].append(*p);
        }
        else{
            auto l = QList<APoint>();
            l.append(*p);
            e.insert(p->id, l);
        }
    }

    return e;
}

int Work1::savemap(const QString& fn, const QList<APoint> &list){

    if(fn.isEmpty())
    {
        zInfo(QStringLiteral("empty source text"));
        return 0;
    }

    int e = 0;
    QStringList txtl;
    txtl.append(QStringLiteral("insole;channel;r;x;y"));
    int i =0;
    zforeach(p, list)
    {
        txtl.append(QStringLiteral("42j16s;%3;%4;%1;%2;%5").arg(p->X).arg(p->Y).arg(p->id).arg(5).arg(p->grbl));
    }

    //QFileInfo fi(sFileName);
    //QFileInfo(fi.dir(), fi.baseName()+"_x."+fi.completeSuffix()).filePath();

    //QString ns = zFileNameHelper::appendToBaseName(sFileName, QStringLiteral("x"));
    com::helper::TextFileHelper::save(txtl.join("\n"), fn);
}

QList<APoint> Work1::grbLoader(const QString& grbFileName){
    auto fi = QFileInfo(grbFileName);
    auto ap_grbFileName = fi.absoluteFilePath();

    auto grblines = com::helper::TextFileHelper::loadLines(ap_grbFileName);

       QList<APoint> list;
       QList<APoint> clist;
       if(grblines.isEmpty())
       {
           zInfo(QStringLiteral("no loaded messages"));
       }

       //else zInfo(QStringLiteral("loaded %1 lines").arg(grblines.count()));

       int xif;
       int xdf;
       int yif;
       int ydf;

       auto seed1 = QStringLiteral(R"(\%FS(?:LA)?X(\d)(\d)Y(\d)(\d)\*\%)");
       zforeach(l, grblines)
       {
           if(!l->isEmpty())
           {
               QRegularExpression r1(seed1);
               auto m1 = r1.match(*l);
               if(m1.hasMatch())
               {
                   xif = m1.captured(1).toInt();
                   xdf = m1.captured(2).toInt();
                   yif = m1.captured(3).toInt();
                   ydf = m1.captured(4).toInt();
                   break;
               }
           }
       }


       int id = 0;
       double x,y,px = 0,py = 0;
       auto seed2 = QStringLiteral(R"((?:X(\d*))?(?:Y(\d*))?D01\*)");
       QRegularExpression r2(seed2);
       int i = 0;
       zforeach(l2, grblines){
           i++;
           if(!l2->isEmpty()){
               if(l2->contains("G36*")){
                   id++;
                   continue;
               }
               auto m2 = r2.match(*l2);
               auto matched = m2.hasMatch();
               if(matched){
                   //auto lngth = m2.captured(1);
                   auto lngth1 = m2.capturedLength(1);
                   auto lngth2 = m2.capturedLength(2);
                   x = m2.captured(1).left(lngth1-xdf).toDouble() + m2.captured(1).right(xdf).toDouble()/qPow(10, xdf);
                   y = m2.captured(2).left(lngth2-xdf).toDouble() + m2.captured(2).right(xdf).toDouble()/qPow(10, xdf);

                   double dist = sqrt(pow(x-px , 2) + pow(y-py, 2));
                   if(dist < 2.0){
                       APoint p(x,y, id, i);
                       clist<<p;
                   }
                   else{
                       int cid;
                       double cx, cy;
                       if(!clist.isEmpty()){
                           double sumx = 0, sumy = 0;
                           zforeach(cp, clist){
                               //zInfo(QStringLiteral("x:%1;y:%2").arg(cp->X).arg(cp->Y));
                               sumx += cp->X;
                               sumy += cp->Y;
                           }
                           cx = sumx/clist.count();
                           cy = sumy/clist.count();
                           cid = clist[0].id;
                           clist.clear();
                       }
                       else{
                           cx = x;
                           cy = y;
                           cid = id;
                       }

                       APoint c(cx, cy, cid, i);
                       list<<c;

                   }
                   px = x;
                   py = y;
               }
           }
       }

       return list;
}
