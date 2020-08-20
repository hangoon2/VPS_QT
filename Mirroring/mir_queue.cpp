#include "mir_queue.h"
#include "MIR_Common.h"

#include <QDebug>

MIR_Queue::MIR_Queue()
{

}

MIR_Queue::~MIR_Queue()
{
    ClearQueue();
}

void* MIR_Queue::AllocateItemMemory()
{
    return m_memPool.Alloc();
}

void MIR_Queue::FreeItemMemory(void *pMemory)
{
    m_memPool.Free(pMemory);
}

BYTE* MIR_Queue::CreateQueueItem(BYTE* pSrc)
{
    BYTE* r = nullptr;
    BYTE* pPacket = (BYTE*)pSrc;

    uint iDataLen = ntohl(*(uint32_t*)&pPacket[1]);
    uint nTotLen = CMD_HEAD_SIZE + iDataLen + CMD_TAIL_SIZE;
    if(nTotLen > MIR_DEFAULT_MEM_POOL_UNIT)
    {
        return r;
    }

    r = (BYTE*)AllocateItemMemory();

    if(r != nullptr)
    {
        memcpy(r, pSrc, nTotLen);
    }

    return r;
}

void MIR_Queue::DeleteQueueItem(BYTE *item)
{
    if(item != nullptr)
    {
        FreeItemMemory(item);
    }
}

void MIR_Queue::ClearQueueInternal()
{
    while(m_mirQueue.size() > 0)
    {
        BYTE* item = m_mirQueue.dequeue();
        DeleteQueueItem(item);
    }

    m_mirQueue.clear();
}

void MIR_Queue::ClearQueue()
{
    m_mMirQueue.lock();

    ClearQueueInternal();

    m_mMirQueue.unlock();
}

void MIR_Queue::Enqueue(BYTE* item)
{
    m_mMirQueue.lock();

    BYTE* itemClone = CreateQueueItem(item);
    if(itemClone != nullptr)
    {
        if(MIR_IsKeyFrame(itemClone))
        {
            ClearQueueInternal();
        }

        m_mirQueue.enqueue(itemClone);
    }

    m_mMirQueue.unlock();
}

bool MIR_Queue::Dequeue(BYTE *o_item)
{
    bool r = false;

    m_mMirQueue.lock();

    if(m_mirQueue.size() > 0)
    {
        BYTE* item = m_mirQueue.dequeue();

        memcpy(o_item, item, MIR_DEFAULT_MEM_POOL_UNIT);

        DeleteQueueItem(item);

        r = true;
    }

    m_mMirQueue.unlock();

    return r;
}
