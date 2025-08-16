#ifndef CONSOLELISTENER_H
#define CONSOLELISTENER_H

#include <QObject>
#include "readlineinthread.h"

class ConsoleListener : public QObject
{
    Q_OBJECT
private:
    ReadLineInThread *_rl;
public:
    explicit ConsoleListener(QObject *parent = nullptr);
    ~ConsoleListener();

public:
    void start();
    void close();

signals:
    // connect to "newLine" to receive console input
    void newLine(const QString &strNewLine);

private slots:
    void processLine(QString l);
};

#endif // CONSOLELISTENER_H
