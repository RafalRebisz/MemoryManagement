#pragma once

#include "MemoryPool.h"
#include <assert.h>
#include <string>


//	Class:		DynamicAllocationSizePool		
//	Author:		Rafal Rebisz
//	Purpose:	Defines a memory pool from witch 
//				allocations of any size can be done 

//	Use:		Instantiate passing pointer to preallocated memory, pool size 
//				(the memory size) and ID into constructor, 
//				call Allocate passing in requested size in order to
//				allocate memory, call Deallocate passing in pointer to
//				previously allocated memory to delete it

//	NOTE:		Minimum pool and memory size must be at least 28 bytes

class DynamicAllocationSizePool: public MemoryPool
{
private: // Structures 

	// semantic structure defines Allocation Block 
	struct AllocationBlock
	{
	public:
		AllocationBlock* LogicalNext;
		AllocationBlock* LogicalPrevious;

		AllocationBlock* PhysicalNext;
		AllocationBlock* PhysicalPrevious;

		size_t allocSize;
		bool isAllocated;
	};
	//********************************************************//

	// semantic structure defines recycled blocks linked list 
	struct RecycledBlocks
	{
	public:
		RecycledBlocks();
		~RecycledBlocks();

		// Returns pointer t first/last element
		AllocationBlock* GetFirst() const;
		AllocationBlock* GetLast() const;

		// Method inserts block into list
		void Insert( AllocationBlock* block );
		// Method removes block from list
		void Remove( AllocationBlock* block );

	private:
		// pointers to first and last element 
		AllocationBlock* head;
		AllocationBlock* tail;
	};
	//********************************************************//

public: // Methods

	// Constructor
	DynamicAllocationSizePool(void* memory,size_t poolSize, std::string poolID);
	// Destructor
	virtual ~DynamicAllocationSizePool(void);

	// Methods used to allocate and free memory
	virtual void* Allocate( size_t requestedSize);
	virtual void Deallocate( void* address );

	// Returns total size of overhead
	virtual size_t GetTotalOverhead(void) const { return m_totalOverhead; }

private: // internal methods

	// Method used to find block of best size in recycled block list  
	AllocationBlock* FindBlockOfBestSize( size_t requestedSize ) const;

	// Method used to recycle block found by method above 
	void* RecycleBlock( AllocationBlock* blockToUse, size_t requestedSize);

	// Method creates new block of given size at given address given size,
	// sets all links to nullptr, new block "isAllocated" member is set to false
	AllocationBlock* CreateBlock( char* atAddress, size_t size ) const;

private: // Members

	// Pointer to "main" block
	//  witch contain free memory
	AllocationBlock* m_mainBlock;
	
	// doubly linked list containing
	// blocks witch can be recycled
	RecycledBlocks m_recycledBlocks;

	// overhead for each allocation in bytes
	const size_t m_OVERHEAD;

	// cached total overhead size in bytes
	size_t m_totalOverhead;
};
