#include "webwirewindow.h"
#include "webwirehandler.h"

#include <QWebEngineView>
#include <QResizeEvent>
#include <QMoveEvent>

void WebWireWindow::closeEvent(QCloseEvent *evt)
{
    if (_callback) {
        _handler->windowCloses(_win);
    }
    QMainWindow::closeEvent(evt);
}

void WebWireWindow::resizeEvent(QResizeEvent *evt)
{
    QSize s(evt->size());
    _handler->windowResized(_win, s.width(), s.height());
}

void WebWireWindow::moveEvent(QMoveEvent *evt)
{
    QPoint p(evt->pos());
    _handler->windowMoved(_win, p.x(), p.y());
}

QWebEngineView *WebWireWindow::view()
{
    return _view;
}

void WebWireWindow::dontCallback()
{
    _callback = false;
}

WebWireWindow::WebWireWindow(WebWireHandler *h, int win) : QMainWindow()
{
    _callback = true;
    _handler = h;
    _win = win;
    _view = new QWebEngineView(this);
    this->setCentralWidget(_view);
}



