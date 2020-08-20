#include "mir_queuehandler.h"

#include <QDebug>
#include <unistd.h>

MIR_QueueHandler::MIR_QueueHandler(QObject *parent) : QObject(parent)
{
//    for(int i = 0; i < MAXCHCNT; i++)
//    {
        m_tempQueueItem = (BYTE*)malloc(MIR_DEFAULT_MEM_POOL_UNIT);

////        connect(&m_mirQueue[i], SIGNAL(mirroringFrame()), this, SLOT(OnMirroringFrame()), Qt::QueuedConnection);
//    }

    m_isThreadRunning = false;
    ResetVars();

    m_pMirroringRoutine = nullptr;

//    moveToThread(&m_hThread);
//    connect(&m_hThread, SIGNAL(started()), this, SLOT(OnStarted()));
//    connect(&m_hThread, SIGNAL(finished()), this, SLOT(OnFinished()));
}

MIR_QueueHandler::~MIR_QueueHandler()
{
//    for(int i = 0; i < MAXCHCNT; i++)
//    {
        if(m_tempQueueItem != nullptr)
        {
            free(m_tempQueueItem);
            m_tempQueueItem = nullptr;
        }
//    }
}

void MIR_QueueHandler::SetIsThreadRunning(bool isThreadRunning)
{
    m_isThreadRunning = isThreadRunning;
}

bool MIR_QueueHandler::IsThreadRunning()
{
    return m_isThreadRunning;
}

void MIR_QueueHandler::ResetVars()
{
    m_isThreadReady = false;
    m_doExitThread = false;
}

bool MIR_QueueHandler::StartThread(PMIRRORING_ROUTINE pMirroringRoutine)
{
    if(IsThreadRunning())
    {
        return false;
    }

    m_pMirroringRoutine = pMirroringRoutine;

    ResetVars();

//    SetIsThreadRunning(true);

//    m_hThread.start();

    m_threadHelper.BeginThread(MIR_QueueHandler::THREAD_PROC, this);

    return true;
}

void MIR_QueueHandler::StopThread()
{
    if(!IsThreadRunning())
    {
        return;
    }

    m_doExitThread = true;
//    m_hThread.terminate();

    SetIsThreadRunning(false);
    m_threadHelper.WaitThread();

//    ResetVars();

//    for(int i = 0; i < MAXCHCNT; i++)
//        m_mirQueue[i].ClearQueue();

//    SetIsThreadRunning(false);
}

bool MIR_QueueHandler::IsOnService()
{
    return IsThreadRunning();   // !m_doExitThread) ? true : false;
}

void MIR_QueueHandler::EnQueue(int nHpNo, BYTE* item)
{
    if(IsOnService())
    {
        m_mirQueue.Enqueue(item);
    }
}

void MIR_QueueHandler::OnMirroring()
{
    if(m_mirQueue.Dequeue(m_tempQueueItem))
    {
        if(m_pMirroringRoutine != nullptr)
        {
            //m_pMirroringRoutine(m_tempQueueItem);
            emit processPacket(m_tempQueueItem);
        }
    }
}

void MIR_QueueHandler::OnStop()
{
    ResetVars();

//    for(int i = 0; i < MAXCHCNT; i++)
    {
        m_mirQueue.ClearQueue();
    }
}

void MIR_QueueHandler::THREAD_PROC(void *pContext)
{
    MIR_QueueHandler* pHandler = (MIR_QueueHandler*)pContext;

    pHandler->SetIsThreadRunning(true);

    while(pHandler->IsThreadRunning())
    {
        pHandler->OnMirroring();

        usleep(1000);
    }

    pHandler->OnStop();

    qDebug() << "MIR_QueueHandler Thread Exit";
}

/* slots */
void MIR_QueueHandler::OnStarted()
{
    qDebug() << "MIR_QueueHandler::Body";

    SetIsThreadRunning(true);

    m_isThreadReady = true;

    while(!m_doExitThread)
    {
//        for(int i = 0; i < MAXCHCNT; i++)
//        {
//            if(m_mirQueue[i].Dequeue(m_tempQueueItem[i]))
//            {
//                if(m_pMirroringRoutine != nullptr)
//                    m_pMirroringRoutine(m_tempQueueItem[i]);
//            }
//        }

        usleep(1);
    }
    qDebug() << "MIR_QueueHandler::Body Thead exit";

    if(!m_hThread.isFinished())
    {
        m_hThread.exit();
    }
}

void MIR_QueueHandler::OnFinished()
{
    qDebug() << "MIR_QueueHandler OnFinished";

//    ResetVars();

//    m_mirQueue.ClearQueue();

//    SetIsThreadRunning(false);
}

void MIR_QueueHandler::OnMirroringFrame()
{
    for(int i = 0; i < MAXCHCNT; i++)
     {
//         if(m_mirQueue[i].Dequeue(m_tempQueueItem[i]))
//         {
//             if(m_pMirroringRoutine != nullptr)
//                 m_pMirroringRoutine(m_tempQueueItem[i]);
//         }
     }
}
