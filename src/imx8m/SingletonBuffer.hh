#pragma once
#include <string>
#include <mutex>
#include <queue>
#include <vector>
#include <iostream>
#include <fstream>
//this class is a singleton

#define PREALLOC_SIZE 30

class SingletonBuffer
{
protected:

	SingletonBuffer() {
	}
	static SingletonBuffer* __SingletonBuffer;

public:

	SingletonBuffer(SingletonBuffer& other) = delete;
	void operator=(const SingletonBuffer&) = delete;
	~SingletonBuffer() {
		if (this->__file.is_open())
			this->__file.close();
	}

	void init(const std::string &string);

	void push(const std::string& string);
	void push(const char* string) {
		this->push(std::string(string));
	}
	void my_push(const std::string& s);
	void flush();

	static SingletonBuffer* GetInstance();
	static SingletonBuffer* GetInstance(const std::string &string);
	void flush_mutex();
	void debug()const ;
	std::vector<char> getBuffer(){
		return this->__buffer;
	}

	bool is_okay() {
		return !this->__error;
	}

private:

	std::mutex __mutex;
	std::vector<char> __buffer;
	std::vector<char> __cache;
	std::ofstream __file;
	bool __error = false;
	bool __full = false;
};

