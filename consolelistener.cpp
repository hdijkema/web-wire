#include "consolelistener.h"

ConsoleListener::ConsoleListener(QObject *parent)
    : QObject(parent)
{
    _rl = new ReadLineInThread(this);
    connect(_rl, &ReadLineInThread::haveALine, this, &ConsoleListener::processLine, Qt::QueuedConnection);
}

ConsoleListener::~ConsoleListener()
{
}

void ConsoleListener::start()
{
}

void ConsoleListener::close()
{
    _rl->quit();
}

void ConsoleListener::processLine(QString l)
{
    emit newLine(l);
}



