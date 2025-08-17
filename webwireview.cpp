#include "webwireview.h"

#include "webwirehandler.h"
#include "webwirepage.h"

WebWireView::WebWireView(QWebEngineProfile *profile, int win, WebWireHandler *h, QWidget *parent)
    : QWebEngineView(profile, parent)
{
    _handle_nr = 0;
    _handler = h;
    _win = win;

    _page = new WebWirePage(this, win, h, profile);
    setPage(_page);

    connect(_page, &WebWirePage::loadFinished, this, &WebWireView::urlProcessed);
}

int WebWireView::setUrl(const QUrl &u)
{
    int handle = ++_handle_nr;
    QWebEngineView::setUrl(u);
    return handle;
}

void WebWireView::urlProcessed(bool ok)
{
    _handler->evt(QString::asprintf("set-html/url-processed: %d %d %s", _win, _handle_nr, ok ? "true" : "false"));
}

void WebWireView::acceptNextNavigation()
{
    _page->acceptNextNavigation();
}

WebWirePage *WebWireView::page()
{
    return _page;
}


