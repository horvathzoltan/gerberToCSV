#include "work1.h"
#include "common/logger/log.h"
#include "apoint.h"
#include "common/helper/string/stringhelper.h"
#include "common/helper/textfilehelper/textfilehelper.h"
#include <QFileInfo>
#include <QRegularExpression>
#include <QtMath>

Work1Params Work1::params;

Work1::Work1()= default;

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

      //gerberből poligonok
      auto pmap = pointListToMap(list);
      //poligon súlypontok
      auto wmap = pointMapWeight(pmap);
      //poligon rádiuszok hozzáadása az eredeti listához
      pointMapRadius(pmap, &wmap);
      // százalékban megadni
      auto pwmap = pointMapPercent(wmap);
      auto spwmap = pointMapShift(pwmap, -50, -50);


      auto list2 = pointMapToList(spwmap);

      savemap(params.outFile, list);
      savemap("3"+params.outFile, list2);

      QString txt = drawPointList(list2, 100, 100, 50,50);

      auto spwmapb = pointMapMirror(spwmap);
      auto list2b = pointMapToList(spwmapb);
      savemap("3b"+params.outFile, list2b);
      savemapjb("3jb"+params.outFile, list2, list2b);
      QString txtb = drawPointList(list2b, 100, 100, 50,50);

      if(!txt.isEmpty()){
          auto isok = com::helper::TextFileHelper::save(txt, "4"+params.outFile);
          auto isok2 = com::helper::TextFileHelper::save(txtb, "4b"+params.outFile);
          if(isok && isok2){
              zInfo(QStringLiteral("map saved: %1").arg("4"+params.outFile));
          }
          else
          {
              zInfo(QStringLiteral("map not saved"));
          }
      }else{
          zInfo(QStringLiteral("map is empty"));
      }


    zInfo(QStringLiteral("Work1 done"));
    return 0;
}

QMap<int, APoint> Work1::pointMapMirror(const QMap<int, APoint> &map)
{
    QMap<int, APoint> e;

    zforeach(p, map){
        e.insert(p->id, APoint{-p->X, p->Y, p->id, p->grbl, p->R});
    }
    return e;
}

QMap<int, APoint> Work1::pointMapShift(const QMap<int, APoint>& map, double x, double y){
    QMap<int, APoint> e;
    zforeach(p, map){
        e.insert(p->id, APoint{p->X+x, p->Y+y, p->id, p->grbl, p->R});
    };
    return e;
}

QMap<int, APoint> Work1::pointMapPercent(const QMap<int, APoint>& map){
    double maxx=std::numeric_limits<double>::min(),maxy=maxx,minx=std::numeric_limits<double>::max(),miny=minx;
    zforeach(p, map){
        if(p->X>maxx) maxx=p->X;
        if(p->Y>maxy) maxy=p->Y;
        if(p->X<minx) minx=p->X;
        if(p->Y<miny) miny=p->Y;
    }

    double dx=maxx-minx;
    double dy=maxy-miny;

    double mx=10, my=10;
    double px=(dx+2*mx)/100; //0-99 a tartományunk, 100 szám
    double py=(dy+2*my)/100; //0-99 a tartomány, ez 100 szám

    QMap<int, APoint> e;
    zforeach(p, map){
        double x= px?((p->X-minx)+mx)/px:0;
        double y= py?((p->Y-miny)+my)/py:0;
        double r = (px!=-py)?(2*(p->R))/(px+py):0;

        e.insert(p->id, APoint{x, y, p->id, p->grbl, r});
    }
    return e;
}

void Work1::pointMapRadius(const QMap<int, QList<APoint>> &map,QMap<int, APoint> *wmap){
    zforeach(wp, *wmap){
        int wpk = wp.key();
        auto l = map[wpk];
        auto D=0;
        double n=0;
        int ix=1;
        for(auto dp = l.begin(); dp != l.end()-ix; ++dp){
        //zforeach(dp, l){
            D+= std::hypot(dp->X-wp->X, dp->Y-wp->Y);
            n++;
        }
        wp->R=D/n;
        //zInfo(QStringLiteral("R: %1 %2").arg(p->R).arg((*wmap)[pk].R));
    }
}

double Work1::round(double val)
{
    if( val < 0 ) return ceil(val - 0.5);
    return floor(val + 0.5);
}

