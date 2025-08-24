#include "webwireprofile.h"

#include <QWebEngineScript>
#include <QWebEngineScriptCollection>
#include "webwirepage.h"
#include "webwireview.h"
#include "webwirehandler.h"
#include "execjs.h"

static QString cssCode(const QString &css)
{
   return QString("{") +
   "let styles ='" + css + "';" +
   "let stylesheet = document.createElement('style');"
   "stylesheet.setAttribute('id', 'web-wire-css');"
   "stylesheet.textContent = styles;"
   "document.head.appendChild(stylesheet);"
   "}";
}

QString WebWireProfile::esc(const QString &in)
{
    QString s = in;
    return s.replace("'", "\\'");
}

WebWireProfile::WebWireProfile(const QString &name, const QString &default_css, QObject *parent)
    : QWebEngineProfile(name, parent)
{
    _profile_name = name;
    _css = default_css;

    QWebEngineScript dom_access;

    int world_id = WEB_WIRE_PROFILE_WORLD_ID;

    dom_access.setWorldId(world_id);
    dom_access.setName("dom_access");
    dom_access.setRunsOnSubFrames(true);
    dom_access.setSourceCode(
        QString::asprintf(
            "window.dom_set_html_%d = function(id, data, do_fetch) {"
            "  let el = document.getElementById(id);"
            "  if (el !== undefined && el !== null) {"
            "     if (do_fetch) { "
            "        fetch(data).then(x => x.text()).then(y => el.innerHTML = y);"
            "     } else {"
            "       el.innerHTML = data;"
            "     }"
            "     return 'bool:true';"
            "  } else {"
            "    console.error('element with id ' + id + ' not found');"
            "    return 'bool:false';"
            "  }"
            "};"
            "window.dom_get_html_%d = function(id) {"
            "  let el = document.getElementById(id);"
            "  if (el !== undefined && el !== null) {"
            "     return el.innerHTML;"
            "  } else {"
            "     return '';"
            "  }"
            "};"
            "window.dom_set_attr_%d = function(id, attr, val) {"
            "  let el = document.getElementById(id);"
            "  if (el === undefined || el === null) {"
            "     return 'bool:false';"
            "  } else {"
            "     el.setAttribute(attr, val);"
            "     return 'bool:true';"
            "  }"
            "};"
            "window.dom_get_attr_%d = function(id, attr) {"
            "  let el = document.getElementById(id);"
            "  if (el === undefined || el === null) {"
            "    return '';"
            "  } else {"
            "    let v = el.getAttribute(attr);"
            "    if (v === null) { return ''; }"
            "    else { return v; }"
            "  }"
            "};"
            "window.dom_del_attr_%d = function(id, attr) {"
            "  let el = document.getElementById(id);"
            "  if (el === undefined || el === null) {"
            "    return 'bool:false';"
            "  } else {"
            "    el.removeAttribute(attr);"
            "    return 'bool:true';"
            "  }"
            "};"
            "window.split_style_%d = function(st) {"
            "  if (st === null) { st = ''; }"
            "  let parts = st.split(';');"
            "  let n_parts = [];"
            "  parts.forEach(function(part) {"
            "     if (part != '') {"
            "        let kv = part.split(':');"
            "        let key = kv[0].trim();"
            "        let val = (kv.length == 2) ? kv[1] : '';"
            "        n_parts.push({ key: key, val: val});"
            "     }"
            "  });"
            "  return n_parts;"
            "};"
            "window.dom_add_style_%d = function(id, style) {"
            "  let style_parts = split_style_%d(style);"
            "  let st = window.dom_get_attr_%d(id, 'style');"
            "  if (st === null) { st = ''; }"
            "  st_parts = window.split_style_%d(st);"
            "  style_parts.forEach(function(style_part) {"
            "     let sp_key = style_part.key;"
            "     st_parts = st_parts.filter((e) => e.key != sp_key);"
            "     st_parts.push(style_part);"
            "  });"
            "  n_style = '';"
            "  st_parts.forEach(function(p) {"
            "    n_style += p.key + ':' + p.val + ';';"
            "  });"
            "  window.dom_set_attr_%d(id, 'style', n_style);"
            "};"
            "window.dom_set_style_%d = function(id, style) {"
            "  window.dom_set_attr_%d(id, 'style', style);"
            "};"
            "window.dom_get_style_%d = function(id) {"
            "  return window.dom_get_attr_%d(id, 'style');"
            "};"
            "window.dom_set_css_%d = function(css) {"
            "  let stylesheet = document.getElementById('web-wire-css');"
            "  if (stylesheet) {"
            "     stylesheet.textContent = css;"
            "     return 'bool:true';"
            "  }"
            "  return 'bool:false';"
            "};"
            "window.dom_get_attrs_%d = function(id) {"
            "  let el = document.getElementById(id);"
            "  if (el === undefined || el === null) {"
            "     return 'json:[]';"
            "  } else {"
            "    let res = [];"
            "    let attr_names = el.getAttributeNames();"
            "    for(const name of attr_names) {"
            "       res.push([name, el.getAttribute(name)]);"
            "    }"
            "    let ret = 'json:' + JSON.stringify(res);"
            "    return ret;"
            "  }"
            "};"
            "window.dom_get_elements_%d = function(selector) {"
            "  let nodelist = document.querySelectorAll(selector);"
            "  if (nodelist === undefined || nodelist === null) {"
            "     return 'json:[]';"
            "  } else {"
            "    let els = [];"
            "    nodelist.forEach(function(el) { "
            "      let attr_names = el.getAttributeNames();"
            "      let attrs = [];"
            "      for(const name of attr_names) {"
            "         attrs.push([name, el.getAttribute(name)]);"
            "      }"
            "      els.push([el.nodeName, attrs]);"
            "    });"
            "    return 'json:' + JSON.stringify(els);"
            "  }"
            "};",
            world_id,   // set_html
            world_id,   // get_html
            world_id,   // set_attr
            world_id,   // get_attr
            world_id,   // del_attr
            world_id,   // split_style
            world_id, world_id, world_id, world_id, world_id,   // add_style
            world_id, world_id,                                 // set_style
            world_id, world_id,                                 // get_style
            world_id,                                            // set-css
            world_id,    // get-attrs
            world_id     // get-elements
            )
        );

    QWebEngineScript eventing;
    eventing.setWorldId(0);
    eventing.setName("eventing");
    eventing.setRunsOnSubFrames(true);
    eventing.setSourceCode(
        QString() +
        "window._web_wire_evt_queue = [];"
        "window._web_wire_put_evt = function(evt) { _web_wire_evt_queue.push(evt); };"
        "window._web_wire_event_info = function(e, id, evt) {"
        "  let obj = {};"
        "  if (e == 'input') {"
        "     obj['data'] = evt.data;"
        "     obj['dataTransfer'] = evt.dataTransfer;"
        "     obj['inputType'] = evt.inputType;"
        "     obj['isComposing'] = evt.isComposing;"
        "     obj['value'] = document.getElementById(id).value;"
        "  } else if (e == 'change') {"
        "     obj['value'] = document.getElementById(id).value;"
        "  } else if (e == 'mousemove' || e == 'mouseover' || e == 'mouseenter' || "
                    "e == 'mouseleave' || e == 'click' || e == 'dblclick' || "
                    "e == 'mousedown' || e == 'mouseup' ) {"
        "     obj['altKey'] = evt.altKey;"
        "     obj['buttons'] = evt.buttons;"
        "     obj['clientX'] = evt.clientX;"
        "     obj['clientY'] = evt.clientY;"
        "     obj['ctrlKey'] = evt.ctrlKey;"
        "     obj['metaKey'] = evt.metaKey;"
        "     obj['movementX'] = evt.movementX;"
        "     obj['movementY'] = evt.movementY;"
        "     obj['screenX'] = evt.screenX;"
        "     obj['screenY'] = evt.screenY;"
        "     obj['shiftKey'] = evt.shiftKey;"
        "     obj['x'] = evt.x;"
        "     obj['y'] = evt.y;"
        "  } else if (e == 'keydown' || e == 'keyup' || e == 'keypress') {"
        "     obj['key'] = evt.key;"
        "     obj['code'] = evt.code;"
        "     obj['altKey'] = evt.altKey;"
        "     obj['ctrlKey'] = evt.ctrlKey;"
        "     obj['metaKey'] = evt.metaKey;"
        "     obj['repeat'] = evt.repeat;"
        "     obj['shiftKey'] = evt.shiftKey;"
        "  }"
                    // More events can be added like pointerEvent, clipboardEvent, etc.
        "  return obj;"
        "};"
        "window._web_wire_get_evts = function() {"
        "   let v = _web_wire_evt_queue;"
        "   _web_wire_evt_queue = [];"
        "   return JSON.stringify(v);"      // This needs no extra type info, as it is internally used only
        "};"
        "window._web_wire_bind_evt_ids = function(selector, event_kind) {"
        "   let nodelist = document.querySelectorAll(selector);"
        "   if (nodelist === undefined || nodelist === null) {"
        "      return 'json:[]';"
        "   }"
        "   let ids = [];"
        "   nodelist.forEach(function(el) { "
        "      let el_id = el.getAttribute('id');"
        "      let el_tag = el.nodeName;"
        "      let el_type = el.getAttribute('type');"
        "      if (el_type === null) { el_type = ''; }"
        "      if (el_id !== null) {"
        "        el.addEventListener(event_kind, "
        "          function(e) {"
        "             let obj = {evt: event_kind, id: el_id, js_evt: window._web_wire_event_info(event_kind, el_id, e) };"
        "             window._web_wire_put_evt(obj);"
        "          }"
        "        );"
        "        let info = [ el_id, el_tag, el_type ];"
        "        ids.push(info);"
        "      }"
        "   });"
        "   return 'json:' + JSON.stringify(ids);"
        "};"
    );

    _css_script.setWorldId(0);
    _css_script.setName("css");
    _css_script.setRunsOnSubFrames(true);
    _css_script.setInjectionPoint(QWebEngineScript::DocumentReady);
    _css_script.setSourceCode(cssCode(esc(_css)));

    QWebEngineScriptCollection *col = scripts();
    col->clear();
    col->insert(QList<QWebEngineScript>() << dom_access << eventing << _css_script);

    _set_html_name = QString::asprintf("window.dom_set_html_%d", world_id);
    _get_html_name = QString::asprintf("window.dom_get_html_%d", world_id);
    _set_attr_name = QString::asprintf("window.dom_set_attr_%d", world_id);
    _get_attr_name = QString::asprintf("window.dom_get_attr_%d", world_id);
    _get_attrs_name = QString::asprintf("window.dom_get_attrs_%d", world_id);
    _del_attr_name = QString::asprintf("window.dom_del_attr_%d", world_id);
    _add_style_name = QString::asprintf("window.dom_add_style_%d", world_id);
    _set_style_name = QString::asprintf("window.dom_set_style_%d", world_id);
    _get_style_name = QString::asprintf("window.dom_get_style_%d", world_id);
    _get_elements_name = QString::asprintf("window.dom_get_elements_%d", world_id);
    _set_css_name = QString::asprintf("window.dom_set_css_%d", world_id);

    _world_id = dom_access.worldId();

    fprintf(stderr, "world-id of domaccess: %d\n", dom_access.worldId());
    fprintf(stderr, "world-id of eventing: %d\n", eventing.worldId());fflush(stderr);

    int usage = 0;
    setProperty("__win_wire_usage_count__", usage);
}

