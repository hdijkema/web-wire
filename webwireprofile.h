#ifndef WEBWIREPROFILE_H
#define WEBWIREPROFILE_H

#include <QObject>
#include <QWebEngineProfile>
#include <QWebEngineScript>
#include <QWidget>

class WebWireHandler;
class QWebEnginePage;

#define WEB_WIRE_PROFILE_WORLD_ID 425542

class WebWireProfile : public QWebEngineProfile
{
    Q_OBJECT
private:
    QString _profile_name;

private:
    QString _set_html_name;
    QString _get_html_name;
    QString _set_attr_name;
    QString _get_attr_name;
    QString _get_attrs_name;
    QString _del_attr_name;
    QString _add_style_name;
    QString _set_style_name;
    QString _get_style_name;
    QString _set_css_name;
    QString _get_elements_name;

private:
    int _world_id;
    QString _css;
    QWebEngineScript _css_script;

private:
    QString esc(const QString &in);
    int exec(WebWireHandler *h, int win, int handle, const QString &name, const QString &js, bool is_void = false);
    
public:
    explicit WebWireProfile(const QString &name, const QString &default_css, QObject *parent = nullptr);

public:
    void decUsage();
    void incUsage();
    int  usage();
    QString profileName();

public:
    int set_html(WebWireHandler *h, int win, int handle, const QString &element_id, const QString &html, bool fetch);
    int get_html(WebWireHandler *h, int win, int handle, const QString &element_id);

    int set_attr(WebWireHandler *h, int win, int handle, const QString &element_id, const QString &attr, const QString &val);
    int get_attr(WebWireHandler *h, int win, int handle, const QString &element_id, const QString &attr);
    int get_attrs(WebWireHandler *h, int win, int handle, const QString &element_id);
    int del_attr(WebWireHandler *h, int win, int handle, const QString &element_id, const QString &attr);

    int add_style(WebWireHandler *h, int win, int handle, const QString &element_id, const QString &style);
    int set_style(WebWireHandler *h, int win, int handle, const QString &element_id, const QString &style);
    int get_style(WebWireHandler *h, int win, int handle, const QString &element_id);

    int set_css(WebWireHandler *h, int win, int handle, const QString &css);

    int get_elements(WebWireHandler *h, int win, int handle, const QString &selector);
};

#endif // WEBWIREPROFILE_H
