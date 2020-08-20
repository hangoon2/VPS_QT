#include "ThreadHelper.h"

#include <QtDebug>

ThreadHelper::ThreadHelper(QObject *parent) :
    QThread(parent)
{
    m_isRunning = false;

    m_nCallingTID = pthread_self();

    m_nThreadTID = 0;

    m_pfnThreadProc = NULL;
    m_pContext = NULL;
}

void ThreadHelper::run()
{
    m_isRunning = true;

    if(m_pfnThreadProc == NULL)
    {
        goto FINAL;
    }

    m_nThreadTID = pthread_self();
    // Thread Procedure 호출
    (*m_pfnThreadProc)(m_pContext);

FINAL:
    m_isRunning = false;
}

bool ThreadHelper::BeginThread(PFN_THREAD_PROC pfnThreadProc, void *pContext)
{
    bool bRtnValue = false;

    if(pfnThreadProc == NULL || pContext == NULL)
    {
        bRtnValue = false;
        goto FINAL;
    }

    if(pthread_equal(m_nCallingTID, pthread_self()) == 0)
    {
        // instance를 생성한 caller thread와 다른 thread에서 호출을 방지한다.
        bRtnValue = false;
        qDebug() << "Invalid caller thread";
        goto FINAL;
    }

    if(m_isRunning)
    {
        bRtnValue = false;
        goto FINAL;
    }

    m_pfnThreadProc = pfnThreadProc;
    m_pContext = pContext;

    start();

FINAL:
    return bRtnValue;
}

void ThreadHelper::WaitThread(void)
{
    if(pthread_equal(m_nCallingTID, pthread_self()) == 0)
    {
        // instance를 생성한 caller thread와 다른 thread에서 호출을 방지한다.
        qDebug() << "Invalid caller thread";
        goto FINAL;
    }

    // 종료시까지 대기한다.
    qDebug() << "Wait for thread : " << ((unsigned long int)m_nThreadTID);
    //wait(ULONG_MAX);
    wait(1);
    qDebug() << "Finished to wait for thread : " << ((unsigned long int)m_nThreadTID);

FINAL:
    return;
}
