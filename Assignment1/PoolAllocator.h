#ifndef PoolAllocator_h
#define PoolAllocator_h
#include <cstdlib>
#include <mutex>

#include "MemoryManager.h"

#define MB(mb) mb * 1024 * 1024

template<typename T>
class PoolAllocator
{
public:
    struct Block
    {
        Block* pNext = nullptr;
    };

    inline PoolAllocator(int sizeInBytes = MB(4))
        : m_pMemory(nullptr),
        m_pFreeListHead(nullptr),
		m_pToFreeListHead(nullptr),
        m_SizeInBytes(sizeInBytes),
		m_FreeMutex(),
		m_ToFreeMutex()
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
		std::lock_guard<std::mutex> lock(m_FreeMutex);
        
		Block* pCurrent = m_pFreeListHead;
		if (!pCurrent)
		{
			std::lock_guard<std::mutex> lock(m_ToFreeMutex);

			m_pFreeListHead = m_pToFreeListHead;
			m_pToFreeListHead = nullptr;
		}
		else
		{
			m_pFreeListHead = m_pFreeListHead->pNext;
		}
        return (void*)pCurrent;
    }


    template<typename... Args>
    inline T* MakeNew(Args&& ... args)
    {
        return new(AllocateBlock()) T(std::forward<Args>(args) ...);
    }
    
    
    inline void Free(T* pObject)
    {
		std::lock_guard<std::mutex> lock(m_ToFreeMutex);

		pObject->~T();

        Block* pFirst = (Block*)pObject;
        pFirst->pNext = m_pToFreeListHead;
		m_pToFreeListHead = pFirst;
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
	Block* m_pToFreeListHead;
    size_t m_SizeInBytes;
	std::mutex m_FreeMutex;
	std::mutex m_ToFreeMutex;
};

#endif
