#include "mainwindow.h"
#include "form.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Form w;
//    MainWindow w("C:/Users/aaa/Desktop/test");
    w.show();
    return a.exec();
}
