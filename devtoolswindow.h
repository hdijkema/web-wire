#ifndef DEVTOOLSWINDOW_H
#define DEVTOOLSWINDOW_H

#include <QMainWindow>

class QWebEnginePage;
class QWebEngineView;

class DevToolsWindow : public QMainWindow
{
    Q_OBJECT

private:
    QWebEngineView *_devtools_view;

public:
    explicit DevToolsWindow(QWidget *parent = nullptr);

public:
    QWebEnginePage *page();

protected:
    void closeEvent(QCloseEvent *event);
};

#endif // DEVTOOLSWINDOW_H
