#include "RArea.h"

using namespace Redopera;

RArea::Format RArea::areaFormat;

void RArea::setDefaultFormat(RArea::Format format)
{
    areaFormat = format;
}

RArea::Format RArea::getDefaultFormat()
{
    return areaFormat;
}

RArea::RArea():
    format_(areaFormat)
{

}

RArea::RArea(int width, int height, int x, int y, int z):
    format_{ RSize(width, height), RPoint(x, y, z) }
{

}

RArea::RArea(int width, int height, const RPoint &pos):
    format_{ RSize(width, height), pos }
{

}

RArea::RArea(const RSize &size, const RPoint &pos):
    format_{ size, pos }
{

}

RArea::RArea(const RRect &rect, int z):
    format_{ rect.size(), RPoint(rect.bottomLeft(), z) }
{

}

RArea::RArea(const RArea::Format &format):
    format_(format)
{

}

RArea::RArea(RArea &area):
    format_(area.format_)
{

}

int clamp(int v, int min, int max)
{
    if(v < min)
        return min;
    else if(v > max)
        return max;
    else
        return v;
}

void RArea::setSize(int width, int height)
{
    width = clamp(width, format_.minW, format_.maxW);
    height = clamp(height, format_.minH, format_.maxH);
    format_.size.set(width, height);
}

void RArea::setSize(const RSize &size)
{
    int width = clamp(size.width(), format_.minW, format_.maxW);
    int height = clamp(size.height(), format_.minH, format_.maxH);
    format_.size.set(width, height);
}

void RArea::setWidth(int width)
{
    width = clamp(width, format_.minW, format_.maxW);
    format_.size.setWidth(width);
}

void RArea::setHeight(int height)
{
    height = clamp(height, format_.minH, format_.maxH);
    format_.size.setHeight(height);
}

void RArea::setMinSize(int minw, int minh)
{
    minw = minw < 0 ? 0 : minw;
    minh = minh < 0 ? 0 : minh;
    format_.minW = minw;
    format_.minH = minh;
}

void RArea::setMinSize(const RSize &size)
{
    int minw = size.width() < 0 ? 0 : size.width();
    int minh = size.height() < 0 ? 0 : size.height();
    format_.minW = minw;
    format_.minH = minh;
}

void RArea::setMaxSize(int maxw, int maxh)
{
    if(maxw < 1 || maxh < 1)
        return;
    format_.maxW = maxw;
    format_.maxH = maxh;
}

void RArea::setMaxSize(const RSize &size)
{
    if(size.invalid())
        return;
    format_.maxW = size.width();
    format_.maxH = size.height();
}

void RArea::setPos(int x, int y, int z)
{
    format_.pos.set(x, y, z);
}

void RArea::setPos(const RPoint &pos)
{
    format_.pos = pos;
}

void RArea::setPosX(int x)
{
    format_.pos.setX(x);
}

void RArea::setPosY(int y)
{
    format_.pos.setY(y);
}

void RArea::setPosZ(int z)
{
    format_.pos.setZ(z);
}

void RArea::setOuterPos(const RPoint &pos)
{
    format_.pos.set(pos.x()+format_.margin.l, pos.y()+format_.margin.b, format_.pos.z());
}

void RArea::setCenterPos(const RPoint &pos)
{
    format_.pos.set(pos.x()-format_.size.width()/2, pos.y()-format_.size.height()/2, format_.pos.z());
}

void RArea::setInnerPos(const RPoint &pos)
{
    format_.pos.set(pos.x()-format_.padding.l, pos.y()-format_.padding.b, format_.pos.z());
}

void RArea::setMargin(int top, int bottom, int left, int right)
{
    format_.margin = { top, bottom, left, right };
}

void RArea::setMargin(int value)
{
    format_.margin = { value, value, value, value };
}

void RArea::setPadding(int top, int bottom, int left, int right)
{
    format_.padding = { top, bottom, left, right };
}

void RArea::setPadding(int value)
{
    format_.padding = { value, value, value, value };
}

void RArea::setMode(RArea::Mode mode)
{
    format_.mode = mode;
}

void RArea::setAlign(RArea::Align v, RArea::Align h)
{
    format_.align = { v, h };
}

RRect RArea::rect() const
{
    return { format_.size, format_.pos };
}

RSize RArea::size() const
{
    return format_.size;
}

RPoint RArea::pos() const
{
    return format_.pos;
}

int RArea::width() const
{
    return format_.size.width();
}

int RArea::height() const
{
    return format_.size.height();
}

int RArea::x() const
{
    return format_.pos.x();
}

int RArea::y() const
{
    return format_.pos.y();
}

int RArea::z() const
{
    return format_.pos.z();
}

int &RArea::rx()
{
    return format_.pos.rx();
}

int &RArea::ry()
{
    return format_.pos.ry();
}

int &RArea::rz()
{
    return format_.pos.rz();
}

RPoint RArea::centerPos() const
{
    return RPoint(format_.pos.x() + format_.size.width()/2, format_.pos.y() + format_.size.height()/2, format_.pos.z());
}

RRect RArea::outerRect() const
{
    return RRect(outerSize(), outerPos());
}

RSize RArea::outerSize() const
{
    return RSize(outerWidth(), outerHeight());
}

RPoint RArea::outerPos() const
{
    return RPoint(format_.pos.x()-format_.margin.l, format_.pos.y()-format_.margin.b, format_.pos.z());
}

int RArea::outerWidth() const
{
    return format_.size.width() + format_.margin.l + format_.margin.r;
}

int RArea::outerHeight() const
{
    return format_.size.height() + format_.margin.t + format_.margin.b;
}

RRect RArea::innerRect() const
{
    return RRect(innerSize(), innerPos());
}

RSize RArea::innerSize() const
{
    return RSize(innerWidth(), innerHeight());
}

RPoint RArea::innerPos() const
{
    return RPoint(format_.pos.x()+format_.padding.l, format_.pos.y()+format_.padding.b, format_.pos.z());
}

int RArea::innerWidth() const
{
    return format_.size.width() - format_.padding.l - format_.padding.r;
}

int RArea::innerHeight() const
{
    return format_.size.height() - format_.padding.t - format_.padding.b;
}

RArea::Mode RArea::mode() const
{
    return format_.mode;
}

RArea::Align RArea::vAlign() const
{
    return format_.align.v;
}

RArea::Align RArea::hAlign() const
{
    return format_.align.h;
}

RSize RArea::maxSize() const
{
    return RSize(format_.maxW, format_.maxH);
}

RSize RArea::minSize() const
{
    return RSize(format_.minW, format_.minH);
}

const RArea::Format &RArea::format() const
{
    return format_;
}
