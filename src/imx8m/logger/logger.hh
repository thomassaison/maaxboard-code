#pragma once
#include <string>
#include <mutex>
#include <queue>
#include <vector>
#include <iostream>
#include <fstream>
//this class is a singleton

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define PREALLOC_SIZE 8192
namespace imx8m{
	namespace logger{
		class Logger
		{
		public:
			Logger(const size_t size = PREALLOC_SIZE) noexcept
				:__file{-1}
			{
				this->__buffer.reserve(size);
			}

			Logger(const char *path, const size_t size = PREALLOC_SIZE) noexcept
				: Logger(size)
			{
				this->open(path);
			}

			Logger(const std::string& path, const size_t size = PREALLOC_SIZE) noexcept
				: Logger(path.c_str(), size)
			{}

			Logger(const Logger&) = delete;
			Logger& operator=(const Logger&) = delete;

			Logger& operator=(Logger&& sb) noexcept {
				this->swap(sb);
				return *this;
			}

			Logger(Logger&& sb) noexcept
				: Logger()
			{
				this->swap(sb);
			}

			~Logger() noexcept {
				if (__file != -1)
					close(__file);
			}

			void open(const std::string& string) noexcept{
				this->open(string.c_str());
			}

			void open(const char *string) noexcept{
				this->__file = ::open(string,
									  O_APPEND | O_CREAT | O_WRONLY,
									  S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH);
			}

			void push(const std::string& string) noexcept;

			void push(const char* string) noexcept {
				this->push(std::string(string));
			}

			void flush() noexcept;
			
			void swap(Logger &sb) noexcept;

			void debug() const noexcept;

			const std::vector<char>& getBuffer() const noexcept{
				return this->__buffer;
			}

			bool is_okay() const noexcept {
				return !this->__error && __file >= 0;
			}

		private:
			
			std::mutex __mutex;
			std::vector<char> __buffer;
			std::vector<char> __cache;
			int __file;
			bool __error = false;
			bool __full = false;

			void __push(const std::string& s) noexcept;
			void __flush_mutex() noexcept;
			static void __write(int fd, std::vector<char>& __vec) noexcept;
		};
	}
}