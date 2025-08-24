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
#include <QIcon>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonParseError>
#include <QJsonObject>
#include <QMenu>
#include <QMenuBar>
#include <QSettings>
#include <QFileDialog>

#include "webwire.h"
#include "consolelistener.h"
#include "webwirewindow.h"
#include "webwireview.h"
#include "webwirepage.h"
#include "webwireprofile.h"
#include "execjs.h"
#include "default_css.h"
#include "devtoolswindow.h"
#include "default_css.h"

#ifdef Q_OS_LINUX
#include <unistd.h>
#endif


#define defun(name)         static void name(QString cmd, WebWireHandler *h, const QStringList &args)

#define r_ok(str)           h->addOk(str)
#define r_nok(str)          h->addNOk(str)
#define r_err(str)          h->addErr(str)
#define msg(str)            h->message(QObject::tr(str));

#define view(win)           h->getView(win)

#define var(type, v)        Var(type, v, #v)
#define opt(type, v, d)     Var(type, v, #v, true, d)

#define check(cmd, vars)    h->getArgs(cmd, win, QList<Var>() << vars, args)
#define checkWin            WebWireWindow *w = h->getWindow(win); \
                            if (w == nullptr) { \
                               r_err(cmd + ": window " + QString::asprintf("%d", win) + " does not exist"); \
                               r_nok(cmd + ":" + QString::asprintf("%d", win)); \
                               return; \
                            }

#define js_el(id, code)     QString("let el = document.getElementById('") + id + "');" + \
                            QString("if (el !== null) { ") + code + "}"

defun(cmdSetUrl)
{
    int win = -1;
    int handle = -1;
    QUrl url_location;

    if (check("set-url", var(integer, win) << var(integer, handle) << var(url, url_location))) {
        checkWin;

        w->setUrl(url_location, handle);
        r_ok(QString::asprintf("set-url:%d:done", win));
    }
}

defun(cmdExecJs)
{
    int win = -1;
    int handle = -1;
    QString code;
    if (check("exec-js", var(integer, win) << var(integer, handle) << var(string, code))) {
       int r_handle = h->execJs(win, handle, code);
        if (r_handle == 0) {
           r_err(cmd + ": cannot execute code");
           return;
        } else {
            r_ok(QString::asprintf("exec-js:%d:%d", win, r_handle));
        }
    }
}


defun(cmdSetHtml)
{
    int win = -1;
    int handle = -1;
    QString file;
    if (check("set-html", var(integer, win) << var(integer, handle) << var(string, file))) {
        QFileInfo f(file);
        if (f.exists()) {
            if (!f.isReadable()) {
                r_err(QString::asprintf("set-html:%d:file ", win) + file + " is not readable");
                r_nok(QString::asprintf("set-html:%d", win));
            } else {
                checkWin;

                WinInfo_t *i = h->getWinInfo(win);
                QString base_url = i->base_url;
                QString url = base_url + file;
                QUrl u(url);
                QString p_url = u.toString();
                h->message(QString("requesting: ") + p_url);

                int r_handle = w->setUrl(u, handle);
                r_ok(QString::asprintf("set-html:%d:%d", win, r_handle));
            }
        } else {
            r_err(QString::asprintf("set-html:%d:file ", win) + file + " does not exist");
            r_nok(QString::asprintf("set-html:%d", win));
        }
    }
}

defun(cmdSetInnerHtml)
{
    int win = -1;
    int handle = -1;
    QString id;
    QString data;
    if (check("set-inner-html", var(integer, win) << var(integer, handle) << var(string, id) << var(string, data))) {
        QFileInfo f(data);
        bool is_file = false;
        if (f.exists()) {
            if (f.isReadable()) {
                is_file = true;
            }
        }

        checkWin;

        int r_handle;
        WinInfo_t *i = h->getWinInfo(win);
        if (is_file) {
            QString base_url = i->base_url;
            QString url = base_url + data;
            QUrl u(url);
            QString p_url = u.toString();
            //h->message(QString("requesting: ") + p_url);
            r_handle = i->profile->set_html(h, win, handle, id, p_url, true);
        } else {
            r_handle = i->profile->set_html(h, win, handle, id, data, false);
        }
        r_ok(QString::asprintf("set-inner-html:%d:%d", win, r_handle));
    }
}

defun(cmdGetInnerHtml)
{
    int win = -1;
    int handle = -1;
    QString id;
    if (check("get-inner-html", var(integer, win) << var(integer, handle) << var(string, id))) {
        checkWin;

        int r_handle;
        WinInfo_t *i = h->getWinInfo(win);
        r_handle = i->profile->get_html(h, win, handle, id);
        r_ok(QString::asprintf("get-inner-html:%d:%d", win, r_handle));
    }
}

