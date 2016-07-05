#pragma once

#include "MemoryPool.h"


// size 132 bytes
class FixedAllocationSizePool: public MemoryPool
{
private:

	// defines allocation block structure
	struct AllocationBlock
	{
		AllocationBlock* nextFreeBlock;
	};
	
public: // Methods

	// Constructor
	FixedAllocationSizePool(void* memory,unsigned int nrOfBlocks,size_t blockSize, std::string poolID);
	// Destructor
	virtual ~FixedAllocationSizePool();

	// Methods used to allocate and free memory
	virtual void* Allocate( size_t size );
	virtual void Deallocate( void* address );

	// Returns block size in bytes
	virtual size_t GetBlockSize() const { return m_blockSize; }
	////////////////////////////////////////

private: // Data members

	// stores block size in bytes
	size_t m_blockSize;

	// Singly linked list of avaliable blocks
	AllocationBlock* m_freeBlocks;
};