void WebWireProfile::decUsage()
{
    int usage = property("__win_wire_usage_count__").toInt();
    usage -= 1;
    if (usage > 0) {
        setProperty("__win_wire_usage_count__", usage);
    } else {
        this->deleteLater();    // The profile is now not used anymore,
                                // but we want to be sure it is not somewhere in use anymore
                                // So, we delete it later (in the eventloop).
    }
}

void WebWireProfile::incUsage()
{
    int usage = property("__win_wire_usage_count__").toInt();
    usage += 1;
    setProperty("__win_wire_usage_count__", usage);
}

int WebWireProfile::usage()
{
    int usage = property("__win_wire_usage_count__").toInt();
    return usage;
}

QString WebWireProfile::profileName()
{
    return _profile_name;
}

int WebWireProfile::exec(WebWireHandler *h, int win, int handle, const QString &name, const QString &js, bool is_void)
{
    WebWireView *v = h->getView(win);
    WebWirePage *p = v->page();

    h->message(js);

    ExecJs *e = new ExecJs(h, win, handle, name, is_void);
    int r_handle = e->handle();
    e->run(p, _world_id, js);

    return r_handle;
}

int WebWireProfile::set_html(WebWireHandler *h, int win, int handle, const QString &element_id, const QString &data, bool fetch)
{
    QString do_fetch = fetch ? "true" : "false";

    return exec(h, win, handle, "set-inner-html",
                _set_html_name + "(" + "'" + esc(element_id) + "', " +
                                          "'" + esc(data) + "', " +
                                          do_fetch +
                                    ");"
              );
}

