#ifndef WEBWIREPROFILE_H
#define WEBWIREPROFILE_H

#include <QObject>
#include <QWebEngineProfile>
#include <QWidget>

class WebWireHandler;
class QWebEnginePage;

#define WEB_WIRE_PROFILE_WORLD_ID 425542

class WebWireProfile : public QWebEngineProfile
{
    Q_OBJECT
private:
    QString _set_html_name;
    QString _get_html_name;
    QString _set_attr_name;
    QString _get_attr_name;
    QString _del_attr_name;
    QString _set_style_name;
    QString _get_style_name;

private:
    int _world_id;

private:
    QString esc(const QString &in);
    int exec(WebWireHandler *h, int win, const QString &name, const QString &js);

public:
    explicit WebWireProfile(const QString &name, QObject *parent = nullptr);

public:
    int set_html(WebWireHandler *h, int win, const QString &element_id, const QString &html, bool fetch);
    int get_html(WebWireHandler *h, int win, const QString &element_id);

    int set_attr(WebWireHandler *h, int win, const QString &element_id, const QString &attr, const QString &val);
    int get_attr(WebWireHandler *h, int win, const QString &element_id, const QString &attr);
    int del_attr(WebWireHandler *h, int win, const QString &element_id, const QString &attr);

    int set_style(WebWireHandler *h, int win, const QString &element_id, const QString &style);
    int get_style(WebWireHandler *h, int win, const QString &element_id);
};

#endif // WEBWIREPROFILE_H
