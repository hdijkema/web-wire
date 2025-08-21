#include "execjs.h"

#include <QWebEnginePage>
#include "webwirehandler.h"

void ExecJs::run(QWebEnginePage *p, const QString &code) {
    p->runJavaScript(code, [this](const QVariant &v) {
        if (!_is_void) {
            QString result = v.toString().replace("\"", "\\\"");
            _handler->evt(QString::asprintf("js-result:%d:%d:%s:", _win, _handle, _name.toUtf8().data()) +
                          QString("\"") + result + QString("\"")
                          );
        }
        delete this;
    });
}

void ExecJs::run(QWebEnginePage *p, int world_id, const QString &code)
{
    p->runJavaScript(code, world_id, [this](const QVariant &v) {
        if (!_is_void) {
            QString result = v.toString().replace("\"", "\\\"");
            _handler->evt(QString::asprintf("js-result:%d:%d:%s: ", _win, _handle, _name.toUtf8().data()) +
                          QString("\"") + result + QString("\"")
                          );
        }
        delete this;
    });
}

ExecJs::ExecJs(WebWireHandler *handler, int win, int handle, QString name, bool is_void)
{
    _is_void = is_void;

    _handler = handler;
    _handle = handle;
    _win = win;
    _name = name;
}

int ExecJs::handle()
{
    return _handle;
}

