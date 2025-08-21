#ifndef EXECJS_H
#define EXECJS_H

#include <QVariant>

class QWebEnginePage;
class WebWireHandler;

class ExecJs
{
private:
    WebWireHandler *_handler;
    int             _handle;
    int             _win;
    QString         _name;
    bool            _is_void;

public:
    void run(QWebEnginePage *p, const QString &code);
    void run(QWebEnginePage *p, int world_id, const QString &code);

public:
    int handle();

public:
    ExecJs(WebWireHandler *handler, int win, int handle, QString name = "exec-js", bool is_void = false);
};

#endif // EXECJS_H
