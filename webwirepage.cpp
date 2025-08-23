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

    if (url.scheme() == "click") {
        QString s = url.toString();
        static QRegularExpression re("([^:]+)[:](.*)");
        QRegularExpressionMatch m = re.match(s);
        //QString click = m.captured(1);
        QString id = m.captured(2);
        id = id.replace("\"", "\\\"");
        _handler->evt(QString::asprintf("click:%d:", _win) + QString("\"") + id + "\"");
    } else {
        QString u = url.toString().replace("\"", "\\\"");
        _handler->evt(QString("navigate:") + QString::asprintf("%d:", _win) + "\"" + u + "\":" + navigation_type);
    }

    return false;
}
