#include "logger.hh"

namespace imx8m{
	namespace logger{

void Logger::push(const std::string &string) noexcept{
	//lock guard is automatically released when lock goes out of scope
	const std::lock_guard<std::mutex> lock(this->__mutex);
	std::cout << "PUSH : before\n";
	debug();
	if (this->__buffer.size() + string.size() + 1 > PREALLOC_SIZE)
		this->__full = true;
	this->my_push(string);
	std::cout << "PUSH: after\n";
	debug();
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

/*
void Logger::init(const std::string &string) {
	std::cout << "init\n";
	debug();
	this->__file.exceptions(std::ifstream::badbit);
	try {
		this->__file.open(string);
	}
	catch (const std::ifstream::failure e) {
		this->__error = true;
	}
	if ((this->__file.rdstate() & std::ifstream::failbit) != 0)
		this->__error = true;

}
*/


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
	ssize_t ret;

	while((ret = write(__file, __buffer.data(), __buffer.size() * sizeof(char))) < 0)
		if (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK)
			break;

	this->__buffer.clear();
	this->__full = false;

	if (!__cache.empty()){
		while((ret = write(__file, __buffer.data(), __buffer.size() * sizeof(char))) < 0)
			if (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK)
				break;
		__cache.clear();
	}
/*	if (this->__cache.size() >= PREALLOC_SIZE) {
		for (int i = 0; i < PREALLOC_SIZE; i++) {
			this->__buffer.push_back(this->__cache[i]);
		}
		//std::copy(this->__cache.begin(), this->__cache.begin() + PREALLOC_SIZE, std::back_inserter(this->__buffer));
		this->__cache.erase(this->__cache.begin(), this->__cache.begin() + PREALLOC_SIZE);
		this->flush_mutex();
	}
	else
		std::copy(this->__cache.begin(), this->__cache.end(), std::back_inserter(this->__buffer));
*/
}

//Logger* Logger::__Logger = nullptr;;

void Logger::debug() const noexcept{
	std::cout << "Vector size:" << __buffer.size() << "\nCache size: " << __cache.size() << std::endl;
}

	}
}