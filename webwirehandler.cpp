#include "webwirehandler.h"

#include <QTimer>
#include <QWebEngineView>
#include <QIODevice>
#include <QRegularExpression>
#include <QApplication>
#include <QHttpServer>
#include <QTcpServer>
#include <QHttpServerResponse>
#include <QWebEngineProfile>

#include "consolelistener.h"
#include "webwirewindow.h"
#include "webwireview.h"
#include "webwirepage.h"


#define defun(name)         static void name(QString cmd, WebWireHandler *h, const QStringList &args)
#define r_ok(str)           h->addOk(str)
#define r_err(str)          h->addErr(str)
#define msg(str)            h->message(QObject::tr(str));
#define view(win)           h->getView(win)
#define check(cmd, vars)    h->getArgs(cmd, QList<Var>() << vars, args)
#define var(type, v)        Var(type, v, #v)

defun(cmdSetUrl)
{
    int win;
    QUrl url_location;

    if (check("set-url", var(integer, win) << var(url, url_location))) {
        WebWireWindow *w = h->getWindow(win);
        if (w == nullptr) {
            r_err(cmd + ": window " + QString::asprintf("%d", win) + " does not exist");
            return;
        }

        w->setUrl(url_location);
        r_ok("set");
    }
}

defun(cmdExecJs)
{
    int win;
    QString code;
    if (check("exec-js", var(integer, win) << var(string, code))) {
       int handle = h->execJs(win, code);
        if (handle == 0) {
           r_err(cmd + ": cannot execute code");
           return;
        } else {
            r_ok(QString::asprintf("%d", handle));
        }
    }
}


defun(cmdSetHtml)
{
    int win;
    QString file;
    if (check("set-html", var(integer, win) << var(string, file))) {
        QFile f(file);
        if (f.exists()) {
            if (f.isReadable()) {
                r_err(QString("file ") + file + " is not readable");
            } else {
                WebWireWindow *w = h->getWindow(win);
                if (w == nullptr) {
                    r_err(cmd + ": window " + QString::asprintf("%d", win) + " does not exist");
                    return;
                }

                WinInfo_t *i = h->getWinInfo(win);
                QString base_url = i->base_url;
                QString url = base_url + file;
                QUrl u(url);
                QString p_url = u.toString();
                h->message(QString("requesting: ") + p_url);

                int handle = w->setUrl(u);
                r_ok(QString::asprintf("%d", handle));
            }
        } else {
            r_err(QString("file ") + file + " does not exist");
        }

    }

}

defun(cmdExit)
{
    r_ok("exiting");
    h->closeListener();
    h->doQuit();
}

defun(cmdHelp)
{
    msg("new -> <win-id> - opens a new web wire window");
    msg("set-url <win> <url> - set webviewer <win> to load the given <url>");
    msg("close <win> - closes window <win>. It cannot be used after that");
    msg("move <win> <x> <y> - moves window <win> to screen coordinates x, y");
    msg("resize win <width> <height> - resizes window <win> to width, height");
    msg("set-title <title> - sets the window title");
    msg("set-icon <file path:<png|jpg|svg>> - sets the window icon to this bitmap file");
    msg("");
    //msg("add-css <win> <marker> - add css for <win>. The next lines until <marker> will be put into a css container");
    //msg("clear-css <win> - clear all added css for <win>");
    //msg("add-script <win> <marker> - add javascript scripting for <win>. The next lines until <marker> will be put into a scripting container");
    //msg("clear-scripts <win> - clear all added scripts for <win>");
    msg("set-html <win> <file> - set the html of the web-wire window <win> to file <file>.");
    msg("");
    msg("exit - exit web racket");

    h->addOk("help given");
}

defun(cmdMove)
{
    int win, x, y;

    if (check("move", var(integer, win) << var(integer, x) << var(integer, y))) {
        h->moveWindow(win, x, y);
    }

}

defun(cmdClose)
{
    int win;
    if (check("close", var(integer, win))) {
        h->closeWindow(win);
    }

}

defun(cmdResize)
{
    int win, width, height;
    if (check("resize", var(integer, win) << var(integer, width) << var(integer, height))) {
        h->resizeWindow(win, width, height);
    }

}

defun(cmdSetTitle)
{
    int win;
    QString title;
    if (check("set-title", var(integer, win) << var(string, title))) {
        h->setWindowTitle(win, title);
    }
}

defun(cmdSetIcon)
{

}

defun(cmdNewWindow)
{
    QString app_name;
    if (check("new", var(string, app_name))) {
        int win = h->newWindow(app_name);
        r_ok(QString::asprintf("%d", win));
    }
}

