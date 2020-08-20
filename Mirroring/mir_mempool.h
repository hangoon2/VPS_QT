#ifndef MIR_MEMPOOL_H
#define MIR_MEMPOOL_H

#include "VPSCommon.h"

#include <QMutex>

#define MIR_DEFAULT_MEM_POOL_UNIT       (1 * 960 * 960)
#define MIR_DEFAULT_MEM_POOL_UNIT_COUNT 20

class MIR_MemPool
{
public:
    MIR_MemPool(int nAllocUnit = MIR_DEFAULT_MEM_POOL_UNIT, int nAllocUnitCount = MIR_DEFAULT_MEM_POOL_UNIT_COUNT);
    ~MIR_MemPool();

    void* Alloc();
    void Free(void* pMem);

private:
    int m_nAllocUnit;
    int m_nAllocUnitCount;

    BYTE* m_pMemPool;
    BYTE* m_pbyAllocedFlag;
    int m_nAllocedCount;

    QMutex m_mMemPool;
};

#endif // MIR_MEMPOOL_H
