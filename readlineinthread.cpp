#include "readlineinthread.h"

ReadLineInThread::ReadLineInThread(QObject *parent)
    : QObject{parent}
{
    _buffer_len = 10 * 1024 * 1024;      // max bufferlen, i.e. max line len = 10MB
    _buffer = static_cast<char *>(malloc(_buffer_len + 1));
    _thread = QThread::create([this]() {
        //int i = 0;
        while(1) {
            //printf("[%4d]%%", ++i);
            char *s = fgets(_buffer, _buffer_len, stdin);
            QString l(s);
            emit haveALine(l);
        }
    });
    _thread->start();
}

ReadLineInThread::~ReadLineInThread()
{
    quit();
    free(_buffer);
}

void ReadLineInThread::quit()
{
    if (_thread != nullptr) {
        _thread->terminate();
        _thread->wait();
        _thread = nullptr;
    }
}