int WebWireProfile::get_html(WebWireHandler *h, int win, int handle, const QString &element_id)
{
    return exec(h, win, handle, "get-inner-html",
                _get_html_name + "('" + esc(element_id) + "');"
                );
}

int WebWireProfile::set_attr(WebWireHandler *h, int win, int handle, const QString &element_id, const QString &attr, const QString &val)
{
    return exec(h, win, handle, "set-attr",
                _set_attr_name + "(" + "'" + esc(element_id) + "', " +
                                          "'" + esc(attr) + "', " +
                                          "'" + esc(val) + "'" +
                                    ");"
              );
}

int WebWireProfile::get_attr(WebWireHandler *h, int win, int handle, const QString &element_id, const QString &attr)
{
    return exec(h, win, handle, "get-attr",
                _get_attr_name + "(" +  "'" + esc(element_id) + "', " +
                                           "'" + esc(attr) + "'" +
                                    ");"
              );
}

int WebWireProfile::get_attrs(WebWireHandler *h, int win, int handle, const QString &element_id)
{
    return exec(h, win, handle, "get-attrss",
                _get_attrs_name + "(" +  "'" + esc(element_id) + "'" +
                    ");"
                );
}

int WebWireProfile::get_elements(WebWireHandler *h, int win, int handle, const QString &selector)
{
    return exec(h, win, handle, "get-attrss",
                _get_elements_name + "(" +  "'" + esc(selector) + "'" +
                    ");"
                );
}


