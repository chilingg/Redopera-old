#include <RInfoWindow.h>
#include <iostream>

using namespace Redopera;

int main()
{
    RResource::setResourcePath("../../Redopera/");
    RInfoWIndow window;
    window.show();
    return window.exec();
}
