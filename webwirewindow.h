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
    bool              _closing;

protected:
    virtual void closeEvent(QCloseEvent *evt);
    virtual void resizeEvent(QResizeEvent *evt);
    virtual void moveEvent(QMoveEvent *evt);
    virtual void contextMenuEvent(QContextMenuEvent *event);
    virtual void showEvent(QShowEvent *event);
    virtual void hideEvent(QHideEvent *event);

public:
    WebWireView *view();

public:
    void dontCallback();
    void setClosing(bool c);

public:
    int setUrl(const QUrl &u, int handle);

    QString showState();
    void setShowState(const QString &state);

public:
    explicit WebWireWindow(WebWireHandler *h, int win, const QString &app_name, WebWireWindow *parent = nullptr);

};

#endif // WEBWIREWINDOW_H
