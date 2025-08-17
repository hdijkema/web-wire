#ifndef WEBWIREPROFILE_H
#define WEBWIREPROFILE_H

#include <QObject>
#include <QWebEngineProfile>
#include <QWidget>

class WebWireHandler;

#define WEB_WIRE_PROFILE_WORLD_ID 425542

class WebWireProfile : public QWebEngineProfile
{
    Q_OBJECT
private:
    QString _set_html_name;
    QString _get_html_name;
    QString _set_attr_name;
    QString _get_attr_name;
    QString _set_style_name;

public:
    explicit WebWireProfile(QObject *parent = nullptr);

public:
    void set_html(WebWireHandler *h, int win, const QString &element_id, const QString &html);
    void get_html(WebWireHandler *h, int win, const QString &element_id);
    void set_attr(WebWireHandler *h, int win, const QString &element_id, const QString &attr, const QString &val);
    void get_attr(WebWireHandler *h, int win, const QString &element_id, const QString &attr);
    void set_style(WebWireHandler *h, int win, const QString &element_id, const QString &style);
};

#endif // WEBWIREPROFILE_H
