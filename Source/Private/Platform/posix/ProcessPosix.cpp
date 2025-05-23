
#if defined(__unix__) || defined(__linux__) || defined(__APPLE__)
#define POSIX 
#endif

#ifdef POSIX
#include "Platform/posix/ProcessPosix.h"

namespace aby::sys::posix {

	Process::Process(const std::function<void(const std::string&)>& read_callback) :
		sys::Process(read_callback) 
	{

	}

	Process::~Process() {

	}

	bool Process::is_open() {
		return false;
	}


	bool Process::open(const std::string& cmd) {
		return false;
	}

	void Process::close() {

	}
	void Process::write(const std::string& data) {

	}

	void Process::kill() {

	}

}

#endif
