#include "webwirewindow.h"
#include "webwirehandler.h"
#include "webwirepage.h"
#include "webwireview.h"

#include <QWebEngineView>
#include <QResizeEvent>
#include <QMoveEvent>

void WebWireWindow::closeEvent(QCloseEvent *evt)
{
    if (_closing) {
        evt->accept();
        QMainWindow::closeEvent(evt);
        return;
    }

    if (_callback) {
        _handler->requestClose(_win);
    }
    evt->ignore();
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

void WebWireWindow::contextMenuEvent(QContextMenuEvent *event)
{
    _handler->evt(QString::asprintf("request-context-menu:%d:%d,%d;%d,%d", _win,
                                    event->x(), event->y(), event->globalX(), event->globalY()));
}

void WebWireWindow::showEvent(QShowEvent *event)
{
    _handler->evt(QString::asprintf("show-event:%d", _win));
}

void WebWireWindow::hideEvent(QHideEvent *event)
{
    _handler->evt(QString::asprintf("hide-event:%d", _win));
}

QString WebWireWindow::showState()
{
    if (isVisible()) {
        if (isFullScreen()) {
            return "fullscreen";
        } else if (isMaximized()) {
            return "maximized";
        } else if (isMinimized()) {
            return "minimized";
        } else {
            return "normal";
        }
    } else {
        return "hidden";
    }
}

void WebWireWindow::setShowState(const QString &state)
{
    if (state == "minimize") {
        showMinimized();
    } else if (state == "maximize") {
        showMaximized();
    } else if (state == "normalize") {
        showNormal();
    } else if (state == "fullscreen") {
        showFullScreen();
    } else if (state == "show") {
        show();
    } else if (state == "hide") {
        hide();
    }
}

WebWireView *WebWireWindow::view()
{
    return _view;
}

void WebWireWindow::dontCallback()
{
    _callback = false;
}

int WebWireWindow::setUrl(const QUrl &u, int handle)
{
    _view->acceptNextNavigation();
    return _view->setUrl(u, handle);
}

void WebWireWindow::setClosing(bool c)
{
    _closing = c;
}

WebWireWindow::WebWireWindow(WebWireHandler *h, int win, const QString &app_name, WebWireWindow *parent)
    : QMainWindow(parent)
{
    _callback = true;
    _closing = false;
    _handler = h;
    _win = win;

    WinInfo_t *i = _handler->getWinInfo(_win);
    _view = new WebWireView(i->profile, _win, h, this);
    this->setCentralWidget(_view);

    if (parent != nullptr) { // this will be a modal dialog window
        this->setWindowModality(Qt::WindowModality::WindowModal);
    }
}



