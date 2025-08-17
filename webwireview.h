#ifndef WEBWIREVIEW_H
#define WEBWIREVIEW_H

#include <QObject>
#include <QWebEngineView>
#include <QWebEngineProfile>
#include <QWidget>

class WebWireHandler;
class WebWirePage;

class WebWireView : public QWebEngineView
{
    Q_OBJECT
private:
    int             _handle_nr;
    WebWireHandler *_handler;
    WebWirePage    *_page;
    int             _win;

public:
    WebWireView(QWebEngineProfile *profile, int win, WebWireHandler *h, QWidget *parent);

public:
    int setUrl(const QUrl &u);

public:
    void acceptNextNavigation();
    WebWirePage *page();

private slots:
    void urlProcessed(bool ok);
};

#endif // WEBWIREVIEW_H
