#include "webwirepage.h"
#include "webwirehandler.h"
#include <QRegularExpression>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>

#define EVENT_CHECK_INTERVAL 10

WebWirePage::WebWirePage(QWidget *parent, int win, WebWireHandler *handler, QWebEngineProfile *profile) : QWebEnginePage(profile, parent)
{
    _win = win;
    _handler = handler;
    _accept_next_navigation = false;
    connect(&_evt_timer, &QTimer::timeout, this, &WebWirePage::getEvents);
    connect(this, &WebWirePage::loadFinished, this, &WebWirePage::startTimer);
    connect(this, &WebWirePage::loadStarted, this, &WebWirePage::stopTimer);
}

void WebWirePage::acceptNextNavigation()
{
    _accept_next_navigation = true;
}

void WebWirePage::getEvents(void)
{
    this->runJavaScript("window._web_wire_get_evts();", [this](const QVariant &r) {
        QString str = r.toString().trimmed();
        if (str != "[]" && str != "") {
            QJsonParseError err;
            QJsonDocument doc = QJsonDocument::fromJson(str.toUtf8(), &err);
            if (err.error != QJsonParseError::NoError) {
                _handler->error("Cannot interpret events from HTML: " + err.errorString());
                _handler->error("Got: " + str);
            } else {
                //_handler->message(str);
                QJsonArray events = doc.array();
                int i;
                for(i = 0; i < events.size(); i++) {
                    QJsonObject obj = events[i].toObject();
                    QString evt = "js-" + obj["evt"].toString();
                    QString id = obj["id"].toString();
                    QString js_evt;
                    QJsonDocument doc;
                    QJsonObject o;
                    o["data"] = obj["js_evt"];
                    doc.setObject(o);
                    js_evt = doc.toJson(QJsonDocument::Compact);
                    _handler->evt(evt + ":" + QString::number(_win) + ":" + id + ":" + js_evt);
                }
            }
        }
    });
}

void WebWirePage::startTimer(bool ok)
{
    _evt_timer.setInterval(EVENT_CHECK_INTERVAL);
    _evt_timer.start();

    // See acceptNavigationRequest() for why this is here.
    if (_navigation_event != "") {
        _handler->evt(_navigation_event);
        _navigation_event = "";
    }
}

void WebWirePage::stopTimer(void)
{
    _evt_timer.stop();
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

    QString u = url.toString();
    WinInfo_t *i = _handler->getWinInfo(_win);
    QString navigation_kind = "set-url";
    if (u.startsWith(i->base_url)) {
        navigation_kind = "set-html";
        u = u.mid(i->base_url.length());
    }

    QString r_u = u.replace("\"", "\\\"");
    // We need to postpone the url handling until loadFinished, otherwise we don't get the right results.
    // Loading will interfere between set-html / set-url commands and this running command.
    // So we put this to be give event already there for startTimer (which is called on loadFinish) to handle.
    _navigation_event = QString("navigate:") + QString::asprintf("%d:", _win) + "\"" + r_u + "\":" + navigation_type + ":" + navigation_kind;

    return false;
}
