#include "webwirewindow.h"
#include "webwirehandler.h"
#include "webwirepage.h"

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

void WebWireWindow::setUrl(const QUrl &u)
{
    _page->acceptNextNavigation();
    _view->setUrl(u);
}

WebWireWindow::WebWireWindow(WebWireHandler *h, int win, const QString &app_name) : QMainWindow()
{
    _callback = true;
    _handler = h;
    _win = win;

    WinInfo_t *i = _handler->getWinInfo(_win);

    _view = new QWebEngineView(i->profile, this);

    _page = new WebWirePage(this, win, h, i->profile);
    _view->setPage(_page);

    this->setCentralWidget(_view);
}



