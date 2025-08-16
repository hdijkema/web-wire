#include "webwirepage.h"
#include "webwirehandler.h"

WebWirePage::WebWirePage(QWidget *parent, int win, WebWireHandler *handler, QWebEngineProfile *profile) : QWebEnginePage(profile, parent)
{
    _win = win;
    _handler = handler;
    _accept_next_navigation = false;
}

void WebWirePage::acceptNextNavigation()
{
    _accept_next_navigation = true;
}

bool WebWirePage::acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame)
{
    if (_accept_next_navigation) {
        _accept_next_navigation = false;
        return QWebEnginePage::acceptNavigationRequest(url, type, isMainFrame);
    }

    QString navigation_type = "unknown";

    switch(type) {
        case NavigationTypeLinkClicked: navigation_type = "link-clicked";
            break;
        case NavigationTypeBackForward: navigation_type = "back-forward";
            break;
        case NavigationTypeReload: navigation_type = "reload";
            break;
        case NavigationTypeRedirect: navigation_type = "redirect";
            return QWebEnginePage::acceptNavigationRequest(url, type, isMainFrame);
            break;
        case NavigationTypeTyped: navigation_type = "typed";
            break;
        case NavigationTypeOther: navigation_type = "other";
            break;
        case NavigationTypeFormSubmitted: navigation_type = "form-submitted";
            break;
    }

    QString u = url.toString().replace("\"", "\\\"");

    _handler->evt(QString("navigate: ") + QString::asprintf("%d ", _win) + "\"" + u + "\" " + navigation_type);

    return false;
}
