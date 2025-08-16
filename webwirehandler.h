#ifndef WEBWIREHANDLER_H
#define WEBWIREHANDLER_H

#include <QObject>

class QWebEngineView;
class QTimer;
class QIODevice;
class ConsoleListener;
class WebWireWindow;
class QHttpServer;
class QHttpServerRouterRule;
class QWebEngineProfile;

#include <QHash>
#include <QSize>
#include <QPoint>
#include <QDir>

typedef enum {
    integer,
    string,
    bitmap,
    url
} VarType;

class Var
{
public:
    QString  name;
    VarType  type;
    int     *i;
    QString *s;
    QBitmap *b;
    QUrl    *u;
public:
    Var(VarType t, int &v, const char *vname) { i = &v; type = t; name = vname; }
    Var(VarType t, QString &v, const char *vname) { s = &v; type = t; name = vname; }
    Var(VarType t, QBitmap *v, const char *vname) { b = v; type = t; name = vname; }
    Var(VarType t, QUrl &v, const char *vname) { u = &v; type = t; name = vname; }
};

class WinInfo_t
{
public:
    QSize  size;
    bool   size_set;
    QPoint pos;
    bool   pos_set;
    QString app_name;
    QHttpServerRouterRule *rule;
    QString base_url;
    QWebEngineProfile *profile;
public:
    WinInfo_t();
    ~WinInfo_t();
};

class WebWireHandler : public QObject
{
    Q_OBJECT
private:
    QHash<int, WebWireWindow *> _windows;
    QHash<int, QTimer *>          _timers;
    QHash<int, WinInfo_t *>       _infos;

    int                           _window_nr;
    int                           _code_handle;
    ConsoleListener              *_listener;
    QStringList                   _reasons;
    QStringList                   _responses;
    QApplication                 *_app;
    FILE                         *_log_fh;
    QDir                          _my_dir;

    QHttpServer                  *_server;
    int                           _port;

private:
    QStringList splitArgs(QString l);

private slots:
    void processInput(const QString &line);
    void handleTimer();

public:
    WebWireHandler(QApplication *app, int argc, char *argv[]);
    ~WebWireHandler();

    // WebWireWindow callbacks
public:
    void windowCloses(int win, bool do_close = false);
    void windowResized(int win, int w, int h);
    void windowMoved(int win, int x, int y);

    // WebWire Command handling
public:
    int newWindow(const QString &app_name);
    bool closeWindow(int win);
    bool moveWindow(int win, int x, int y);
    bool resizeWindow(int win, int w, int h);
    bool setWindowTitle(int win, const QString &title);
    bool setWindowIcon(int win, const QIcon &icn);
    int execJs(int win, const QString &code);

    // WebWire internal
public:
    QWebEngineView *getView(int win);
    WebWireWindow *getWindow(int win);
    WinInfo_t *getWinInfo(int win);

    void addErr(const QString &msg);
    void addOk(const QString &msg);

    void msg(const QString &msg);
    void message(const QString &msg);
    void error(const QString &msg);
    void ok(const QString &msg);
    void evt(const QString &msg);

    void closeListener();
    void doQuit();

    bool getArgs(QString cmd, QList<Var> types, QStringList args);

public:
    void start();

protected:
    void processCommand(const QString &cmd, const QStringList &args);
};

#endif // WEBWIREHANDLER_H
