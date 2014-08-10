/*
	Logging memory consumption of large data structures

	Author:   Steffen Rendle, http://www.libfm.org/
	modified: 2011-03-10

	Copyright 2011 Steffen Rendle, see license.txt for more information
*/

#ifndef MEMORY_H_
#define MEMORY_H_

#include <vector>
#include <assert.h>



typedef unsigned long long int uint64;
typedef signed long long int int64;


class MemoryLog {
	private:
		uint64 mem_size;
		
	public:		
		static MemoryLog& getInstance() {
    			static MemoryLog instance;
    			return instance;
		}
			
		MemoryLog() {
			mem_size = 0;
		}

		void logNew(std::string message, uint64 size, uint64 count = 1) {
			mem_size += size*count;
			// std::cout << "total memory consumption=" << mem_size << " bytes" << "\t" << "reserving " << count << "*" << size << " for " << message << std::endl;
		}
		void logFree(std::string message, uint64 size, uint64 count = 1) {
			mem_size -= size*count;
			// std::cout << "total memory consumption=" << mem_size << " bytes" << std::endl;
		}

};


#endif
