#ifndef APOINT_H
#define APOINT_H


class APoint
{
public:
    APoint();
    APoint(double x, double y, int id, int grbl);
    double X;
    double Y;
    int id;
    int grbl;
};

#endif // APOINT_H
