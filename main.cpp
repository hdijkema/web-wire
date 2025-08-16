#include <QApplication>
#include <QWebEngineView>
#include <QMainWindow>
#include "webwirehandler.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(false);

    WebWireHandler *handler = new WebWireHandler(&a, argc, argv);
    handler->start();

    int retval = a.exec();

    delete handler;

    return retval;
}