defun(cmdSetAttr)
{
    int win = -1;
    int handle = -1;
    QString id;
    QString attr;
    QString val;

    if (check("set-attr", var(integer, win) << var(integer, handle) << var(string, id) << var(string, attr) << var(string, val))) {
        checkWin;

        int r_handle;
        WinInfo_t *i = h->getWinInfo(win);
        r_handle = i->profile->set_attr(h, win, handle, id, attr, val);
        r_ok(QString::asprintf("set-attr:%d:%d", win, r_handle));
    }
}

defun(cmdGetAttr)
{
    int win = -1;
    int handle = -1;
    QString id;
    QString attr;

    if (check("get-attr", var(integer, win) << var(integer, handle) << var(string, id) << var(string, attr))) {
        checkWin;

        int r_handle;
        WinInfo_t *i = h->getWinInfo(win);
        r_handle = i->profile->get_attr(h, win, handle, id, attr);
        r_ok(QString::asprintf("get-attr:%d:%d", win, r_handle));
    }
}

defun(cmdGetAttrs)
{
    int win = -1;
    int handle = -1;
    QString id;

    if (check("get-attrs", var(integer, win) << var(integer, handle) << var(string, id))) {
        checkWin;

        int r_handle;
        WinInfo_t *i = h->getWinInfo(win);
        r_handle = i->profile->get_attrs(h, win, handle, id);
        r_ok(QString::asprintf("get-attrs:%d:%d", win, r_handle));
    }
}

defun(cmdGetElements)
{
    int win = -1;
    int handle = -1;
    QString selector;

    if (check("get-attrs", var(integer, win) << var(integer, handle) << var(string, selector))) {
        checkWin;

        int r_handle;
        WinInfo_t *i = h->getWinInfo(win);
        r_handle = i->profile->get_elements(h, win, handle, selector);
        r_ok(QString::asprintf("get-elements:%d:%d", win, r_handle));
    }
}

defun(cmdDelAttr)
{
    int win = -1;
    int handle = -1;
    QString id;
    QString attr;

    if (check("del-attr", var(integer, win) << var(integer, handle) << var(string, id) << var(string, attr))) {
        checkWin;

        int r_handle;
        WinInfo_t *i = h->getWinInfo(win);
        r_handle = i->profile->del_attr(h, win, handle, id, attr);
        r_ok(QString::asprintf("del-attr:%d:%d", win , r_handle));
    }
}

defun(cmdSetStyle)
{
    int win = -1;
    int handle = -1;
    QString id;
    QString val;

    if (check("set-style", var(integer, win) << var(integer, handle) << var(string, id) << var(string, val))) {
        checkWin;

        int r_handle;
        WinInfo_t *i = h->getWinInfo(win);
        r_handle = i->profile->set_style(h, win, handle, id, val);
        r_ok(QString::asprintf("set-style:%d:%d", win, r_handle));
    }
}

defun(cmdAddStyle)
{
    int win = -1;
    int handle = -1;
    QString id;
    QString val;

    if (check(cmd, var(integer, win) << var(integer, handle) << var(string, id) << var(string, val))) {
        checkWin;

        int r_handle;
        WinInfo_t *i = h->getWinInfo(win);
        r_handle = i->profile->add_style(h, win, handle, id, val);
        r_ok(QString::asprintf("add-style:%d:%d", win, r_handle));
    }
}

defun(cmdGetStyle)
{
    int win = -1;
    int handle;
    QString id;

    if (check("get-styler", var(integer, win) << var(integer, handle) << var(string, id))) {
        checkWin;

        int r_handle;
        WinInfo_t *i = h->getWinInfo(win);
        r_handle = i->profile->get_style(h, win, handle, id);
        r_ok(QString::asprintf("get-style:%d:%d", win, r_handle));
    }
}

defun(cmdOn)
{
    int win = -1;
    int handle = -1;
    QString event;
    QString id;

    if (check("on", var(integer, win) << var(integer, handle) << var(string, event) << var(string, id))) {
        checkWin;

        id = id.replace("'", "\\'");
        QString js_set_on_evt   = "{"
                                  "  let el = document.getElementById('" + id + "');"
                                  "  if (el !== undefined) {"
                                  "     el.addEventListener("
                                  "        '" + event + "', "
                                  "        function(e) {"
                                  "           let obj = {evt: '" + event + "', id: '" + id +"', js_evt: window._web_wire_event_info('" + event + "', '" + id + "', e) };"
                                  "           window._web_wire_put_evt(obj);"
                                  "        }"
                                  "     );"
                                  "  }"
                                  "}";

        int r_handle = h->execJs(win, handle, js_set_on_evt, true, "on");

        r_ok(QString::asprintf("on:%d:%d:%s", win, r_handle, event.toUtf8().data()));
    }
}

defun(cmdBind)
{
    int win = -1;
    int handle = -1;
    QString event;
    QString selector;

    if (check("bind", var(integer, win) << var(integer, handle) << var(string, event) << var(string, selector))) {
        checkWin;

        selector = selector.replace("'", "\\'");
        event = event.replace("'", "\\'");

        QString js_bind_evt = "window._web_wire_bind_evt_ids('" + selector + "', '" + event + "');";

        int r_handle = h->execJs(win, handle, js_bind_evt, false, "bind");

        r_ok(QString::asprintf("bind:%d:%d:%s", win, r_handle, event.toUtf8().constData()));
    }
}

