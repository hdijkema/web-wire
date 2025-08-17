#include "readlineinthread.h"

#ifdef Q_OS_LINUX
#include <sys/select.h>
#endif

ReadLineInThread::ReadLineInThread(QObject *parent)
    : QThread{parent}
{
    _buffer_len = 10 * 1024 * 1024;      // max bufferlen, i.e. max line len = 10MB
    _buffer = static_cast<char *>(malloc(_buffer_len + 1));
    _go_on = true;
    start();
}

ReadLineInThread::~ReadLineInThread()
{
    quit();
    free(_buffer);
}

void ReadLineInThread::quit()
{
    _go_on = false;
#ifdef Q_OS_WIN
    terminate();
#endif
    wait();
}

void ReadLineInThread::run()
{
    while(_go_on) {
        char *s;
        bool have_line = false;
#ifdef Q_OS_WIN
        s = fgets(_buffer, _buffer_len, stdin);
        have_line = true;
#else
        fd_set          read_set;
        struct timeval  tv;
        FD_ZERO(&read_set);
        FD_SET(fileno(stdin), &read_set);
        tv.tv_sec = 0;
        tv.tv_usec = 100 * 1000;    // 100 ms
        int retval = select(1, &read_set, NULL, NULL, &tv);
        if (retval == -1) {
            perror("select()");
        } else if (retval) {
            s = fgets(_buffer, _buffer_len, stdin);
            have_line = true;
        }
#endif
        if (have_line) {
            QString l(s);
            emit haveALine(l);
        }
    }
}
