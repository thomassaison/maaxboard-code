#include "logger.hh"
#include <limits.h>

namespace imx8m{
	namespace logger{

		void Logger::push(const std::string &string) noexcept{
			const std::unique_lock<std::mutex> lock(this->__mutex);
			[[gnu::unlikely]] if (this->__buffer.size() + string.size() + 1 > PREALLOC_SIZE)
				this->__full = true;
			this->__push(string);
		}

		void Logger::__push(const std::string& s) noexcept{
			[[gnu::likely]] if (!this->__full) {
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
			const std::unique_lock<std::mutex> lock(this->__mutex);
			__flush_mutex();
		}

		void Logger::__write(int fd, std::vector<char>& vec) noexcept {
			size_t count = 0;
			ssize_t ret;

			const size_t size = vec.size();
			const char *ptr = vec.data();

			do {
				ret = write(fd, ptr + count, size - count);
				if (ret < 0) {
					if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
						continue;
					break;
				}
				count += static_cast<size_t>(ret & SSIZE_MAX);
			} while(count < size);
		}

		void Logger::__flush_mutex() noexcept{
			__write(__file, __buffer);

			this->__buffer.clear();
			this->__full = false;

			if (!__cache.empty()){
				__write(__file, __cache);
				__cache.clear();
			}
		}

		void Logger::debug() const noexcept {

			std::cout << "Vector size:"
					  << __buffer.size()
					  << "\nCache size: "
					  << __cache.size()
					  << "\nCapacity: "
					  << __buffer.capacity()
					  << '\n';

			for (size_t i =0; i < __buffer.size(); i++)
				std::cout << __buffer[i];
			std::cout << std::endl;
		}

	}
}