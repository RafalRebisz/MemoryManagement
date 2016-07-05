#pragma once

#include <string>
#include <unordered_map>


#ifdef _DEBUG
#include "MemoryAllocationInfo.h"
// Type definitions for memory allocation track pair and iterator

// std map typedef
typedef std::unordered_map<void*, MemoryAllocationInfo> AllocationMap;
// std pair typedef
typedef std::pair<void*, MemoryAllocationInfo> AllocationPair;
// constant std map iterator typedef
typedef std::unordered_map<void*, MemoryAllocationInfo>::const_iterator ConstAllocationIt;
#endif //if not Debug



// class: MemoryPool	Author: Rafal Rebisz

// Memory Pool: Defines abstract base object
// that other pools are derived from 
class MemoryPool
{
public:
	// constructor
	MemoryPool( void* memory, size_t size, std::string poolID, std::string poolType );
	// Destructor
	virtual ~MemoryPool( void );

	// Allocate / Deallocate 
	// Must be implemented in deriving object
	virtual void* Allocate( size_t size ) = 0;
	virtual void Deallocate( void* address ) = 0;

	// Returns pool size
	virtual size_t GetPoolSize( void ) const { return m_poolSize; }

	// returns pool Id
	virtual const std::string& GetPoolID( void ) const { return m_poolId; }

	// Returns pool Type
	virtual const std::string& GetPoolType( void ) const { return m_poolType; }

	// Returns Pointer To Memory That Pool operates on
	virtual void* GetMemoryPointer( void ) const { return m_poolMemory; }

	// Returns number of currently allocated blocks
	virtual unsigned int GetNumberOfAllocations( void ) const { return m_nrOfAllocations; }

	// Returns total size of allocations
	virtual size_t GetTotalAllocated( void ) const { return m_totalAllocated; }

	// Returns number of blocks in pool 
	virtual unsigned int GetNumberOfBlocks() const { return m_nrOfBlocks; }

public: // Methods used to track memory leaks

	// For Debug Use Only
#ifdef _DEBUG

	// Adds Allocation Track 
	virtual void AddAllocationTrack(void* ptr, std::string file, unsigned int line, size_t size);
	// Removes Allocation track
	virtual void RemoveAllocationTrack( void* ptr );
	// Dump memory leaks into *.txt file
	virtual void DumpMemoryLeaks(std::string fileName) const;
	// Method returns a constant reference to allocation map
	virtual const AllocationMap& GetAllocationMap(void) const { return m_allocationMap; }

protected:// internal methods
	// Method returns true if given memory address
	// was allocated in this pool
	virtual bool CheckIfAllocatedHere( void* ptrToCheck ) const;

#endif // if not Debug


protected: // Members

	// Memory that pool operates on 
	void* m_poolMemory;

	// pool size 
	size_t m_poolSize;

	// Pool ID
	std::string m_poolId;

	// Pool type
	std::string m_poolType;

	// stores number of allocation
	unsigned int m_nrOfAllocations;



	// stores total amount allocated without overhead
	size_t m_totalAllocated;

	// stores number of blocks
	unsigned int m_nrOfBlocks;

#ifdef _DEBUG
	// Map Of Allocation information
	AllocationMap m_allocationMap;
#endif // if not debug

};