defun(cmdElementInfo)
{
    int win = -1;
    int handle = -1;
    QString id;

    if (check("element-info", var(integer, win) << var(integer, handle) << var(string, id))) {
        checkWin;

        QString _id = id;
        _id = _id.replace("'", "\\'");

        QString js = "{"
                     "  let el = document.getElementById('" + _id + "');"
                     "  let obj;"
                     "  if ((el === null) || (el === undefined)) {"
                     "    obj = [ '" + _id + "', '', '', false ];"
                     "  } else {"
                     "    let type = el.getAttribute('type');"
                     "    if (type === null) { type = ''; }"
                     "    obj = [ '" + _id + "', el.nodeName, type, true ];"
                     "  }"
                     "  let ret = 'json:' + JSON.stringify(obj);"
                     "  ret;"
                     "}";

        int r_handle = h->execJs(win, handle, js, false, "element-info");
        id = id.replace("\"", "\\\"");
        r_ok(QString::asprintf("element-info:%d:%d:%s", win, r_handle,
                               id.toUtf8().constData()));
    }
}

defun(cmdValue)
{
    int win = -1;
    int handle = -1;
    QString id;
    QString val;
    QString dummy = "@@WEB-WIRE-NONE-GIVEN@@";

    if (check("value", var(integer, win) << var(integer, handle) << var(string, id) << opt(string, val, dummy))) {
        checkWin;

        QString _id = id;
        _id = _id.replace("'", "\\'");

        QString js_value;
        if (val == dummy) {
            js_value = "{"
                       "   let el = document.getElementById('" + _id + "');"
                       "   el.value;"
                       "}";
        } else {
            val = val.replace("'", "\\'");
            js_value = "{"
                       "   let el = document.getElementById('" + _id + "');"
                       "   el.value = '" + val + "';"
                       "   el.value;"
                       "}";
        }
        int r_handle = h->execJs(win, handle, js_value, false, "value");
        id = id.replace("\"" , "\\\"");
        r_ok(QString::asprintf("value:%d:%d:%s", win, r_handle, id.toUtf8().data()));
    }
}

defun(cmdAddClass)
{
    int win = -1;
    int handle = -1;
    QString id;
    QString cl;

    if (check(cmd, var(integer, win) << var(integer, handle) << var(string, id) << var(string, cl))) {
        checkWin;

        QString _id = id;
        _id = _id.replace("'", "\\'");

        QString _cl = cl;
        _cl = _cl.replace("'", "\\'");

        QString js = QString("{") +
                     js_el(_id,
                        "let cl = el.getAttribute('class');"
                        "if (cl === null) { cl = ''; }"
                        "let _cl = '" + _cl + "';"
                        "cl = cl.replace(_cl, '');"
                        "cl += ' ' + _cl;"
                        "cl = cl.replace(/\\s+/g, ' ');"
                        "el.setAttribute('class', cl);"
                    ) + "}";

        int r_handle = h->execJs(win, handle, js, true, "add-class");
        r_ok(QString::asprintf("add-class:%d:%d", win, r_handle));
    }
}

defun(cmdRemoveClass)
{
    int win = -1;
    int handle = -1;
    QString id;
    QString cl;

    if (check(cmd, var(integer, win) << var(integer, handle) << var(string, id) << var(string, cl))) {
        checkWin;

        QString _id = id;
        _id = _id.replace("'", "\\'");

        QString _cl = cl;
        _cl = _cl.replace("'", "\\'");

        QString js = QString("{") +
                     js_el(_id,
                           "let cl = el.getAttribute('class');"
                           "if (cl === null) { cl = ''; }"
                           "let _cl = '" + _cl + "';"
                           "cl = cl.replace(_cl, '');"
                           "cl = cl.replace(/\\s+/g, ' ');"
                           "el.setAttribute('class', cl);"
                    ) + "}";
        int r_handle = h->execJs(win, handle, js, true, "remove-class");
        r_ok(QString::asprintf("remove-class:%d:%d", win, r_handle));
    }
}

defun(cmdSetMenu)
{
    int win = -1;
    QString menu;

    if (check("set-menu", var(integer, win) << var(json_string, menu))) {
        checkWin;

        if (h->setMenu(win, menu)) {
            r_ok(QString::asprintf("set-menu:%d", win));
        } else {
            r_nok(QString::asprintf("set-menu:%d", win));
        }
    }
}


defun(cmdExit)
{
    r_ok("exit:done");
    h->closeListener();
    h->doQuit();
}

defun(cmdDebug)
{
    int win = -1;
    if (check("debug", var(integer, win))) {
        checkWin;

        h->debugWin(win);

        r_ok(QString::asprintf("debug:%d", win));
    }
}


