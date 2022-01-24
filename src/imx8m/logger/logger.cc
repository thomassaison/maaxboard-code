#include "logger.hh"
#include <limits.h>

namespace imx8m{
	namespace logger{

void Logger::push(const std::string &string) noexcept{
	//lock guard is automatically released when lock goes out of scope
	const std::lock_guard<std::mutex> lock(this->__mutex);
	if (this->__buffer.size() + string.size() + 1 > PREALLOC_SIZE)
		this->__full = true;
	this->my_push(string);
}

void Logger::my_push(const std::string& s) noexcept{
	if (!this->__full) {
		std::copy(s.begin(), s.end(), std::back_inserter(this->__buffer));
		this->__buffer.push_back('\n');
	}
	else {
		std::copy(s.begin(), s.end(), std::back_inserter(this->__cache));
		this->__cache.push_back('\n');
	}
}

void Logger::swap(Logger &sb) noexcept{
		std::unique_lock<std::mutex> l1(__mutex);
		std::unique_lock<std::mutex> l2(sb.__mutex);
		
		this->__buffer.swap(sb.__buffer);
		this->__cache.swap(sb.__cache);
		std::swap(__file, sb.__file);
		std::swap(__error, sb.__error);
		std::swap(__full, sb.__full);
	}


void Logger::flush() noexcept{
	const std::lock_guard<std::mutex> lock(this->__mutex);
	flush_mutex();
}

void Logger::flush_mutex() noexcept{
	size_t count = 0;
	ssize_t ret;

	do {
		ret = write(__file, __buffer.data(), __buffer.size() * sizeof(char));
		if (ret < 0) {
			if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
				continue;
			break;
		}
		count += static_cast<size_t>(ret & SSIZE_MAX);
	} while(count < __buffer.size());


	this->__buffer.clear();
	this->__full = false;

	if (!__cache.empty()){
		count = 0;
		do {
			ret = write(__file, __cache.data(), __cache.size() * sizeof(char));
			if (ret < 0) {
				if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
					continue;
				break;
			}
			count += static_cast<size_t>(ret & SSIZE_MAX);
		} while(count < __cache.size());

		__cache.clear();
	}
}

void Logger::debug() const noexcept{
	std::cout << "Vector size:" << __buffer.size() << "\nCache size: " << __cache.size() << "\nCapacity: "<< __buffer.capacity() << '\n';
	for (size_t i =0; i < __buffer.size(); i++){
		std::cout << __buffer[i];
	}
	std::cout << std::endl;
}

	}
}