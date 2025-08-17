#include "webwireprofile.h"

#include <QWebEngineScript>
#include <QWebEngineScriptCollection>
#include "webwirepage.h"
#include "webwireview.h"
#include "webwirehandler.h"
#include "execjs.h"

QString WebWireProfile::esc(const QString &in)
{
    QString s = in;
    return s.replace("'", "\\'");
}

WebWireProfile::WebWireProfile(const QString &name, QObject *parent)
    : QWebEngineProfile(name, parent)
{

    QWebEngineScript dom_access;

    int world_id = WEB_WIRE_PROFILE_WORLD_ID;

    dom_access.setWorldId(world_id);
    dom_access.setName("dom_access");
    dom_access.setRunsOnSubFrames(true);
    dom_access.setSourceCode(
        QString::asprintf(
            "window.dom_set_html_%d = function(id, data, do_fetch) {"
            "  let el = document.getElementById(id);"
            "  if (el !== undefined) {"
            "     if (do_fetch) { "
            "        fetch(data).then(x => x.text()).then(y => el.innerHTML = y);"
            "     } else {"
            "       el.innerHTML = data;"
            "     }"
            "     return true;"
            "  } else {"
            "    console.error('element with id ' + id + ' not found');"
            "    return false;"
            "  }"
            "};"
            "window.dom_get_html_%d = function(id) {"
            "  let el = document.getElementById(id);"
            "  if (el !== undefined) {"
            "     return el.innerHTML;"
            "  } else {"
            "     return '';"
            "  }"
            "};"
            "window.dom_set_attr_%d = function(id, attr, val) {"
            "  let el = document.getElementById(id);"
            "  if (el === undefined) {"
            "     return false;"
            "  } else {"
            "     el.setAttribute(attr, val);"
            "     return true;"
            "  }"
            "};"
            "window.dom_get_attr_%d = function(id, attr) {"
            "  let el = document.getElementById(id);"
            "  if (el === undefined) {"
            "    return '';"
            "  } else {"
            "    let v = el.getAttribute(attr);"
            "    if (v === null) { return ''; }"
            "    else { return v; }"
            "  }"
            "};"
            "window.dom_del_attr_%d = function(id, attr) {"
            "  let el = document.getElementById(id);"
            "  if (el === undefined) {"
            "    return false;"
            "  } else {"
            "    el.removeAttribute(attr);"
            "    return true;"
            "  }"
            "};"
            "window.dom_set_style_%d = function(id, style) {"
            "  window.dom_set_attr_%d(id, 'style', style);"
            "};"
            "window.dom_get_style_%d = function(id) {"
            "  return window.dom_get_attr_%d(id, 'style');"
            "};"
            "console.log('Set all window.<functions');",
            world_id,   // set_html
            world_id,   // get_html
            world_id,   // set_attr
            world_id,   // get_attr
            world_id,   // del_attr
            world_id, world_id,   // set_style
            world_id, world_id    // get_style
            )
        );

    QWebEngineScriptCollection *col = scripts();
    col->clear();
    col->insert(dom_access);

    _set_html_name = QString::asprintf("window.dom_set_html_%d", world_id);
    _get_html_name = QString::asprintf("window.dom_get_html_%d", world_id);
    _set_attr_name = QString::asprintf("window.dom_set_attr_%d", world_id);
    _get_attr_name = QString::asprintf("window.dom_get_attr_%d", world_id);
    _get_attr_name = QString::asprintf("window.dom_del_attr_%d", world_id);
    _set_style_name = QString::asprintf("window.dom_set_style_%d", world_id);
    _get_style_name = QString::asprintf("window.dom_get_style_%d", world_id);

    _world_id = dom_access.worldId();
}

int WebWireProfile::exec(WebWireHandler *h, int win, const QString &name, const QString &js)
{
    WebWireView *v = h->getView(win);
    WebWirePage *p = v->page();

    ExecJs *e = new ExecJs(h, win, name);
    int handle = e->handle();
    e->run(p, _world_id, js);

    return handle;
}

int WebWireProfile::set_html(WebWireHandler *h, int win, const QString &element_id, const QString &data, bool fetch)
{
    QString do_fetch = fetch ? "true" : "false";

    return exec(h, win, "dom-result",
                _set_html_name + "(" + "'" + esc(element_id) + "', " +
                                          "'" + esc(data) + "', " +
                                          do_fetch +
                                    ");"
              );
}

int WebWireProfile::get_html(WebWireHandler *h, int win, const QString &element_id)
{
    return exec(h, win, "dom-result",
                _get_html_name + "('" + esc(element_id) + "');"
                );
}

int WebWireProfile::set_attr(WebWireHandler *h, int win, const QString &element_id, const QString &attr, const QString &val)
{
    return exec(h, win, "dom-result",
                _set_attr_name + "(" + "'" + esc(element_id) + "', " +
                                          "'" + esc(attr) + "', " +
                                          "'" + esc(val) + "'" +
                                    ");"
              );
}

int WebWireProfile::get_attr(WebWireHandler *h, int win, const QString &element_id, const QString &attr)
{
    return exec(h, win, "dom-result",
                _get_attr_name + "(" +  "'" + esc(element_id) + "', " +
                                           "'" + esc(attr) + "', " +
                                    ");"
              );
}

int WebWireProfile::del_attr(WebWireHandler *h, int win, const QString &element_id, const QString &attr)
{
    return exec(h, win, "dom-result",
                _del_attr_name + "(" +  "'" + esc(element_id) + "', " +
                              "'" + esc(attr) + "', " +
                              ");"
                     );
}

int WebWireProfile::set_style(WebWireHandler *h, int win, const QString &element_id, const QString &style)
{
    return exec(h, win, "dom-result",
                _set_style_name + "(" +  "'" + esc(element_id) + "', " +
                                            "'" + esc(style) + "'" +
                                     ");"
              );
}

int WebWireProfile::get_style(WebWireHandler *h, int win, const QString &element_id)
{
    return exec(h, win, "dom-result",
                _get_style_name + "('" + esc(element_id) + "');"
                );
}








