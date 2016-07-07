// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR 
// THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "MemoryPool.h"

#include <fstream>
#include <assert.h>

// Constructor
MemoryPool::MemoryPool( void* memory, size_t size, std::string poolID, std::string poolType ):
	m_poolMemory( memory ),
	m_poolSize( size ),
	m_poolId( poolID ),
	m_poolType( poolType ),
	m_nrOfAllocations( 0 ),
	m_totalAllocated( 0 ),
	m_nrOfBlocks(0)
{
	assert( m_poolMemory != nullptr && "Pool memory not allocated" );
}
/////////////////////////////////////////////////////////////

// Destructor
MemoryPool::~MemoryPool(void)
{
	// When in debug mode call dump memory leaks to save file
	// with any memory leaks detected if no leaks occurred file will be empty 
#ifdef _DEBUG
	m_allocationMap.clear();
#endif // if DEBUG
}
/////////////////////////////////////////////////////////////


// For Debug Use Only
#ifdef _DEBUG
// Add allocation track when memory
// is allocated to help track memory leaks 
void
MemoryPool::AddAllocationTrack( void* ptr, std::string file, unsigned int line,size_t size )
{
	m_allocationMap.insert(AllocationPair(ptr, MemoryAllocationInfo(file, line, size)));
}
///////////////////////////////////////////

// Remove allocation track from list when address is 
// Deallocated 
void
MemoryPool::RemoveAllocationTrack( void* ptr )
{
	m_allocationMap.erase( m_allocationMap.find( ptr ) );
}
//////////////////////////////////////////////

// If any leaks occurred their are going to
// be written into text file
// file format is : line - file Name
// If no leaks occurred file will be empty
void
MemoryPool::DumpMemoryLeaks(std::string fileName) const
{
	// Create and open file
	std::fstream file(fileName);

	std::string temp( (m_poolId + "MemoryLeaks.txt") );

	// if allocation map contain data than memory wasn't deallocated
	for(ConstAllocationIt It = m_allocationMap.begin(); It != m_allocationMap.end(); It++)
	{
		// Write detected leak information in to file
		temp.erase();
		temp += (("File:	" + It->second.GetFileName() + "\n") +
				 ("Line:	" + std::to_string(It->second.GetLine()) + "\n") +
				 ("Size:	" + std::to_string(It->second.GetSize()) + "\n"));
		

		file << temp;
	}

	file.close();
}
//////////////////////////////////////////////

// Method used to check if given chunk of memory was allocated in this pool
bool
MemoryPool::CheckIfAllocatedHere(void* ptrToCheck) const
{
	if (reinterpret_cast<char*>(ptrToCheck) >= reinterpret_cast<char*>(m_poolMemory) &&
		reinterpret_cast<char*>(ptrToCheck) <= (reinterpret_cast<char*>(m_poolMemory) + m_poolSize))
	{
		return true;
	}
	return false;
}
/////////////////////////////////////////////
#endif // if DEBUG