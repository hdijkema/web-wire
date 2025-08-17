#include "webwirewindow.h"
#include "webwirehandler.h"
#include "webwirepage.h"
#include "webwireview.h"

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

WebWireView *WebWireWindow::view()
{
    return _view;
}

void WebWireWindow::dontCallback()
{
    _callback = false;
}

int WebWireWindow::setUrl(const QUrl &u)
{
    _view->acceptNextNavigation();
    return _view->setUrl(u);
}

WebWireWindow::WebWireWindow(WebWireHandler *h, int win, const QString &app_name) : QMainWindow()
{
    _callback = true;
    _handler = h;
    _win = win;

    WinInfo_t *i = _handler->getWinInfo(_win);

    _view = new WebWireView(i->profile, _win, h, this);


    this->setCentralWidget(_view);
}



