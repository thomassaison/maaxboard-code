#pragma once

#include <vector>
#include <cassert>
#include <climits>

namespace imx8m {
    namespace container {

        template<typename T>
        class UniqueRingVectorSorted
        {
        public:
            UniqueRingVectorSorted()
                : __vector{32}
            {}

            UniqueRingVectorSorted(const UniqueRingVectorSorted&) = delete;

            UniqueRingVectorSorted(UniqueRingVectorSorted&& vec) noexcept
                : UniqueRingVectorSorted()
            {
                this->swap(vec);
            }
            
            UniqueRingVectorSorted& operator=(const UniqueRingVectorSorted&) = delete;

            UniqueRingVectorSorted& operator=(UniqueRingVectorSorted&& vec) noexcept
            {
                this->swap(vec);
                return *this;
            }

            ~UniqueRingVectorSorted() = default;

            bool empty() const noexcept {
                return __len == 0;
            }

            T& front() noexcept {
                return __vector[__begin_idx];
            }

            void swap(UniqueRingVectorSorted& vec) noexcept;

            size_t search(const T& elm) const noexcept {
                return (!this->empty()) ? __search(0, __len - 1, elm) : 0;
            }

            bool insert(const T& elm) noexcept;
            bool insert(T&& elm) noexcept;

            void pop() noexcept;

            template<typename F>
            void foreach(F&& fn) noexcept;

            size_t len() const noexcept {
                return __len;
            }

        private:
            std::vector<T> __vector;
            size_t         __len       = 0;
            size_t         __begin_idx = 0;
            size_t         __end_idx   = 0;

            bool __insert(const T& elm) noexcept {
                size_t idx = search(elm);
                if (__vector[idx] == elm)
                    return false;
                __insert_always(idx, elm);
                return true;
            }

            size_t __search(size_t low, size_t high, const T& value) const noexcept;
            void   __insert_always(const size_t idx, const T& elm) noexcept;
        };

        template<typename T>
        template<typename F>
        void UniqueRingVectorSorted<T>::foreach(F&& fn) noexcept {
            const size_t capacity = __vector.capacity();
            for (size_t i = __begin_idx; i != __end_idx; i = (i + 1) % capacity)
                fn(__vector[i]);
        }

        template<typename T>
        bool UniqueRingVectorSorted<T>::insert(const T& elm) noexcept {
            return __insert(elm);
        }

        template<typename T>
        bool UniqueRingVectorSorted<T>::insert(T&& elm) noexcept {
            return __insert(elm);
        }

        template<typename T>
        size_t UniqueRingVectorSorted<T>::__search(size_t low,
                                                   size_t high,
                                                   const T& value)
        const noexcept
        {
            size_t mid;
            do {
                mid = (low + high) / 2;
                if (__vector[mid] > value)
                    high = mid - 1;
                else
                    low = mid + 1;
            } while (low <= high && __vector[mid] != value);
            return mid;
        }

        template<typename T>
        void UniqueRingVectorSorted<T>::pop() noexcept {
            if (!this->empty()) {
                ++__begin_idx;
                --__len;
                if (__begin_idx != __end_idx)
                    __begin_idx %= __vector.capacity();
                else {
                    __begin_idx = __end_idx = 0;
                    __vector.clear();
                }
            }
        }

        template<typename T>
        void UniqueRingVectorSorted<T>::swap(UniqueRingVectorSorted& vec) noexcept {
            std::swap(__vector, vec.__vector);
            std::swap(__begin_idx, vec.__begin_idx);
            std::swap(__end_idx, vec.__end_idx);
            std::swap(__len, vec.__len);
        }

        template<typename T>
        void UniqueRingVectorSorted<T>::__insert_always(const size_t idx, const T& elm) noexcept {
            assert(idx < __len);

            const size_t capacity = __vector.capacity();

            const size_t real_idx = (idx + __begin_idx) % capacity;

            ++__len;
            /* If empty, __begin_idx == 0 and __end_idx == 0 */
            if (__begin_idx == 0) { /* Empty or __begin == 0 */
                __vector.insert(__vector.begin() + (real_idx & LONG_MAX), std::move(elm));
                ++__end_idx;
            }
            else if (__begin_idx < __end_idx) { /* loop */
                __vector[0] = elm;
                __end_idx = 0;
            }
            else if (real_idx <= __end_idx) {
                for (size_t i = real_idx; i < __end_idx; ++i)
                    std::swap(__vector[real_idx], __vector[i + 1]);
                __vector[real_idx] = elm;
                ++__end_idx;
            }
            else {
                std::vector<T> v(capacity * 2);
                v.swap(__vector);
                this->foreach([&](T& e) { __vector.push_back(e); });
                __vector.push_back(elm);
                __begin_idx = 0;
                __end_idx = __len;
            }
        }
    }
}