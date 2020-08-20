#ifndef MIR_QUEUEHANDLER_H
#define MIR_QUEUEHANDLER_H

#include "mir_queue.h"
#include "MirroringCallback.h"
#include "ThreadHelper.h"

#include <QThread>
#include <QObject>
#include <QTimer>

class MIR_QueueHandler : public QObject
{
    Q_OBJECT
public:
    explicit MIR_QueueHandler(QObject *parent = nullptr);
    ~MIR_QueueHandler();

private:
    MIR_Queue m_mirQueue;

//    // JPG 데이터 조작 중 혹시라도 최대 패킷 크기인 1MB를 벗어나 Crash가 발생할 경우를 대비해서 2MB로 잡음
//    BYTE m_tempQueueItem[MIR_DEFAULT_MEM_POOL_UNIT];

    BYTE* m_tempQueueItem;

    bool m_isThreadReady;
    bool m_doExitThread;
    bool m_isThreadRunning;

    QThread m_hThread;

//    QTimer m_timer;

    PMIRRORING_ROUTINE m_pMirroringRoutine;

    ThreadHelper m_threadHelper;

public:
    bool StartThread(PMIRRORING_ROUTINE pMirroringRoutine);
    void StopThread();
    bool IsOnService();
    void EnQueue(int nHpNo, BYTE* item);

    void SetIsThreadRunning(bool isThreadRunning);
    bool IsThreadRunning();

    void OnMirroring();
    void OnStop();

protected:
    static void THREAD_PROC(void* pContext);

private:
    void ResetVars();

signals:
    void processPacket(BYTE*);

public slots:
    void OnStarted();
    void OnFinished();
    void OnMirroringFrame();
};

#endif // MIR_QUEUEHANDLER_H
