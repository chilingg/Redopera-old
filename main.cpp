#include "Testctr.h"

#include <RDebug.h>

int main()
{
    RWindowCtrl window;
    window.showWindow();
    TestCtr ctrl("Test", &window);

    return window.exec();
}
