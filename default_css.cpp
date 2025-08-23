#include "default_css.h"

static QString css = "body {"
                     "  font-family: Arial, Helvetica, Sans;"
                     "  font-size: 11pt;"
                     "}"
                     ""
                     "button {"
                     "  background: #ed8f24;"
                     "  padding-top: 1em;"
                     "  padding-bottom: 1em;"
                     "  padding-left: 0.5em;"
                     "  padding-right: 0.5em;"
                     "  min-width: 50px;"
                     "  color: white;"
                     "  border: 1px solid #8a5719;"
                     "  margin: 2px;"
                     "}"
                     "button:hover {"
                     "  background: #b06919;"
                     "}"
                     "button:active {"
                     "  border-width: 3px;"
                     "  margin: 0px;"
                     "}"
                     ".disabled {"
                     "  pointer-events: none;"
                     "  color: #707070;"
                     "}"
                     "button.disabled {"
                     "  background: #909090;"
                     "  border: 1px solid #606060;"
                     "  pointer-events: none;"
                     "}"
                     "";

QString defaultCss()
{
    return css;
}

void setDefaultCss(const QString &new_css)
{
    css = new_css;
}