int WebWireProfile::del_attr(WebWireHandler *h, int win, int handle, const QString &element_id, const QString &attr)
{
    return exec(h, win, handle, "del-attr",
                _del_attr_name + "(" +  "'" + esc(element_id) + "', " +
                              "'" + esc(attr) + "', " +
                              ");"
                     );
}

int WebWireProfile::add_style(WebWireHandler *h, int win, int handle, const QString &element_id, const QString &style)
{
    return exec(h, win, handle, "add-style",
                _add_style_name + "(" +  "'" + esc(element_id) + "', " +
                    "'" + esc(style) + "'" +
                    ");"
                );
}


int WebWireProfile::set_style(WebWireHandler *h, int win, int handle, const QString &element_id, const QString &style)
{
    return exec(h, win, handle, "set-style",
                _set_style_name + "(" +  "'" + esc(element_id) + "', " +
                                            "'" + esc(style) + "'" +
                                     ");"
              );
}

int WebWireProfile::get_style(WebWireHandler *h, int win, int handle, const QString &element_id)
{
    return exec(h, win, handle, "get-style",
                _get_style_name + "('" + esc(element_id) + "');"
                );
}

int WebWireProfile::set_css(WebWireHandler *h, int win, int handle, const QString &css)
{
    _css  = css;
    _css_script.setSourceCode(cssCode(esc(css)));

    return exec(h, win, handle, "set-css",
                _set_css_name + "('" + esc(css) + "');",
                true // no result expected
                );
}








