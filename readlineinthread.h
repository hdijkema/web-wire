#ifndef READLINEINTHREAD_H
#define READLINEINTHREAD_H

#include <QObject>
#include <QThread>

class ReadLineInThread : public QThread
{
    Q_OBJECT
private:
    char    *_buffer;
    int      _buffer_len;
    bool     _go_on;

public:
    explicit ReadLineInThread(QObject *parent = nullptr);
    ~ReadLineInThread();

public:
    void quit();

signals:
    void haveALine(QString l);
    void haveEof();
    void haveError(int error_number);

    // QThread interface
protected:
    void run();
};

#endif // READLINEINTHREAD_H
