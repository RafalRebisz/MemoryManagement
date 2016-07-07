// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR 
// THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "DynamicAllocationSizePool.h"

// Constructor
DynamicAllocationSizePool::DynamicAllocationSizePool( void* memory, size_t poolSize, std::string poolID ):
	MemoryPool(memory,poolSize,poolID,"DynamicAllocationSizePool"),
	m_totalOverhead(0),
	m_OVERHEAD(sizeof(AllocationBlock))
{
	// Pool size must be at least 28 bytes
	assert( poolSize > (sizeof( AllocationBlock ) + sizeof(int)) && " Pool Size to small" );

	// Create the main memory block 
	m_mainBlock = CreateBlock( reinterpret_cast<char*>(memory), poolSize - m_OVERHEAD );
	m_totalOverhead += m_OVERHEAD;
}
///////////////////////////////////////////////////////////

// Destructor
DynamicAllocationSizePool::~DynamicAllocationSizePool(void)
{}
///////////////////////////////////////////////////////////


// Method allocates memory block of requested size
void* 
DynamicAllocationSizePool::Allocate( size_t requestedSize)
{
	// Temporary variables
	AllocationBlock* blockToUse = nullptr;

	// Check if any blocks can be recycled
	blockToUse = FindBlockOfBestSize(requestedSize);

	// if block can be recycled
	if(blockToUse != nullptr)
	{
		return RecycleBlock( blockToUse, requestedSize);
	}

	// if here than ether no recyclable blocks are available or
	// they are to small. Allocate from main block if a available
	else if(blockToUse == nullptr && m_mainBlock != nullptr)
	{ 
		// check if space will be left after allocation, must be 
		// enough to allocate at least 4bytes after creating new block

		if(((int)(m_mainBlock->allocSize - requestedSize)) >= ((int)(m_OVERHEAD + 4)))
		{
			// Calculate new size and address for mainBlock, it will be created
			// at the address of (mainBlock + allocationSize) 
			char* address = (reinterpret_cast<char*>(m_mainBlock) + m_OVERHEAD + requestedSize);
			size_t newBlockSize = (m_mainBlock->allocSize - m_OVERHEAD - requestedSize);

			// copy existing mainBlock address to the pointer of
			// block that will be used and update its values 
			blockToUse = m_mainBlock;
			blockToUse->allocSize = requestedSize;
			blockToUse->isAllocated = true;

			// Create new mainBlock at previously calculated address 
			m_mainBlock = CreateBlock( address, newBlockSize );

			// update physical links
			m_mainBlock->PhysicalPrevious = blockToUse;
			blockToUse->PhysicalNext = m_mainBlock;

			// Update values in pool data members
			m_totalOverhead += m_OVERHEAD;
			m_nrOfBlocks++;
			m_totalAllocated += requestedSize;
			m_nrOfAllocations++;

			return (++blockToUse);
		}
		// if mainBlock is big enough to allocate from it
		// but not big enough to split it, than it will be used 
		// "as it is" the mainBlock pointer will be set to nullptr
		// indicating that there is no free space in main memory 
		// however there still may be space available in recyclableBlocks list
		else if(m_mainBlock->allocSize >= requestedSize)
		{
			blockToUse = m_mainBlock;
			blockToUse->isAllocated = true;

			m_totalAllocated += requestedSize;
			m_nrOfAllocations++;

			m_mainBlock = nullptr;

			return (++blockToUse);
		}
	}

	// If here than ether no free memory available or available memory
	// is not big enough to allocate from 
	assert( false && "No Free Memory Or Pool has become fragmented" );
	return nullptr;
}
///////////////////////////////////////////////////////////


