#include "webwireprofile.h"

#include <QWebEngineScript>

WebWireProfile::WebWireProfile(QObject *parent)
    : QWebEngineProfile{parent}
{

    QWebEngineScript dom_access;
    dom_access.setWorldId(WEB_WIRE_PROFILE_WORLD_ID);
    dom_access.setName("dom_access");
    dom_access.setRunsOnSubFrames(true);
    dom_access.setSourceCode(
        QString::asprintf(
            "window.dom_set_html_%d = function(id, html) {"
            "",
            dom_access.worldId()
            )
        );


}


