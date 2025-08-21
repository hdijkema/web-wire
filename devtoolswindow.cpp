#include "devtoolswindow.h"

#include <QWebEngineView>
#include <QWebEnginePage>
#include <QSettings>

DevToolsWindow::DevToolsWindow(QWidget *parent)
    : QMainWindow{parent}
{
    _devtools_view = new QWebEngineView(this);
    setCentralWidget(_devtools_view);

    QSettings s("web-wire");
    int x = s.value("x", 100).toInt();
    int y = s.value("y", 100).toInt();
    int w = s.value("w", 800).toInt();
    int h = s.value("h", 600).toInt();
    move(x, y);
    resize(w, h);
}

void DevToolsWindow::closeEvent(QCloseEvent *event)
{
    QSettings s("web-wire");
    QPoint p = pos();
    s.setValue("x", p.x());
    s.setValue("y", p.y());

    QSize sz = size();
    s.setValue("w", sz.width());
    s.setValue("h", sz.height());

    QMainWindow::closeEvent(event);
}

QWebEnginePage *DevToolsWindow::page()
{
    return _devtools_view->page();
}
