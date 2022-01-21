#include "SingletonBuffer.hh"

SingletonBuffer* SingletonBuffer::GetInstance(const std::string &string) {
	if (__SingletonBuffer == nullptr) {
		__SingletonBuffer = new SingletonBuffer();
		__SingletonBuffer->init(string);
	}
	return __SingletonBuffer;
}

void SingletonBuffer::push(const std::string &string) {
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

void SingletonBuffer::my_push(const std::string& s) {
	if (!this->__full) {
		std::copy(s.begin(), s.end(), std::back_inserter(this->__buffer));
		this->__buffer.push_back('\n');
	}
	else{
		std::copy(s.begin(), s.end(), std::back_inserter(this->__cache));
		this->__cache.push_back('\n');
	}
}


void SingletonBuffer::init(const std::string &string) {
	this->__buffer.reserve(PREALLOC_SIZE);
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

SingletonBuffer* SingletonBuffer::GetInstance() {
	if (__SingletonBuffer == nullptr) {
		__SingletonBuffer = new SingletonBuffer();
		__SingletonBuffer->__error = true;
	}
	return __SingletonBuffer;
}

void SingletonBuffer::flush() {
	const std::lock_guard<std::mutex> lock(this->__mutex);
	flush_mutex();
}

void SingletonBuffer::flush_mutex() {
	std::ostream_iterator<char> output_iterator(this->__file, "");
	std::cout << "flush\n";
	debug();
	std::copy(this->__buffer.begin(), this->__buffer.end(), output_iterator);
	this->__buffer.clear();
	this->__full = false;
	if (this->__cache.size() >= PREALLOC_SIZE) {
		for (int i = 0; i < PREALLOC_SIZE; i++) {
			this->__buffer.push_back(this->__cache[i]);
		}
		//std::copy(this->__cache.begin(), this->__cache.begin() + PREALLOC_SIZE, std::back_inserter(this->__buffer));
		this->__cache.erase(this->__cache.begin(), this->__cache.begin() + PREALLOC_SIZE);
		this->flush_mutex();
	}
	else
		std::copy(this->__cache.begin(), this->__cache.end(), std::back_inserter(this->__buffer));

}

SingletonBuffer* SingletonBuffer::__SingletonBuffer = nullptr;;

void SingletonBuffer::debug() const {
	std::cout << "Vector size:" << __buffer.size() << "\nCache size: " << __cache.size() << std::endl;
}