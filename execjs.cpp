#include "execjs.h"

#include <QWebEnginePage>
#include "webwirehandler.h"

static int _code_handle = 0;


void ExecJs::run(QWebEnginePage *p, const QString &code) {
    p->runJavaScript(code, [this](const QVariant &v) {
        QString result = v.toString().replace("\"", "\\\"");
        _handler->evt(QString::asprintf("%s: %d %d ", _name.toUtf8().data(), _win, _handle) +
                      QString("\"") + result + QString("\"")
                      );
        delete this;
    });
}

void ExecJs::run(QWebEnginePage *p, int world_id, const QString &code)
{
    p->runJavaScript(code, world_id, [this](const QVariant &v) {
        QString result = v.toString().replace("\"", "\\\"");
        _handler->evt(QString::asprintf("%s: %d %d ", _name.toUtf8().data(), _win, _handle) +
                      QString("\"") + result + QString("\"")
                      );
        delete this;
    });
}

ExecJs::ExecJs(WebWireHandler *handler, int win, QString name) {

    _code_handle += 1;
    if (_code_handle == 0) { _code_handle += 1; }

    _handler = handler;
    _handle = _code_handle;
    _win = win;
    _name = name;
}

int ExecJs::handle()
{
    return _handle;
}

