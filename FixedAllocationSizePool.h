// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR 
// THE USE OR OTHER DEALINGS IN THE SOFTWARE.

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