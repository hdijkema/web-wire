#ifndef WEBWIREWINDOW_H
#define WEBWIREWINDOW_H

#include <QMainWindow>

class WebWireHandler;
class QWebEngineView;

class WebWireWindow : public QMainWindow
{
    Q_OBJECT
private:
    WebWireHandler   *_handler;
    QWebEngineView   *_view;
    int               _win;
    bool              _callback;

protected:
    virtual void closeEvent(QCloseEvent *evt);
    virtual void resizeEvent(QResizeEvent *evt);
    virtual void moveEvent(QMoveEvent *evt);

public:
    QWebEngineView *view();

public:
    void dontCallback();

public:
    explicit WebWireWindow(WebWireHandler *h, int win);

signals:
};

#endif // WEBWIREWINDOW_H
