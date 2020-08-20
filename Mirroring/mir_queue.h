#ifndef MIR_QUEUE_H
#define MIR_QUEUE_H

#include "mir_mempool.h"

#include <QMutex>
#include <QQueue>

class MIR_Queue
{
public:
    MIR_Queue();
    ~MIR_Queue();

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
};

#endif // MIR_QUEUE_H
