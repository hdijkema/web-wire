#include "execjs.h"

#include <QWebEnginePage>
#include "webwirehandler.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>

static QString makeResult(WebWireHandler *h, const QVariant &v)
{
    QString in = v.toString();
    QJsonObject obj;
    QJsonDocument doc;

    h->message(QString("makeResult:") + in);

    if (in.startsWith("json:")) {
        QJsonParseError err;
        QJsonDocument d = QJsonDocument::fromJson(in.mid(5).toUtf8(), &err);

        if (err.error == QJsonParseError::NoError) {
            if (d.isArray()) {
                obj["result"] = d.array();
            } else if (d.isObject()) {
                obj["result"] = d.object();
            } else {
                obj["result"] = false;      // Unknown problem
            }
        } else {
            h->error(QString("exec-js, makeResult from '") + in + "', parse error:" + err.errorString());
            obj["result"] = false;
        }

    } else if (in.startsWith("int:")) {
        obj["result"] = in.mid(4).toInt();
    } else if (in.startsWith("bool:")) {
        QString b = in.mid(5).toLower();
        obj["result"] = (b == "true") ? true : false;
    } else if (in.startsWith("float:")) {
        obj["result"] = in.mid(6).toDouble();
    } else {
        obj["result"] = in;
    }

    doc.setObject(obj);
    return doc.toJson(QJsonDocument::Compact);
}


void ExecJs::run(QWebEnginePage *p, const QString &code) {
    p->runJavaScript(code, [this](const QVariant &v) {
        if (!_is_void) {
            _handler->evt(QString::asprintf("js-result:%d:%d:%s:", _win, _handle, _name.toUtf8().data()) +
                              makeResult(_handler, v)
                          );
        }
        delete this;
    });
}

void ExecJs::run(QWebEnginePage *p, int world_id, const QString &code)
{
    p->runJavaScript(code, world_id, [this](const QVariant &v) {
        if (!_is_void) {
            _handler->evt(QString::asprintf("js-result:%d:%d:%s: ", _win, _handle, _name.toUtf8().data()) +
                              makeResult(_handler, v)
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