defun(cmdMove)
{
    int win = -1;
    int x = -1;
    int y = -1;

    if (check("move", var(integer, win) << var(integer, x) << var(integer, y))) {
        checkWin

        h->moveWindow(win, x, y);
        r_ok(QString::asprintf("move:%d", win));
    }

}

defun(cmdClose)
{
    int win = -1;
    if (check("close", var(integer, win))) {
        checkWin

        h->closeWindow(win);
        r_ok(QString::asprintf("close:%d", win));
    }
}

defun(cmdSetShowState)
{
    int win = -1;
    QString state;
    if (check("set-show-state", var(integer, win) << var(string, state))) {
        checkWin;

        if (state != "minimize" && state != "maximize" && state != "normalize" &&
            state != "show" && state != "hide" && state != "fullscreen") {
            r_err(QString::asprintf("set-show-state:%d:", win) + QString("State '") + state + "' is not correct");
            r_nok(QString::asprintf("set-show-state:%d", win));
        } else {
            h->setShowState(win, state);
            r_ok(QString::asprintf("set-show-state:%d:%s", win, h->showState(win).toUtf8().constData()));
        }
    }
}

defun(cmdShowState)
{
    int win = -1;
    if (check("show-state", var(integer, win))) {
        checkWin;

        QString st = h->showState(win);
        r_ok(QString::asprintf("show-state:%d:%s", win, st.toUtf8().constData()));
    }
}

defun(cmdResize)
{
    int win = -1, width = -1, height = -1;
    if (check("resize", var(integer, win) << var(integer, width) << var(integer, height))) {
        checkWin

        h->resizeWindow(win, width, height);
        r_ok(QString::asprintf("resize:%d", win));
    }

}

defun(cmdSetTitle)
{
    int win = -1;
    QString title;
    if (check("set-title", var(integer, win) << var(string, title))) {
        checkWin

        h->setWindowTitle(win, title);
        r_ok(QString::asprintf("set-title:%d", win));
    }
}

defun(cmdSetIcon)
{
    int win = -1;
    QString icon_file;
    if (check("set-icon", var(integer, win) << var(string, icon_file))) {
        checkWin

        QFileInfo f(icon_file);
        if (f.exists() && f.isReadable()) {
            QIcon icn(icon_file);
            h->setWindowIcon(win, icn);
            r_ok(QString::asprintf("set-icon:%d", win));
        } else {
            if (!f.exists()) {
                r_err(QString::asprintf("set-icon:%d:Icon File '", win) + icon_file + "' does not exist");
                r_nok(QString::asprintf("set-icon:%d", win));
                return;
            }

            if (!f.isReadable()) {
                r_err(QString::asprintf("set-icon:%d:Icon file '", win) + icon_file + "' is not readable");
                r_nok(QString::asprintf("set-icon:%d", win));
                return;
            }
        }
    }
}

defun(cmdFileOpen)
{
    int win = -1;
    QString title;
    QString directory;
    QString file_types;
    if (check("file-open", var(integer, win) << var(string, title) << var(string, directory) << var(string, file_types))) {
        checkWin;

        bool ok;
        QString result = h->fileOpen(win, title, directory, file_types, ok);
        if (ok) {
            QString r = QString("\"") + result.replace("\"", "\\\"") + "\"";
            r_ok(QString::asprintf("file-open:%d:%s", win, r.toUtf8().constData()));
        } else {
            r_nok(QString::asprintf("file-open:%d", win));
        }
    }
}

defun(cmdFileSave)
{
    int win = -1;
    QString title;
    QString directory;
    QString file_types;
    bool overwrite = false;

    if (check("file-save", var(integer, win) << var(string, title) << var(string, directory) << var(string, file_types) << opt(boolean, overwrite, false))) {
        checkWin;

        bool ok;
        QString result = h->fileSave(win, title, directory, file_types, overwrite, ok);
        if (ok) {
            QString r = QString("\"") + result.replace("\"", "\\\"") + "\"";
            r_ok(QString::asprintf("file-save:%d:%s", win, r.toUtf8().constData()));
        } else {
            r_nok(QString::asprintf("file-save:%d", win));
        }
    }
}

defun(cmdChooseDir)
{
    int win = -1;
    QString title;
    QString directory;
    if (check("choose-dir", var(integer, win) << var(string, title) << var(string, directory))) {
        checkWin;

        bool ok;
        QString result = h->chooseDir(win, title, directory, ok);
        if (ok) {
            QString r = QString("\"") + result.replace("\"", "\\\"") + "\"";
            r_ok(QString::asprintf("file-open:%d:%s", win, r.toUtf8().constData()));
        } else {
            r_nok(QString::asprintf("file-open:%d", win));
        }
    }
}


defun(cmdNewWindow)
{
    QString profile;
    int parent_win_id = -1;
    int win = 0;
    if (check("new", var(string, profile) << opt(integer, parent_win_id, -1))) {
        int win = h->newWindow(profile, parent_win_id);
        r_ok(QString::asprintf("new:%d", win));
    }
}

