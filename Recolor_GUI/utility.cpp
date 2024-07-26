#include "utility.h"
#include <cmath>
#include <QDebug>

myfloat3::myfloat3(float _x, float _y, float _z) : x(_x), y(_y), z(_z)
{
}

myfloat3::myfloat3() : x(0.f), y(0.f), z(0.f)
{
}

const float myfloat3::norm() const
{
    float ans = x * x + y * y + z * z;
    if(ans < 0) ans = 0;
    return sqrtf(ans);
}

float myfloat3::dot(myfloat3 another)
{
    return x * another.x + y * another.y + z * another.z;
}

myfloat3 myfloat3::cross(myfloat3 another)
{
    const float &x_ = another.x;
    const float &y_ = another.y;
    const float &z_ = another.z;
    return myfloat3(y * z_ - z * y_, z * x_ - x * z_, x * y_ - y * x_);
}

const myfloat3 myfloat3::operator +(const myfloat3 another) const
{
    const float &x_ = another.x;
    const float &y_ = another.y;
    const float &z_ = another.z;

    return myfloat3(x + x_, y + y_, z + z_);
}

const myfloat3 myfloat3::operator -(const myfloat3 another) const
{
    const float &x_ = another.x;
    const float &y_ = another.y;
    const float &z_ = another.z;

    return myfloat3(x - x_, y - y_, z - z_);
}

const myfloat3 myfloat3::operator *(float k) const
{
    return myfloat3(x * k, y * k, z * k);
}

const myfloat3 myfloat3::operator /(float k) const
{
    k = 1.f / k;
    return myfloat3(x * k, y * k, z * k);
}

float myfloat3::getX() const
{
    return x;
}

float myfloat3::getY() const
{
    return y;
}

float myfloat3::getZ() const
{
    return z;
}

float dot(myfloat3 x, myfloat3 y)
{
    return x.dot(y);
}

myfloat3 cross(myfloat3 x, myfloat3 y)
{
    return x.cross(y);
}

QDebug operator<<(QDebug debug, const myfloat3 &c)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << '(' << c.getX() << ", " << c.getY() << ", " << c.getZ() << ')';

    return debug;
}

float myasin(float x)
{
    if(x > 1) x = 1;
    if(x < -1) x = -1;
    return asin(x);
}

float myacos(float x)
{
    if(x > 1) x = 1;
    if(x < -1) x = -1;
    return acos(x);
}

float mysqrt(float x)
{
    if(x < 0) x = 0;
    return sqrt(x);
}