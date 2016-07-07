// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR 
// THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "FixedAllocationSizePool.h"
#include <assert.h>

FixedAllocationSizePool::FixedAllocationSizePool( void* memory, unsigned int nrOfBlocks, size_t blockSize, std::string poolID ):
	MemoryPool( memory, (nrOfBlocks*blockSize), poolID, "FixedAllocationSizePool" ),
	m_blockSize( blockSize )
{
	assert( blockSize >= sizeof( AllocationBlock ) && "Memory pool does not support allocations smaller than 4 bytes" );
	
	m_nrOfBlocks = nrOfBlocks;

	// create first block
	char* bytePtr = reinterpret_cast<char*>(m_poolMemory);

	m_freeBlocks = reinterpret_cast<AllocationBlock*>(bytePtr);
	m_freeBlocks->nextFreeBlock = nullptr;

	// create proceeding blocks omitting first one 
	unsigned int i = 1;
	for(i; i < nrOfBlocks; i++)
	{
		AllocationBlock* currentBlock = reinterpret_cast<AllocationBlock*>(bytePtr + (blockSize * i));
		currentBlock->nextFreeBlock = m_freeBlocks;
		m_freeBlocks = currentBlock;
	}
}
/////////////////////////////////////////////////////

FixedAllocationSizePool::~FixedAllocationSizePool()
{
	m_blockSize = 0;
	m_freeBlocks = nullptr;
}
/////////////////////////////////////////////////////

// Method used to allocate memory and return it's address
void*
FixedAllocationSizePool::Allocate( size_t size )
{
	assert( size <= m_blockSize && "Incorrect allocation size" );
	assert( m_freeBlocks != nullptr && "No Free Memory" );

	// store firs available block, this block is 
	// going to be used to allocate memory
	AllocationBlock* blockToAllocate = m_freeBlocks;

	// update linked list
	m_freeBlocks = m_freeBlocks->nextFreeBlock;

	m_nrOfAllocations++;
	m_totalAllocated += m_blockSize;

	return blockToAllocate;
}
/////////////////////////////////////////////////////

// Method used to return memory into pool 
void 
FixedAllocationSizePool::Deallocate( void* address )
{
#ifdef _DEBUG
	assert( CheckIfAllocatedHere( address ) == true && "Memory wasn't allocated in this pool !" );
#endif

	// Create block at returned address
	AllocationBlock* returnedBlock = reinterpret_cast<AllocationBlock*>(address);

	// insert returned block into free blocks list
	returnedBlock->nextFreeBlock = m_freeBlocks;
	m_freeBlocks = returnedBlock;

	m_nrOfAllocations--;
	m_totalAllocated -= m_blockSize;
}
/////////////////////////////////////////////////////