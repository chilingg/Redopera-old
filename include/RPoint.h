#ifndef RPOINT_H
#define RPOINT_H

#include <functional>

namespace Redopera {

class RPoint2
{
    friend std::hash<RPoint2>;

public:
    constexpr static int INVALID_POINT_VALUE = ~0u >> 1;

    explicit RPoint2(int x, int y = 0): x_(x), y_(y) {}
    RPoint2(): x_(INVALID_POINT_VALUE), y_(INVALID_POINT_VALUE) {}
    bool operator==(const RPoint2 &pos) const { return x_ == pos.x_ && y_ == pos.y_; }

    int x() const { return x_; }
    int y() const { return y_; }
    int& rx() { return x_; }
    int& ry() { return y_; }
    bool isValid() const { return x_ != INVALID_POINT_VALUE && y_ != INVALID_POINT_VALUE; }

    void setPoint(int x, int y) { x_ = x; y_ = y; }
    void setX(int x) { x_ = x; }
    void setY(int y) { y_ = y; }

    RPoint2 MirrorH(int axis = 0) { return RPoint2((axis-x_)*2 + x_, y_); }
    RPoint2 MirrorV(int axis = 0) { return RPoint2(x_, (axis-y_)*2 + y_); }

protected:
    int x_;
    int y_;
};

class RPoint3 : public RPoint2
{
    friend std::hash<RPoint3>;

public:
    explicit RPoint3(int x, int y = 0, int z = 0):
        RPoint2(x, y), z_(z) {}

    RPoint3():
        RPoint2(), z_(INVALID_POINT_VALUE) {}

    bool operator==(const RPoint3 &pos) const { return x_ == pos.x_ && y_ == pos.y_ && z_ == pos.z_; }

    int z() const { return z_; }
    int& rz() { return z_; }

    bool isValid() const { return RPoint2::isValid() && z_ != INVALID_POINT_VALUE; }

    void setPoint(int x, int y, int z) { x_ = x; y_ = y; z_ = z; }
    void setZ(int z) { z_ = z; }

protected:
    int z_;
};

} // Redopera
#endif // RPOINT_H
