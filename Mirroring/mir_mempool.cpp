#include "mir_mempool.h"

MIR_MemPool::MIR_MemPool(int nAllocUnit, int nAllocUnitCount)
    : m_nAllocUnit(nAllocUnit),
      m_nAllocUnitCount(nAllocUnitCount)
{
    m_pMemPool = (BYTE*)malloc(nAllocUnit * nAllocUnitCount);
    m_pbyAllocedFlag = (BYTE*)malloc(sizeof(BYTE) * nAllocUnitCount);
    memset(m_pbyAllocedFlag, 0, sizeof(BYTE) * nAllocUnitCount);
    m_nAllocedCount = 0;
}

MIR_MemPool::~MIR_MemPool()
{
    m_mMemPool.lock();

    if(m_pMemPool)
    {
        free(m_pMemPool);
        m_pMemPool = nullptr;
    }

    if(m_pbyAllocedFlag)
    {
        free(m_pbyAllocedFlag);
        m_pbyAllocedFlag = nullptr;
    }

    m_mMemPool.unlock();
}

void* MIR_MemPool::Alloc()
{
    void* r = nullptr;

    m_mMemPool.lock();

    // 미리 할당된 메모리가 사용 전이면, 미리 할당된 메모리를 사용하고,
    // 그렇지 않으면, 새로운 메모리를 할당
    if(m_nAllocedCount < m_nAllocUnitCount)
    {
        for(int i = 0; i < m_nAllocUnitCount; ++i)
        {
            if(m_pbyAllocedFlag[sizeof(BYTE) * i] == 0)
            {
                r = (void*)&m_pMemPool[m_nAllocUnit * i];
                m_pbyAllocedFlag[sizeof(BYTE) * i] = 1;

                ++m_nAllocedCount;
                break;
            }
        }
    }
    else
    {
        r = malloc(m_nAllocUnit);
    }

    m_mMemPool.unlock();

    return r;
}

void MIR_MemPool::Free(void *pMem)
{
    if(pMem == nullptr)
    {
        return;
    }

    m_mMemPool.lock();

    bool isFreed = false;

    if(m_nAllocedCount > 0)
    {
        for(int i = 0; i < m_nAllocUnitCount; ++i)
        {
            if(pMem == (void*)&m_pMemPool[m_nAllocUnit * i])
            {
                m_pbyAllocedFlag[sizeof(BYTE) * i] = 0;
                --m_nAllocedCount;

                isFreed = true;
                break;
            }
        }
    }

    if(!isFreed)
    {
        free(pMem);
    }

    m_mMemPool.unlock();
}