defun(cmdCwd)
{
    QString cwd;
    int win = 0;
    if (check("cwd", opt(string, cwd, ""))) {
        if (cwd != "") {
            QDir::setCurrent(cwd);
        }
        QString d = QDir::currentPath();
        r_ok("cwd:\"" + d + "\"");
    }
}

defun(cmdProtocol)
{
    r_ok(QString::asprintf("protocol:%s", WEB_WIRE_PROTOCOL_VERSION));
}

defun(cmdSetStylesheet)
{
    QString json_css;
    int win = 0;
    if (check("set-stylesheet", var(string, json_css))) {
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(json_css.toUtf8(), &err);
        if (err.error != QJsonParseError::NoError) {
            r_nok(QString("set-stylesheet:error") + err.errorString());
        } else {
            QJsonObject obj = doc.object();
            QString css = obj["css"].toString();
            h->setStylesheet(css);
            r_ok(QString("set-stylesheet:done"));
        }
    }
}

defun(cmdGetStyleheet)
{
    QString css = h->getStylesheet();
    QJsonDocument doc;
    QJsonObject obj;
    obj["css"] = css;
    doc.setObject(obj);
    QString json = doc.toJson(QJsonDocument::Compact);
    r_ok(QString("get-stylesheet:") + json);
}

defun(cmdHelp)
{
    msg("new <profile> [<win-id>] -> <win-id> - opens a new web wire window with given profile (for cookie storage).");
    msg("                                       The optional <win-id> is a parent window, in which case a modal dialog");
    msg("                                       will be created.");
    msg("close <win> - closes window <win>. It cannot be used after that");
    msg("move <win> <x> <y> - moves window <win> to screen coordinates x, y");
    msg("resize win <width> <height> - resizes window <win> to width, height");
    msg("set-title <title> - sets the window title");
    msg("set-icon <file path:<png|jpg|svg>> - sets the window icon to this bitmap file");
    msg("");
    msg("set-url <win-id> <url> - set webviewer <win-id> to load the given <url>");
    msg("set-html <win-id> <file> - set the html content of the web-wire window <win-id> to file <file>.");
    msg("set-inner-html <win-id> <id> <file|html> - set the inner html of the dom element with id <id> to the contents of <html|file>.");
    msg("get-inner-html <win-id> <id> - get the inner html of the dom element with id <id>.");
    msg("");
    msg("on <win-id> <event> <id> - make the <id> of the html of <win-id> trigger a <event>, ");
    msg("                           event can be any javascript DOM event, e.g. click, input, mousemove, etc.");
    msg("value <win-id> <id> [<value>] - get or set the value of id, always returns the current value by event");
    msg("");
    msg("exit - exit web racket");

    r_ok("help:given");
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
    efun("set-inner-html", cmdSetInnerHtml)
    efun("get-inner-html", cmdGetInnerHtml)
    efun("set-attr", cmdSetAttr)
    efun("get-attr", cmdGetAttr)
    efun("get-attrs", cmdGetAttrs)
    efun("get-elements", cmdGetElements)
    efun("del-attr", cmdDelAttr)
    efun("add-style", cmdAddStyle)
    efun("set-style", cmdSetStyle)
    efun("get-style", cmdGetStyle)
    efun("cwd", cmdCwd)
    efun("on", cmdOn)
    efun("bind", cmdBind)
    efun("element-info", cmdElementInfo)
    efun("value", cmdValue)
    efun("set-menu", cmdSetMenu)
    efun("protocol", cmdProtocol)
    efun("add-class", cmdAddClass)
    efun("remove-class", cmdRemoveClass)
    efun("debug", cmdDebug)
    efun("set-show-state", cmdSetShowState)
    efun("show-state", cmdShowState)
    efun("set-stylesheet", cmdSetStylesheet)
    efun("get-stylesheet", cmdGetStyleheet)
    efun("file-open", cmdFileOpen)
    efun("file-save", cmdFileSave)
    efun("choose-dir", cmdChooseDir)
    else {
        WebWireHandler *h = this;
        r_err(QString::asprintf("Unknown command '%s'", cmd.toUtf8().data()));
        r_nok(cmd + ":unknown:Unknown command");
    }
}

void WebWireHandler::addErr(const QString &msg)
{
    _reasons.append(msg);
}

void WebWireHandler::addOk(const QString &msg)
{
    _responses.append("OK:" + msg);
}

void WebWireHandler::addNOk(const QString &msg)
{
    _responses.append("NOK:" + msg);
}

void WebWireHandler::msg(const QString &msg)
{
    message(msg);
}

