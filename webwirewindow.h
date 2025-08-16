#ifndef WEBWIREWINDOW_H
#define WEBWIREWINDOW_H

#include <QMainWindow>

class WebWireHandler;
class QWebEngineView;
class WebWirePage;

class WebWireWindow : public QMainWindow
{
    Q_OBJECT
private:
    WebWireHandler   *_handler;
    QWebEngineView   *_view;
    WebWirePage      *_page;
    int               _win;
    bool              _callback;
    QString           _app_name;

protected:
    virtual void closeEvent(QCloseEvent *evt);
    virtual void resizeEvent(QResizeEvent *evt);
    virtual void moveEvent(QMoveEvent *evt);

public:
    QWebEngineView *view();

public:
    void dontCallback();

public:
    void setUrl(const QUrl &u);

public:
    explicit WebWireWindow(WebWireHandler *h, int win, const QString &app_name);

signals:
};

#endif // WEBWIREWINDOW_H
