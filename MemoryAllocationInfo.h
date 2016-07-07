// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR 
// THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#pragma once

#include <string>

// class MemoryAllocationInfo		Author: Rafal Rebisz

// Object used to store information about memory allocation 
// when application runs in debug mode 
class MemoryAllocationInfo
{
public:
	// constructor
	MemoryAllocationInfo(std::string file, unsigned int line, size_t size ):
		m_allocationFile( file ),
		m_allocationline( line ),
		m_allocationSize( size )
	{}
	//////////////////////////////////////////////////////
	// destructor
	~MemoryAllocationInfo(void)
	{}
	//////////////////////////////////////////////////////
	// returns file name 
	inline const std::string& GetFileName(void) const
	{
		return m_allocationFile;
	}
	//////////////////////////////////////////////////////
	//returns line number
	inline unsigned int GetLine(void) const
	{
		return m_allocationline;
	}
	//////////////////////////////////////////////////////
	// Returns allocation size
	inline size_t GetSize( void ) const
	{
		return m_allocationSize;
	}
	//////////////////////////////////////////////////////
private:

	std::string m_allocationFile;
	unsigned int  m_allocationline;
	size_t m_allocationSize;
};
