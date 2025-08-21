#include "webwireview.h"

#include "webwirehandler.h"
#include "webwirepage.h"
#include "webwireprofile.h"

WebWireView::WebWireView(WebWireProfile *profile, int win, WebWireHandler *h, QWidget *parent)
    : QWebEngineView(profile, parent)
{
    _handle_nr = 0;
    _current_handle_nr = -1;
    _handler = h;
    _win = win;

    _page = new WebWirePage(this, win, h, profile);
    setPage(_page);

    this->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);

    connect(_page, &WebWirePage::loadFinished, this, &WebWireView::urlProcessed);
}

int WebWireView::setUrl(const QUrl &u, int handle)
{
    //int handle = ++_handle_nr;
    int r_handle = handle;
    _current_handle_nr = handle;
    QWebEngineView::setUrl(u);
    return r_handle;
}

void WebWireView::urlProcessed(bool ok)
{
    if (_current_handle_nr > 0) {
        _handler->evt(QString::asprintf("page-loaded:%d:%d:%s", _win, _current_handle_nr, ok ? "true" : "false"));
        _current_handle_nr = -1;
    }
}

void WebWireView::acceptNextNavigation()
{
    _page->acceptNextNavigation();
}

WebWirePage *WebWireView::page()
{
    return _page;
}