#undef msg
#undef view

#define fun(kind, name) if (cmd == kind) { name(cmd, this, args); }
#define efun(kind, name) else if (cmd == kind) { name(cmd, this, args); }

void WebWireHandler::processCommand(const QString &cmd, const QStringList &args)
{
    fun("set-url", cmdSetUrl)
    efun("exit", cmdExit)
    efun("help", cmdHelp)
    efun("move", cmdMove)
    efun("resize", cmdResize)
    efun("close", cmdClose)
    efun("set-title", cmdSetTitle)
    efun("set-icon", cmdSetIcon)
    efun("new", cmdNewWindow)
    efun("set-html", cmdSetHtml)
    efun("exec-js", cmdExecJs)
    else {
        WebWireHandler *h = this;
        r_err(QString::asprintf("Unknown command '%s'", cmd.toUtf8().data()));
    }
}

void WebWireHandler::addErr(const QString &msg)
{
    _reasons.prepend(msg);
}

void WebWireHandler::addOk(const QString &msg)
{
    _responses.append(msg);
}

void WebWireHandler::msg(const QString &msg)
{
    message(msg);
}

void WebWireHandler::message(const QString &msg)
{
    fprintf(_log_fh, "%s\n", msg.toUtf8().data());
    fprintf(stderr, "%s\n", msg.toUtf8().data());
    fflush(stderr);
}

QStringList WebWireHandler::splitArgs(QString l)
{
    int from = 0;
    int i, N;
    bool in_str = false;
    from = 0;
    QStringList r;
    bool prev_escape = false;
    for(i = 0, N = l.length(); i < N; ) {
        if (l[i].isSpace() && !in_str) {
            r.append(l.mid(from, i - from));
            while (i < N && l[i].isSpace()) { i++; }
            from = i;
        } else if (l[i] == '\"') {
            if (in_str) {
                if (!prev_escape) {
                    QString s = l.mid(from, i - from).replace("\\\"", "\"");
                    r.append(s);
                    i += 1;
                    while (i < N && l[i].isSpace()) { i++; }
                    from = i;
                } else {
                    i++;
                    prev_escape = false;
                }
            } else {
                in_str = true;
                i++;
                from = i;
            }
        } else if (l[i] == '\\') {
            if (in_str) { prev_escape = true; }
            i++;
        } else {
            if (in_str) { prev_escape = false; }
            i++;
        }
    }
    if (from != N) {
        r.append(l.mid(from));
    }

    {
        int i;
        for(i = 0; i < r.size(); i++) {
            msg(r[i]);
        }
    }

    return r;
}

void WebWireHandler::processInput(const QString &line)
{
    _reasons.clear();
    _responses.clear();

    QString l = line.trimmed();
    QStringList expr = splitArgs(l);

    if (!expr.empty() && expr.last() == "") {
        expr.removeLast();
    }
    if (expr.size() > 0) {
        QString cmd = expr[0].toLower();
        expr.remove(0);
        processCommand(cmd, expr);
    } else {
        _reasons.append("Does not compute");
    }
    if (_reasons.size() > 0) {
        QString msg = _reasons.join(", ");
        error(msg);
    } else {
        QString msg = _responses.join(", ");
        ok(msg);
    }
}

bool WebWireHandler::getArgs(QString cmd, QList<Var> types, QStringList args)
{
    if (args.size() != types.size()) {
        addErr(cmd + QString::asprintf(": incorrect number of arguments %d, expected %d", args.size(), types.size()));
        return false;
    }
    int i;
    for(i = 0; i < types.size(); i++) {
        VarType t = types[i].type;
        if (t == integer) {
            bool ok = true;
            *(types[i].i) = args[i].toInt(&ok);
            if (!ok) {
                addErr(cmd + ": " + types[i].name + ": expected integer, got " + args[i]);
                return false;
            }
        } else if (t == string) {
            *(types[i].s) = args[i];
        } else if (t == url) {
            QUrl u(args[i]);
            if (!u.isValid()) {
                addErr(cmd + ": " + types[i].name + ": url expected, got " + args[i]);
                return false;
            }
            *(types[i].u) = u;
        }
    }

    return true;
}

void WebWireHandler::error(const QString &msg)
{
    fprintf(_log_fh, "ERR:%s\n", msg.toUtf8().data());
    fprintf(stdout, "ERR:%s\n", msg.toUtf8().data());
    fflush(stdout);
}