// Method used to return previously allocated memory 
void DynamicAllocationSizePool::Deallocate( void* address )
{
	// when in debug mode check if returned address belongs to given pool
#ifdef _DEBUG
	assert( CheckIfAllocatedHere( address ) == true && "Memory wasn't allocated in this pool !" );
#endif

	// decrement value of pointer to get to the memory occupied by block instance
	AllocationBlock* returnedBlock = reinterpret_cast<AllocationBlock*>(address);
	returnedBlock--;

	// update block flag 
	returnedBlock->isAllocated = false;
	// store block size in temp variable 
	size_t sizeReturned = returnedBlock->allocSize;

	// get preceding physical block
	AllocationBlock* physicalPrev = returnedBlock->PhysicalPrevious;

	// Check if preceding and returned block can be merged
	if(physicalPrev != nullptr && physicalPrev->isAllocated == false)
	{
		//if preceding block exist and it is free than it must be on recycledBlocks list 
		// Remove it from recycled list
		m_recycledBlocks.Remove( physicalPrev );


		// NOTE: because returnedBlock is at proceeding address 
		// the merging operation will be done from "right to left" witch means the
		// preceding block links and values are going to be updated and
		// current block "returnedBlock" will be "abandoned"

		// update preceding block size
		physicalPrev->allocSize += (returnedBlock->allocSize + m_OVERHEAD);

		// Update preceding block physical links
		physicalPrev->PhysicalNext = returnedBlock->PhysicalNext;
		if(physicalPrev->PhysicalNext != nullptr)
		{
			physicalPrev->PhysicalNext->PhysicalPrevious = physicalPrev;
		}

		// after merging operation preceding block became larger 
		// and links has been updated, store its address in returnedBlock pointer
		returnedBlock = physicalPrev;

		// update pool data members 
		// overhead and block number is decremented as 
		// blocks has been merged
		m_totalOverhead -= m_OVERHEAD;
		m_nrOfBlocks--;
	}

	// NOTE: if merging succeed returnedBlock pointer points to "new" bigger block else
	// returnedBlock is "untouched" in ether case the check can be done to see if it
	// can be merged with proceeding block

	// Get proceeding block
	AllocationBlock* physicalNext = returnedBlock->PhysicalNext;

	// if physical next block is nullptr and the main block is also nullptr
	// than current block "returnedBlock" must be the last physical block in pool, 
	// therefore it will become the mainBlock and method will terminate
	if(physicalNext == nullptr && m_mainBlock == nullptr)
	{
		// store current block address in mainBlock pointer
		m_mainBlock = returnedBlock;
		// update pool data members 
		m_nrOfAllocations--;
		m_totalAllocated -= sizeReturned;

		return;
	}
	else if(physicalNext == m_mainBlock)
	{
		// if here than next physical block is the m_mainBlock 
		// witch means current block "returnedBlock" can be merged with it
		// because the main block is at proceeding address the merging
		// operation will be done from "right to left" therefore returned 
		// block is going to become the mainBlock its links and values are going
		// to be updated and method will terminate

		// update current block size
		returnedBlock->allocSize += (m_mainBlock->allocSize + m_OVERHEAD);

		// set next physical block to nullptr as this block became the last physical block in pool
		returnedBlock->PhysicalNext = nullptr;

		// store returnedBlock address in the mainBlock pointer
		m_mainBlock = returnedBlock;
 
		// update pool data members

		// overhead and block number is decremented as 
		// blocks has been merged
		m_totalOverhead -= m_OVERHEAD;
		m_nrOfBlocks--;

		m_nrOfAllocations--;
		m_totalAllocated -= sizeReturned;

		return;
	}

	else if(physicalNext != nullptr && physicalNext->isAllocated == false)
	{
		// if here than next physical block must be free and on recycledBlocks
		// list witch means blocks can be merged, the merging operation
		// will be done from "right to left" current block "returnedBlock" is at preceding address  
		// and therefore its links and values are going to be updated, proceeding block will be "abandoned"
		// merged block will be inserted back to the recycledBlocks list and method will terminate

		// Remove proceeding block from recycled list
		m_recycledBlocks.Remove( physicalNext );

		// blocks Can be merged now

		// update current block size
		returnedBlock->allocSize += (physicalNext->allocSize + m_OVERHEAD);

		// update its physical links
		returnedBlock->PhysicalNext = physicalNext->PhysicalNext;
		if(returnedBlock->PhysicalNext != nullptr)
		{
			returnedBlock->PhysicalNext->PhysicalPrevious = returnedBlock;
		}

		// re-insert merged block to recycled list 
		m_recycledBlocks.Insert( returnedBlock );

		// update pool data members

		// overhead and block number is decremented as 
		// blocks has been merged
		m_totalOverhead -= m_OVERHEAD;
		m_nrOfBlocks--;

		m_nrOfAllocations--;
		m_totalAllocated -= sizeReturned;

		return;
	}
	else
	{
		// if here than block cannot be merged ether with previous 
		// and/or with next block just insert it to recycled list 
		m_recycledBlocks.Insert( returnedBlock );
		m_nrOfAllocations--;
		m_totalAllocated -= sizeReturned;

		return;
	}
}
///////////////////////////////////////////////////////////


/******************* Internal Methods *********************/

