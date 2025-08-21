#ifndef WEBWIREPAGE_H
#define WEBWIREPAGE_H

#include <QObject>
#include <QWebEnginePage>
#include <QWidget>
#include <QTimer>

class QWebEngineProfile;
class WebWireHandler;

class WebWirePage : public QWebEnginePage
{
private:
    WebWireHandler *_handler;
    int             _win;
    bool            _accept_next_navigation;
    QTimer          _evt_timer;

public:
    void acceptNextNavigation();
    WebWirePage *page();

public:
    WebWirePage(QWidget *parent, int win, WebWireHandler *handler, QWebEngineProfile *profile);

    // QWebEnginePage interface
protected:
    bool acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame);

private slots:
    void getEvents();
    void startTimer(bool ok);
    void stopTimer();
};

#endif // WEBWIREPAGE_H
