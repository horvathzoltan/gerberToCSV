#include "apoint.h"

APoint::APoint()
{

}

APoint::APoint(double x, double y, int id, int grbl, double r){
    this->X = x;
    this->Y = y;
    this->id = id;
    this->grbl = grbl;
    this->R = r;
}