// internal method used to search through recycled 
// blocks list for block of best possible size 
// it searches through the list from front to back and back to front at the same time
// to minimize search time and it will return ether a block of size exactly
// as requested (if available) or the smallest block that allocation can be done from
// if no such block is available it will return nullptr indicating that none of available
// blocks are big enough to allocate from, or there is no blocks that can be recycled at all
DynamicAllocationSizePool::AllocationBlock* 
DynamicAllocationSizePool::FindBlockOfBestSize( size_t requestedSize) const
{
	AllocationBlock* tempFront = m_recycledBlocks.GetFirst();
	AllocationBlock* tempBack = m_recycledBlocks.GetLast();
	AllocationBlock* bestSoFar = nullptr;

	// While there are blocks available
	// the "back" pointer does not need to
	// be checked as if there are any blocks
	// on the list both pointers cannot be nullptr
	while(tempFront != nullptr)
	{
		// If block size is exactly as requested there 
		// is no need to search any more return the appropriate block
		// this is first of two ways a while loop will terminate
		if(tempFront->allocSize == requestedSize)
		{
			return tempFront;
		}
		else if(tempBack->allocSize == requestedSize)
		{
			return tempBack;
		}
		///////////////////////////////////////////

		// if the search meet than size will be checked and if
		// this is second way a while loop will terminate as if the search meet 
		// than there is no more block to search through if any block meeting 
		// the requirements has been found than it is stored in bestSoFar and
		// will be returned  
		if(tempFront == tempBack)
		{
			// If block is of right size
			if(tempFront->allocSize >= requestedSize)
			{
				// if bestSoFar is nullptr than just store block
				// pointer else check if size can be improved
				if(bestSoFar == nullptr)
				{
					bestSoFar = tempFront;
				}
				else if(bestSoFar->allocSize > tempFront->allocSize)
				{
					bestSoFar = tempFront;
				}
			}
			// nothing more to search
			// this will be ether a block that memory can be allocated from
			// or it will be nullptr indicating that no blocks can be recycled
			return bestSoFar; 
		}
		///////////////////////////////////////////////

		// if front and back search blocks are of the right size, they will be compared and
		// the block with the smaller size will be picked, next if the size of 
		// chosen block will better than the size of current block stored in bestSoFar 
		// than it will be picked and search will continue 
		if(tempFront->allocSize >= requestedSize && tempBack->allocSize >= requestedSize)
		{
			// if back block is smaller
			if(tempFront->allocSize > tempBack->allocSize)
			{
				// if bestSoFar is nullptr than just store block
				// pointer else check if size can be improved 
				if(bestSoFar == nullptr)
				{
					bestSoFar = tempBack;
				}
				else if(bestSoFar->allocSize > tempBack->allocSize)
				{
					bestSoFar = tempBack;
				}
			}
			else // if front block is smaller
			{
				// if bestSoFar is nullptr than just store block
				// pointer else check if size can be improved 
				if(bestSoFar == nullptr)
				{
					bestSoFar = tempFront;
				}
				else if(bestSoFar->allocSize > tempFront->allocSize)
				{
					bestSoFar = tempFront;
				}
			}
		}

		// if front search block is of the right size check if 
		// current bestSoFar can be improved 
		else if(tempFront->allocSize >= requestedSize)
		{
			// if bestSoFar is nullptr than just store block
			// pointer else check if size can be improved 
			if(bestSoFar == nullptr)
			{
				bestSoFar = tempFront;
			}
			else if(bestSoFar->allocSize > tempFront->allocSize)
			{
				bestSoFar = tempFront;
			}
		}

		// if back search block is of the right size check if 
		// current bestSoFar can be improved 
		else if(tempBack->allocSize >= requestedSize)
		{
			// if bestSoFar is nullptr than just store block
			// pointer else check if size can be improved 
			if(bestSoFar == nullptr)
			{
				bestSoFar = tempBack;
			}
			else if(bestSoFar->allocSize > tempBack->allocSize)
			{
				bestSoFar = tempBack;
			}
		}

		// Get next Blocks 
		tempFront = tempFront->LogicalNext;
		tempBack = tempBack->LogicalPrevious;

	}// while loop


	// The search never get here however this return is placed to get rid of the C4715 warning
	return bestSoFar;
}
///////////////////////////////////////////////////////////

