#include "vps.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    VPS w;
    w.show();
    return a.exec();
}