void WebWireHandler::ok(const QString &msg)
{
    fprintf(_log_fh, "OK:%s\n", msg.toUtf8().data());
    fprintf(stdout, "OK:%s\n", msg.toUtf8().data());
    fflush(stdout);
}

void WebWireHandler::evt(const QString &msg)
{
    fprintf(_log_fh, "EVENT:%s\n", msg.toUtf8().data());
    fprintf(stdout, "EVENT:%s\n", msg.toUtf8().data());
    fflush(stdout);
}

void WebWireHandler::closeListener()
{
    _listener->close();
}

void WebWireHandler::doQuit()
{
    QList<int> wins = _windows.keys();
    int i;
    for(i = 0; i < wins.size(); i++) {
        windowCloses(wins[i], true);
    }
    _app->quit();
}


static QString pid()
{
    int _p = _getpid();
    unsigned long long p = _p;
    return QString::asprintf("%llu", p);
}

WebWireHandler::WebWireHandler(QApplication *app, int argc, char *argv[]) : QObject()
{
    _app = app;
    _listener = new ConsoleListener(this);
    connect(_listener, &ConsoleListener::newLine, this, &WebWireHandler::processInput);
    _window_nr = 0;
    _code_handle = 0;

    _server = new QHttpServer(this);
    QTcpServer *tcp_server = new QTcpServer(_server);
    bool listens = tcp_server->listen(QHostAddress("127.0.0.1"));
    if (!listens) {
        _port = -1;
    } else {
        _port = tcp_server->serverPort();
        _server->bind(tcp_server);
    }

    QDir tmp_dir(QDir::temp());
    QDir wr_dir = QDir(tmp_dir.absoluteFilePath("web-wire"));
    _my_dir = QDir(wr_dir.absoluteFilePath(pid()));
    if (!_my_dir.exists()) {
        _my_dir.mkpath(_my_dir.absolutePath());
    }

    QString log_file = _my_dir.absoluteFilePath("webracket.log");
    fopen_s(&_log_fh, log_file.toUtf8().data(), "wt");

    msg("Web Racket file store: " + _my_dir.absolutePath());
    msg("Web Racket log file: " + log_file);
    if (!listens) {
        evt("no-http-service");
    } else {
        msg(QString::asprintf("Web Racket Http Server on http://127.0.0.1:%d", _port));
    }

    _server->router()->clearConverters();
    _server->router()->addConverter(QMetaType(QMetaType::QString), ".*");
}

WebWireHandler::~WebWireHandler()
{
    QDir tmp_dir(QDir::temp());
    QDir wr_dir = QDir(tmp_dir.absoluteFilePath("web-wire"));
    QDir last = QDir(wr_dir.absoluteFilePath("last"));
    if (last.exists()) {
        last.removeRecursively();
    }
    QString from_dir = _my_dir.absolutePath();
    QString to_dir = last.absolutePath();

    msg("my dir   = " + from_dir);
    msg("last dir = " + to_dir);

    fclose(_log_fh);

    last.rename(from_dir, to_dir);
}

void WebWireHandler::windowCloses(int win, bool do_close)
{
    WebWireWindow *w = getWindow(win);
    if (w != nullptr) {
        QTimer *t = _timers[win];
        WinInfo_t *i = _infos[win];

        _windows.remove(win);
        _timers.remove(win);
        _infos.remove(win);

        if (do_close) {
            w->dontCallback();
            w->close();
        }

        delete t;
        delete i;
        delete w;

        evt(QString::asprintf("closed %d", win));
    }
}

void WebWireHandler::windowResized(int win, int w, int h)
{
    QTimer *t = _timers[win];
    WinInfo_t *i = _infos[win];
    i->size = QSize(w, h);
    i->size_set = true;
    t->start(250);
}

void WebWireHandler::windowMoved(int win, int x, int y)
{
    QTimer *t = _timers[win];
    WinInfo_t *i = _infos[win];
    i->pos = QPoint(x, y);
    i->pos_set = true;
    t->start(250);
}

void WebWireHandler::handleTimer(void)
{
    QTimer *t = qobject_cast<QTimer *>(sender());
    int win = t->property("win").toInt();
    if (_infos.contains(win)) {     // We don't want to process anything that has been destroyed
        WinInfo_t *i = _infos[win];

        if (i->pos_set) {
            evt(QString::asprintf("pos %d %d %d", win, i->pos.x(), i->pos.y()));
            i->pos_set = false;
        }
        if (i->size_set) {
            evt(QString::asprintf("size %d %d %d", win, i->size.width(), i->size.height()));
            i->size_set = false;
        }
    }
}

