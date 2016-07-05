#ifndef MEMORYALLOCATIONINFO_H
#define	MEMORYALLOCATIONINFO_H

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

#endif //MEMORYALLOCATIONINFO_H