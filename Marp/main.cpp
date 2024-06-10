#include "widget.h"

#include <QApplication>

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);

    Widget w("android-arp-64","interface-list-64","myIp-64","scan-64");
    w.show();
    return a.exec();
}
