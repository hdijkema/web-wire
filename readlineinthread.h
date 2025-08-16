#ifndef READLINEINTHREAD_H
#define READLINEINTHREAD_H

#include <QObject>
#include <QThread>

class ReadLineInThread : public QObject
{
    Q_OBJECT
private:
    char    *_buffer;
    int      _buffer_len;
    QThread *_thread;

public:
    explicit ReadLineInThread(QObject *parent = nullptr);
    ~ReadLineInThread();

public:
    void quit();

signals:
    void haveALine(QString l);
};

#endif // READLINEINTHREAD_H