// nem hexadecimális, hanem kulcs-érték alpú konverziót valósít meg
char Work1::chToChar(int i){
    static QMap<int,char> m{
        {0,'a'},{1,'b'},{2,'c'},{3,'d'},{4,'f'},{5,'g'},
        {6,'h'},{7,'j'},{8,'k'},{9,'m'},{10,'p'},{11,'r'},
        {12,'s'},{13,'t'},{14,'w'},{15,'z'},{16,'x'},

        {17,'A'},{18,'B'},{19,'C'},{20,'D'},{21,'F'},{22,'G'},
        {23,'H'},{24,'J'},{25,'K'},{26,'M'},{27,'N'},{28,'P'},
        {29,'S'},{30,'T'},{31,'U'},{32,'W'},{33,'X'}
    };
    if(i>=0&&i<=15 && m.contains(i)) return m[i];
    return'*';
}


QString Work1::drawPointList(const QList<APoint>& list, double VX, double VY, double dx, double dy){
    QString e;
    const int MAX_X = 20;
    const int MAX_Y = 20;

    //const double VX = 300;
    //const double VY = 300;

    zInfo(QStringLiteral("drawing map:%1").arg("4"+params.outFile));

    zInfo(QStringLiteral("map init"));
    char m[MAX_X][MAX_Y];
    for(int x=0; x<MAX_X;x++){
        for(int y=0; y<MAX_Y;y++){
            m[x][y] = ' ';
        }
    }
    m[0][0] = '+';
    m[MAX_X-1][0] = 'x';
    m[0][MAX_Y-1] = 'y';

    double rx =VX/(MAX_X-1);//0-19
    double ry =VY/(MAX_Y-1);//0-19

    zInfo(QStringLiteral("map draw"));
    zforeach(p, list){
        int x = (int)round((p->X+dx)/rx);
        int y = (int)round((p->Y+dy)/ry);
        if(x>=MAX_X || y>=MAX_Y){
            zInfo(QStringLiteral("out of range: %1,%2").arg(x).arg(y));
            continue;
        }
        m[x][y]=chToChar(p->id);
    }

    zInfo(QStringLiteral("map textbuild"));
    for(int y=MAX_Y-1; y>=0;y--){
        for(int x=0; x<MAX_X;x++){
            e+=m[x][y];
        }
        e+=com::helper::StringHelper::NewLine;
    }
    e+=com::helper::StringHelper::NewLine;

    zInfo(QStringLiteral("map ok; isempty=%1").arg(e.isEmpty()));
    return e;
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
        int id = m.key();
        e.insert(id, APoint(x/j, y/j, id, 0, 0));
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

    //int e = 0;
    QStringList txtl;
    txtl.append(QStringLiteral("insole;channel;r;x;y;grbl"));
    //int i =0;
    zforeach(p, list)
    {
        auto ch = chToChar(p->id);
        txtl.append(QStringLiteral("42j16s;%3;%4;%1;%2;%5").arg(p->X).arg(p->Y).arg(ch).arg(p->R).arg(p->grbl));
    }

    //QFileInfo fi(sFileName);
    //QFileInfo(fi.dir(), fi.baseName()+"_x."+fi.completeSuffix()).filePath();

    //QString ns = zFileNameHelper::appendToBaseName(sFileName, QStringLiteral("x"));
    com::helper::TextFileHelper::save(txtl.join("\n"), fn);
    return 1;
}

int Work1::savemapjb(const QString& fn, const QList<APoint> &list, const QList<APoint> &listb){

    if(fn.isEmpty())
    {
        zInfo(QStringLiteral("empty source text"));
        return 0;
    }

    //int e = 0;
    QStringList txtl;
    txtl.append(QStringLiteral("insole;sign;r;x;y;grbl"));
    //int i =0;
    int s= list.count()+1;
    zforeach(p, list)
    {
        auto ch = chToChar(p->id);
        txtl.append(QStringLiteral("%7j%6s;%3;%4;%1;%2;%5").arg(p->X).arg(p->Y).arg(ch).arg(p->R).arg(p->grbl).arg(s).arg(params.insoleSize));
    }
    zforeach(pb, listb)
    {
        auto ch = chToChar(pb->id);
        txtl.append(QStringLiteral("%7b%6s;%3;%4;%1;%2;%5").arg(pb->X).arg(pb->Y).arg(ch).arg(pb->R).arg(pb->grbl).arg(s).arg(params.insoleSize));
    }
    //QFileInfo fi(sFileName);
    //QFileInfo(fi.dir(), fi.baseName()+"_x."+fi.completeSuffix()).filePath();

    //QString ns = zFileNameHelper::appendToBaseName(sFileName, QStringLiteral("x"));
    com::helper::TextFileHelper::save(txtl.join("\n"), fn);
    return 1;
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
                       APoint p(x,y, id, i, 0);
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

                       APoint c(cx, cy, cid, i, 0);
                       list<<c;

                   }
                   px = x;
                   py = y;
               }
           }
       }

       return list;
}