int WebWireHandler::newWindow(const QString &app_name)
{
    ++_window_nr;

    QTimer *t = new QTimer();
    _timers[_window_nr] = t;
    t->setProperty("win", _window_nr);
    t->setSingleShot(true);
    connect(t, &QTimer::timeout, this, &WebWireHandler::handleTimer);

    WinInfo_t *i = new WinInfo_t();
    _infos[_window_nr] = i;

    i->app_name = app_name;

    static QRegularExpression re_ws("\\s+");
    QString app_internal_name = app_name.trimmed().replace(re_ws, "_");
    i->base_url = QString::asprintf("http://127.0.0.1:%d/", _port) + app_internal_name + "/";

    i->profile = new QWebEngineProfile(app_internal_name);

    WebWireWindow *w = new WebWireWindow(this, _window_nr, app_name);
    _windows[_window_nr] = w;

    QHttpServerRouterRule *rule = _server->route("/" + app_internal_name + "/<arg>", this, [this](const QString &file) {
        this->message("Serving: " + file);
        QFile f(file);
        if (f.exists()) {
            return QHttpServerResponse::fromFile(file);
        } else {
            QHttpServerResponse resp(QHttpServerResponse::StatusCode::NotFound);
            return resp;
        }
    });

    i->rule = rule;

    w->show();

    return _window_nr;
}

WebWireWindow *WebWireHandler::getWindow(int win)
{
    if (_windows.contains(win)) {
        WebWireWindow *w = _windows[win];
        return w;
    } else {
        _reasons.append(QString::asprintf("Window %d not found", win));
       return nullptr;
    }
}

WinInfo_t *WebWireHandler::getWinInfo(int win)
{
    if (_infos.contains(win)) {
        return _infos[win];
    } else {
        _reasons.append(QString::asprintf("Windows Info for window %d not there, unexpected!", win));
        return nullptr;
    }
}

bool WebWireHandler::closeWindow(int win)
{
    WebWireWindow *w = getWindow(win);
    if (w != nullptr) w->close();
    return w != nullptr;
}

bool WebWireHandler::moveWindow(int win, int x, int y)
{
    WebWireWindow *w = getWindow(win);
    if (w != nullptr) w->move(x, y);
    return w != nullptr;
}

bool WebWireHandler::resizeWindow(int win, int width, int height)
{
    WebWireWindow *w = getWindow(win);
    if (w != nullptr) w->resize(width, height);
    return w != nullptr;
}

bool WebWireHandler::setWindowTitle(int win, const QString &title)
{
    WebWireWindow *w = getWindow(win);
    if (w != nullptr) w->setWindowTitle(title);
    return w != nullptr;
}

bool WebWireHandler::setWindowIcon(int win, const QIcon &icn)
{
    WebWireWindow *w = getWindow(win);
    if (w != nullptr) w->setWindowIcon(icn);
    return w != nullptr;
}

class ExecJs
{
private:
    WebWireHandler *_handler;
    int             _handle;
    int             _win;
public:
    void run(QWebEnginePage *p, const QString &code) {
        p->runJavaScript(code, [this](const QVariant &v) {
            QString result = v.toString().replace("\"", "\\\"");
            _handler->evt(QString::asprintf("exec-js-result: %d %d ", _win, _handle) + QString("\"") + result + QString("\""));
            delete this;
        });
    }
    ExecJs(WebWireHandler *handler, int win, int handle) {
        _handler = handler;
        _handle = handle;
        _win = win;
    }
};

int WebWireHandler::execJs(int win, const QString &code)
{
    WebWireWindow *w = getWindow(win);
    if (w != nullptr) {
        WebWireView *v = w->view();
        WebWirePage *p = v->page();
        _code_handle += 1;
        if (_code_handle == 0) { _code_handle += 1; }
        ExecJs *e = new ExecJs(this, win, ++_code_handle);
        e->run(p, code);
        return _code_handle;
    } else {
        return 0;
    }
}

QWebEngineView *WebWireHandler::getView(int win)
{
    WebWireWindow *w = getWindow(win);
    if (w != nullptr) { return w->view(); }
    else { return nullptr; }
}

void WebWireHandler::start()
{
    _listener->start();
    ok("WebWireHandler v0.1 started");
}

WinInfo_t::WinInfo_t() {
    size_set = false;
    pos_set = false;
    profile = nullptr;
    rule = nullptr;
}

WinInfo_t::~WinInfo_t() {
    if (profile != nullptr) {
        delete profile;
    }
}
