#include "consolelistener.h"

ConsoleListener::ConsoleListener(QObject *parent)
    : QObject(parent)
{
    _rl = new ReadLineInThread(this);
    connect(_rl, &ReadLineInThread::haveALine, this, &ConsoleListener::processLine, Qt::QueuedConnection);
    connect(_rl, &ReadLineInThread::haveEof, this, &ConsoleListener::haveEof, Qt::QueuedConnection);
    connect(_rl, &ReadLineInThread::haveError, this, &ConsoleListener::haveError, Qt::QueuedConnection);
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

void ConsoleListener::haveEof()
{
    emit stopped();
}

void ConsoleListener::haveError(int err)
{
    emit stopped();
}