// Internal Method used to recycle block found on 
// recycled blocks list.
// method will split block if its big enough, and if
// the block that is passed in will be used, newly
// created block will be inserted back to recycledBlocks list 
void* 
DynamicAllocationSizePool::RecycleBlock(AllocationBlock* blockToUse, size_t requestedSize)
{
	// Remove block from recycled list
	m_recycledBlocks.Remove( blockToUse );

	// check if given block is big enough to be split, size after it is split must be 
	// enough to allocate at least 4bytes after creating a new block
	if(((int)(blockToUse->allocSize - requestedSize)) >= ((int)(m_OVERHEAD + 4)))
	{
		// Calculate new size and address for new block, it will be created
		// at the address of (blockToUse + allocationSize) 
		char* address = (reinterpret_cast<char*>(blockToUse) + (m_OVERHEAD + requestedSize));
		size_t newBlockSize = (blockToUse->allocSize - requestedSize - m_OVERHEAD);

		// Create new block 
		AllocationBlock* newBlock = CreateBlock( address, newBlockSize );

		// update new block physical links
		newBlock->PhysicalPrevious = blockToUse;
		newBlock->PhysicalNext = blockToUse->PhysicalNext;
		if(newBlock->PhysicalNext != nullptr)
		{
			newBlock->PhysicalNext->PhysicalPrevious = newBlock;
		}

		// insert newly created block back onto recycled blocks list
		m_recycledBlocks.Insert(newBlock);

		// update blockToUse physical links
		blockToUse->PhysicalNext = newBlock;

		// update its size and set as allocated
		blockToUse->allocSize = requestedSize;
		blockToUse->isAllocated = true; 

		// update pool data members values 
		// overhead and number of blocks is updated as 
		// a new block was just created
		m_totalOverhead += m_OVERHEAD;
		m_nrOfBlocks++;

		m_totalAllocated += requestedSize;
		m_nrOfAllocations++;

		return (++blockToUse);
	}
	else // block is to small to be split, it will be used "as it is" 
	{
		blockToUse->isAllocated = true;
		m_totalAllocated += requestedSize;
		m_nrOfAllocations++;

		return (++blockToUse);
	}

	// this code will never be executed however it is placed here
	// to get rid of the C4715 warning
	return nullptr;
}
///////////////////////////////////////////////////////////

// internal method used to create block of given size at given address in pool
DynamicAllocationSizePool::AllocationBlock*
DynamicAllocationSizePool::CreateBlock( char* atAddress, size_t size ) const
{
	AllocationBlock* block = reinterpret_cast<AllocationBlock*>(atAddress);
	block->LogicalNext = nullptr;
	block->LogicalPrevious = nullptr;
	block->PhysicalNext = nullptr;
	block->PhysicalPrevious = nullptr;

	block->isAllocated = false;
	block->allocSize = size;

	return block;
}
///////////////////////////////////////////////////////////



//*******************************************************************************//

//***************** Recycled Blocks List Structure Definitions *****************//

// Constructor
DynamicAllocationSizePool::RecycledBlocks::RecycledBlocks():
	head(nullptr),
	tail(nullptr)
{}
///////////////////////////////////////////////////////////
// Destructor
DynamicAllocationSizePool::RecycledBlocks::~RecycledBlocks()
{
	head = nullptr;
	tail = nullptr;
}
///////////////////////////////////////////////////////////
// Method returns head of the list
DynamicAllocationSizePool::AllocationBlock*
DynamicAllocationSizePool::RecycledBlocks::GetFirst() const
{
	return head;
}
///////////////////////////////////////////////////////////
// Method returns tail of the list
DynamicAllocationSizePool::AllocationBlock*
DynamicAllocationSizePool::RecycledBlocks::GetLast() const
{
	return tail;
}
///////////////////////////////////////////////////////////

// Method inserts new block on to list 
// NOTE: block must be previously created
void 
DynamicAllocationSizePool::RecycledBlocks::Insert( AllocationBlock* block )
{
	if(tail)
	{
		tail->LogicalNext = block;
		block->LogicalPrevious = tail;
		tail = block;
	}
	else
	{
		head = block;
		tail = block;
	}
}
///////////////////////////////////////////////////////////

// Method removes block of the ist 
// NOTE: block will not be deleted it is only removed 
// and logical links are broken
void 
DynamicAllocationSizePool::RecycledBlocks::Remove( AllocationBlock* block )
{
	AllocationBlock* prev = block->LogicalPrevious;
	AllocationBlock* next = block->LogicalNext;
	block->LogicalNext = nullptr;
	block->LogicalPrevious = nullptr;

	if(prev != nullptr)
	{
		prev->LogicalNext = next;
	}
	else
	{
		head = next;
	}

	if(next != nullptr)
	{
		next->LogicalPrevious = prev;
	}
	else
	{
		tail = prev;
	}

}
///////////////////////////////////////////////////////////