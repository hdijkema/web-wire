#ifndef WEBWIREPAGE_H
#define WEBWIREPAGE_H

#include <QObject>
#include <QWebEnginePage>
#include <QWidget>

class QWebEngineProfile;
class WebWireHandler;

class WebWirePage : public QWebEnginePage
{
private:
    WebWireHandler *_handler;
    int             _win;
    bool            _accept_next_navigation;

public:
    void acceptNextNavigation();
    WebWirePage *page();

public:
    WebWirePage(QWidget *parent, int win, WebWireHandler *handler, QWebEngineProfile *profile);

    // QWebEnginePage interface
protected:
    bool acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame);
};

#endif // WEBWIREPAGE_H
