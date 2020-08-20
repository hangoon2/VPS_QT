#ifndef REC_QUEUEHANDLER_H
#define REC_QUEUEHANDLER_H

#include <QThread>

#include <stdio.h>
#include <sys/types.h>

#ifndef IN
#define IN
#endif

typedef void (*PFN_THREAD_PROC)(void* pContext);

#define ON_MESSAGE_POST (threadhelper, signal, slot) threadhelper.connect(this, signal, slot, Qt::AutoConnection);
#define ON_MESSAGE_SEND (threadhelper, signal, slot) threadhelper.connect(this, signal, slot, Qt::BlockingQueueConnection);

class ThreadHelper : public QThread
{
    Q_OBJECT
public:
    explicit ThreadHelper(QObject *parent = 0);

private:
    PFN_THREAD_PROC m_pfnThreadProc;
    void* m_pContext;

    bool m_isRunning;
    pthread_t m_nCallingTID;
    pthread_t m_nThreadTID;

public:
    bool BeginThread(IN PFN_THREAD_PROC pfnThreadProc, IN void* pContext);
    void WaitThread(void);

protected:
    void run();
};

#endif // REC_QUEUEHANDLER_H
