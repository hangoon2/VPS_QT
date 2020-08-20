#ifndef MIR_QUEUE2_H
#define MIR_QUEUE2_H

#include "mir_mempool.h"

#include <QObject>
#include <QMutex>
#include <QQueue>

class MIR_Queue2 : public QObject
{
    Q_OBJECT
public:
    explicit MIR_Queue2(QObject *parent = nullptr);
    ~MIR_Queue2();

private :
    QQueue<BYTE*> m_mirQueue;
    QMutex m_mMirQueue;

    MIR_MemPool m_memPool;

public:
    void* AllocateItemMemory();
    void FreeItemMemory(void* pMemory);
    BYTE* CreateQueueItem(BYTE* pSrc);
    void DeleteQueueItem(BYTE* item);
    void ClearQueue();
    void Enqueue(BYTE* item);
    bool Dequeue(BYTE* o_item);

private:
    void ClearQueueInternal();

signals:
    void mirroringFrame();
};

#endif // MIR_QUEUE2_H
