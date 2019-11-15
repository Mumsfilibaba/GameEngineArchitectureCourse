#ifndef PoolAllocator_h
#define PoolAllocator_h
#include <cstdlib>

#include "MemoryManager.h"

template<typename T>
class PoolAllocator
{
private:
    struct Block
    {
        Block* pNext = nullptr;
    };

public:
    inline PoolAllocator(int sizeInBytes = 4096)
        : m_pMemory(nullptr),
        m_pFreeListHead(nullptr),
        m_SizeInBytes(sizeInBytes)
    {
        constexpr size_t blockSize = std::max(sizeof(T), sizeof(Block));
        assert(m_SizeInBytes % blockSize == 0);
        
        //Allocate mem
        //m_pMemory = malloc(m_SizeInBytes);
		m_pMemory = MemoryManager::getInstance().Allocate(m_SizeInBytes);
        
        //Init blocks
        Block* pOld        = nullptr;
        Block* pCurrent = (Block*)m_pMemory;
        m_pFreeListHead = pCurrent;

        int blockCount = m_SizeInBytes / blockSize;
        for (int i = 0; i < blockCount; i++)
        {
            pCurrent->pNext     = (Block*)(((char*)pCurrent) + blockSize); //HACKING;
            
            pOld     = pCurrent;
            pCurrent = pCurrent->pNext;
        }

        //Set the last valid ptr's next to null
        pOld->pNext = nullptr;
    }

    
    inline ~PoolAllocator()
    {
        if (m_pMemory)
        {
            //free(m_pMemory);
            m_pMemory = nullptr;
        }
    }


    inline void* AllocateBlock()
    {
        Block* pCurrent = m_pFreeListHead;
        m_pFreeListHead = m_pFreeListHead->pNext;
        return (void*)pCurrent;
    }


    template<typename... Args>
    inline T* MakeNew(Args&& ... args)
    {
        return new(AllocateBlock()) T(std::forward<Args>(args) ...);
    }
    
    
    inline void Free(T* pObject)
    {
        Block* pFirst = (Block*)pObject;
        pFirst->pNext = m_pFreeListHead;
        m_pFreeListHead = pFirst;
    }
    
    
    inline int GetBlockSize() const
    {
        return sizeof(T);
    }
    
    
    inline int GetChunkSize() const
    {
        return m_SizeInBytes;
    }
private:
    void* m_pMemory;
    Block* m_pFreeListHead;
    size_t m_SizeInBytes;
};

#endif
