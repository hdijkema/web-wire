#ifndef WEBWIREWINDOW_H
#define WEBWIREWINDOW_H

#include <QMainWindow>

class WebWireHandler;
class WebWireView;
class WebWirePage;

class WebWireWindow : public QMainWindow
{
    Q_OBJECT
private:
    WebWireHandler   *_handler;
    WebWireView      *_view;
    WebWirePage      *_page;
    int               _win;
    bool              _callback;
    QString           _app_name;

protected:
    virtual void closeEvent(QCloseEvent *evt);
    virtual void resizeEvent(QResizeEvent *evt);
    virtual void moveEvent(QMoveEvent *evt);

public:
    WebWireView *view();

public:
    void dontCallback();

public:
    int setUrl(const QUrl &u);

public:
    explicit WebWireWindow(WebWireHandler *h, int win, const QString &app_name);

signals:
};

#endif // WEBWIREWINDOW_H