QStringList WebWireHandler::splitArgs(QString l)
{
    int from = 0;
    int i, N;
    bool in_str = false;
    from = 0;
    QStringList r;

    auto append = [this, &r](const QString &s) {
        r.append(s);
        //message(QString("Appending: ") + s);
    };
\
    bool prev_escape = false;
    for(i = 0, N = l.length(); i < N; ) {
        if (l[i].isSpace() && !in_str) {
            append(l.mid(from, i - from));
            while (i < N && l[i].isSpace()) { i++; }
            from = i;
        } else if (l[i] == '\"') {
            if (in_str) {
                if (!prev_escape) {
                    QString s = l.mid(from, i - from).replace("\\\"", "\"");
                    append(s);
                    i += 1;
                    while (i < N && l[i].isSpace()) { i++; }
                    from = i;
                    in_str = false;
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
        append(l.mid(from));
    }

    /*{
        int i;
        for(i = 0; i < r.size(); i++) {
            msg(r[i]);
        }
    }*/

    return r;
}

void WebWireHandler::processInput(const QString &line)
{
    _reasons.clear();
    _responses.clear();

    QString l = line.trimmed();
    QStringList expr = splitArgs(l);

    //if (!expr.empty() && expr.last() == "") {
    //    expr.removeLast();
    //}
    if (expr.size() > 0) {
        QString cmd = expr[0].toLower();
        expr.remove(0);
        processCommand(cmd, expr);
    } else {
        _reasons.append("Does not compute");
        _responses.append("NOK:" + l);
    }

    if (_reasons.size() > 0) {
        int i;
        for(i = 0; i < _reasons.size(); i++) {
            QString msg = _reasons[i];
            error(msg);
        }
    }

    if (_responses.size() > 0) {
        QString msg = _responses.join(", ");
        ok(msg);
    }
}

void WebWireHandler::inputStopped()
{
    log(nullptr, _log_fh, "Unexpected:%s", "Input has stopped");
    closeListener();
    doQuit();
}

bool WebWireHandler::getArgs(QString cmd, int win, QList<Var> types, QStringList args)
{
    auto get_min_arg_count = [types]() {
        int i;
        for(i = 0; i < types.size() && !types[i].optional; i++);
        return i;
    };

    if (args.size() < get_min_arg_count()) {
        addErr(cmd + QString::asprintf(": incorrect number of arguments %lld, minimal expected %d", args.size(), get_min_arg_count()));
        QString g("  got:");
        addErr(g + args.join(" "));
        addNOk(cmd + QString::asprintf(":%d", win));
        return false;
    }
    int i;
    int N = args.size();
    for(i = 0; i < types.size(); i++) {
        VarType t = types[i].type;
        if (t == integer) {
            bool ok = true;
            *(types[i].i) = (i < N) ? args[i].toInt(&ok) : types[i].d_i;
            if (!ok) {
                addErr(cmd + ": " + types[i].name + ": expected integer, got " + args[i]);
                addNOk(cmd + QString::asprintf(":%d", win));
                return false;
            }
        } else if (t == string) {
            *(types[i].s) = (i < N) ? args[i] : types[i].d_s;
        } else if (t == url) {
            QUrl u = (i < N) ? QUrl(args[i]) : types[i].d_u;
            if (!u.isValid()) {
                addErr(cmd + ": " + types[i].name + ": url expected, got " + args[i]);
                addNOk(cmd + QString::asprintf(":%d", win));
                return false;
            }
            *(types[i].u) = u;
        } else if (t == json_string) {
            QString s = (i < N) ? args[i] : types[i].d_s;
            QJsonParseError err;
            QJsonDocument::fromJson(s.toUtf8(), &err);
            if (err.error != QJsonParseError::NoError) {
                addErr(cmd + ": " + types[i].name + ": Json Parse Error '" + err.errorString() + "'");
                addNOk(cmd + QString::asprintf(":%d", win));
                return false;
            }
            *(types[i].s) = s;
        }
    }

    return true;
}

void WebWireHandler::log(FILE *fh, FILE *log_fh, const char *format, const char *msg)
{
    int i, N, nl;
    for(i = 0, N = strlen(msg), nl = 1; i < N; i++) {
        if (msg[i] == '\n') nl++;
    }
    if (log_fh != nullptr) {
        fprintf(log_fh, format, nl, msg);
        fflush(log_fh);
    }

    if (fh != nullptr) {
        fprintf(fh, format, nl, msg);
        fflush(fh);
    }
}

void WebWireHandler::error(const QString &msg)
{
    log(stderr, _log_fh, "ERR(%d):%s\n", msg.toUtf8().constData());
}

void WebWireHandler::ok(const QString &msg)
{
    if (msg.startsWith("OK:")) {
        log(stdout, _log_fh, "OK(%d):%s\n", msg.mid(3).toUtf8().constData());
    } else if (msg.startsWith("NOK:")) {
        log(stdout, _log_fh, "NOK(%d):%s\n", msg.mid(4).toUtf8().constData());
    } else {
        log(stdout, _log_fh, "OK:%s\n",msg.toUtf8().constData());
    }
}

void WebWireHandler::evt(const QString &msg)
{
    log(stderr, _log_fh, "EVENT(%d):%s\n", msg.toUtf8().constData());
}

void WebWireHandler::message(const QString &msg)
{
    log(stderr, _log_fh, "MSG(%d):%s\n", msg.toUtf8().constData());
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

void WebWireHandler::setStylesheet(const QString &css)
{
    setDefaultCss(css);
    QList<int> wins = _windows.keys();
    int i;
    for(i = 0; i < wins.size(); i++) {
        int win = wins[i];
        WinInfo_t *i = getWinInfo(win);
        i->profile->set_css(this, win, -1, css);
    }
}

QString WebWireHandler::getStylesheet()
{
    return defaultCss();
}


static QString pid()
{
#ifdef Q_OS_WIN
    int _p = _getpid();
#else
    int _p = getpid();
#endif
    unsigned long long p = _p;
    return QString::asprintf("%llu", p);
}

WebWireHandler::WebWireHandler(QApplication *app, int argc, char *argv[]) : QObject()
{
    _app = app;
    _listener = new ConsoleListener(this);
    connect(_listener, &ConsoleListener::newLine, this, &WebWireHandler::processInput);
    connect(_listener, &ConsoleListener::stopped, this, &WebWireHandler::inputStopped);
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
#ifdef Q_OS_WIN
    _log_fh = _fsopen(log_file.toUtf8().data(), "wt", _SH_DENYNO);
#else
    _log_fh = fopen(log_file.toUtf8().data(), "wt");
#endif

    msg(QString("Web Wire - v") + WEB_WIRE_VERSION + " - " + WEB_WIRE_COPYRIGHT + " - " + WEB_WIRE_LICENSE +
        QString::asprintf(" - Qt libraries %d.%d.%d", QT_VERSION_MAJOR, QT_VERSION_MINOR, QT_VERSION_PATCH)
        );
    msg("Web Wire file store: " + _my_dir.absolutePath());
    msg("Web Wire log file: " + log_file);

    if (!listens) {
        evt("no-http-service");
    } else {
        msg(QString::asprintf("Web Wire Http Server on http://127.0.0.1:%d", _port));
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
        delete w;

        WebWireProfile *p = i->profile;
        if (p != nullptr) {
            if (p->usage() == 1) {  // usage will drop to 0 when i is deleted.
                _profiles.remove(p->profileName());
            }
        }
        delete i;   // delete i after w, because otherwise the WebEnginProfile gets deleted before the WebEnginePage.

        evt(QString::asprintf("closed:%d", win));
    }
}

void WebWireHandler::requestClose(int win)
{
    evt(QString::asprintf("request-close:%d", win));
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
            evt(QString::asprintf("moved:%d:%d %d", win, i->pos.x(), i->pos.y()));
            i->pos_set = false;
        }
        if (i->size_set) {
            evt(QString::asprintf("resized:%d:%d %d", win, i->size.width(), i->size.height()));
            i->size_set = false;
        }
    }
}

int WebWireHandler::newWindow(const QString &profile, int parent_win_id)
{
    ++_window_nr;

    QTimer *t = new QTimer();
    _timers[_window_nr] = t;
    t->setProperty("win", _window_nr);
    t->setSingleShot(true);
    connect(t, &QTimer::timeout, this, &WebWireHandler::handleTimer);

    WinInfo_t *i = new WinInfo_t();
    _infos[_window_nr] = i;

    i->app_name = profile;

    static QRegularExpression re_ws("\\s+");
    QString app_internal_profile = profile.trimmed().replace(re_ws, "_");
    i->base_url = QString::asprintf("http://127.0.0.1:%d/", _port) + app_internal_profile + "/";

    if (_profiles.contains(app_internal_profile)) {
        WebWireProfile *p = _profiles[app_internal_profile];
        p->incUsage();
        i->profile = p;
    } else {
        WebWireProfile *p = new WebWireProfile(app_internal_profile, defaultCss(), this);
        p->incUsage();
        i->profile = p;
    }

    WebWireWindow *parent = nullptr;
    if (parent_win_id > 0) {
        parent = getWindow(parent_win_id);
    }

    WebWireWindow *w = new WebWireWindow(this, _window_nr, profile, parent);
    _windows[_window_nr] = w;

    _server->route("/" + app_internal_profile + "/<arg>", [this](const QString &file) {
        this->message("Serving: " + file);
        QFile f(file);
        if (f.exists()) {
            return QHttpServerResponse::fromFile(file);
        } else {
            QHttpServerResponse resp(QHttpServerResponse::StatusCode::NotFound);
            return resp;
        }
    });

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
    if (w != nullptr) {
        w->setClosing(true);
        w->close();
    }
    return w != nullptr;
}

void WebWireHandler::debugWin(int win)
{
    WebWireWindow *w = getWindow(win);
    WebWireView *v = w->view();
    WebWirePage *p = v->page();

    DevToolsWindow *devtools_win = new DevToolsWindow();
    p->setDevToolsPage(devtools_win->page());
    devtools_win->show();
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

#define m_chk(cond, msg) \
   if (!(cond)) { \
      addErr(QString::asprintf("set-menu:%d:", win) + msg); \
      return false; \
   }

bool WebWireHandler::makeSubMenu(int win, QMenu *m, QJsonArray &a)
{
    int i;
    for(i = 0; i < a.count(); i++) {
        m_chk(a[i].isArray(), "Expected menu entry or (sub) menu");

        QJsonArray entry = a[i].toArray();
        m_chk(entry.count() == 2, "Entry consists of a name + action-id or submenu");

        QString name = entry[0].toString().trimmed();
        m_chk(name != "", "Empty menu entry name");

        if (entry[1].isArray()) {
            QMenu *submenu = m->addMenu(name);
            QJsonArray sm_a = entry[1].toArray();
            if (!makeSubMenu(win, submenu, sm_a)) {
                return false;
            }
        } else {
            QString id = entry[1].toString();
            QAction *a = m->addAction(name);
            a->setData(QString::asprintf("menu-item-choosen:%d:", win) + id);
            connect(a, &QAction::triggered, this, [this](bool checked) {
                QAction *a = qobject_cast<QAction *>(sender());
                QString event = a->data().toString();
                evt(event);
            });
        }
    }

    return true;
}

bool WebWireHandler::makeMenuBar(int win, QMenuBar *b, QJsonArray &a)
{
    int i;
    for(i = 0; i < a.count(); i++) {
        m_chk(a[i].isArray(), "Expected (sub)menu");

        QJsonArray entry = a[i].toArray();
        m_chk(entry.count() == 2, "Entry consists of name + submenu");

        QString name = entry[0].toString().trimmed();
        m_chk(name != "", "Empty menu-bar name for entry");
        m_chk(entry[1].isArray(), "A top level menu entry must contain a name and a list of menu items");

        QMenu *m = b->addMenu(name);
        QJsonArray m_a = entry[1].toArray();
        if (!makeSubMenu(win, m, m_a)) {
            return false;
        }
    }

    return true;
}

#undef m_chk

bool WebWireHandler::setMenu(int win, const QString &menu)
{
    QJsonDocument doc = QJsonDocument::fromJson(menu.toUtf8());
    WebWireWindow *w = getWindow(win);
    if (w == nullptr) {
        return false;
    } else {
        QMenuBar *bar = new QMenuBar(w);
        QJsonArray a = doc.array();
        if (makeMenuBar(win, bar, a)) {
            w->setMenuBar(bar);
            return true;
        } else {
            bar->deleteLater();
            return false;
        }
    }
}

void WebWireHandler::setShowState(int win, const QString &state)
{
    WebWireWindow *w = getWindow(win);
    w->setShowState(state);
}

QString WebWireHandler::showState(int win)
{
    WebWireWindow *w = getWindow(win);
    return w->showState();
}

QString WebWireHandler::fileOpen(int win, const QString &title, const QString &dir, const QString &file_types, bool &ok)
{
    WebWireWindow *w = getWindow(win);
    QString fn = QFileDialog::getOpenFileName(w, title, dir, file_types);
    if (fn.isNull()) {
        ok = false;
    } else {
        ok = true;
    }
    return fn;
}

QString WebWireHandler::fileSave(int win, const QString &title, const QString &dir, const QString &file_types, bool overwrite, bool &ok)
{
    WebWireWindow *w = getWindow(win);

    QFileDialog::Options o = QFlags<QFileDialog::Option>();
    if (overwrite) { o.setFlag(QFileDialog::DontConfirmOverwrite, true); }

    QString fn = QFileDialog::getSaveFileName(w, title, dir, file_types, nullptr, o);
    if (fn.isNull()) {
        ok = false;
    } else {
        ok = true;
    }
    return fn;
}

QString WebWireHandler::chooseDir(int win, const QString &title, const QString &dir, bool &ok)
{
    WebWireWindow *w = getWindow(win);
    QString fn = QFileDialog::getExistingDirectory(w, title, dir);
    if (fn.isNull()) {
        ok = false;
    } else {
        ok = true;
    }
    return fn;
}


int WebWireHandler::execJs(int win, int handle, const QString &code, bool is_void, QString tag)
{
    WebWireWindow *w = getWindow(win);
    if (w != nullptr) {
        WebWireView *v = w->view();
        WebWirePage *p = v->page();
        ExecJs *e = new ExecJs(this, win, handle, tag, is_void);
        e->run(p, code);
        return e->handle();
    } else {
        return 0;
    }
}

WebWireView *WebWireHandler::getView(int win)
{
    WebWireWindow *w = getWindow(win);
    if (w != nullptr) { return w->view(); }
    else { return nullptr; }
}

void WebWireHandler::start()
{
    _listener->start();
    msg("protocol-version: " WEB_WIRE_PROTOCOL_VERSION);
}

WinInfo_t::WinInfo_t() {
    size_set = false;
    pos_set = false;
    profile = nullptr;
}

WinInfo_t::~WinInfo_t() {
    if (profile != nullptr) {
        profile->decUsage();
    }
}
