#pragma once

#include <functional>
#include <string>
#include <ranges>

namespace okser {
    /**
    * A concept to check if a class can be used as an okser output.
    */
    template<typename T>
    concept Output = requires(T t, uint8_t byte)
    {
        { t.add(byte) };
    };

    /**
     * Useful output classes for serialized results
     */
    namespace out {
        /**
         * A simple output to a C++ std::string
         *
         * ## Example
         * \code
         * std::string result;
         * auto output = okser::out::stdstring{result};
         *
         * // ...
         *
         * okser::serialize<bundle>(output, 100, 200, 300);
         * \endcode
         */
        class stdstring {
        public:
            std::reference_wrapper<std::string> str;

            template<typename T>
            void add(const T &value) {
                str.get().append(value);
            }
        };

        template<>
        inline void stdstring::add(const unsigned char &value) {
            str.get().push_back(value);
        }

        /**
         * An output to a pre-allocated C++ range, such as an std::array
         *
         * ## Example
         * \code
         * std::array<uint8_t, 3> result;
         * auto output = okser::out::fixed_container{result};
         *
         * // ...
         *
         * okser::serialize<bundle>(output, 100, 200, 300);
         * \endcode
         *
         * @note This only works with containers that have a certain number of elements preallocated. For example,
         * an empty vector with 0 elements will thrown an error, while a vector with N elements will have those elements
         * replaced.
         */
        template<std::ranges::output_range<uint8_t> C>
        class fixed_container {
            private:
                C::iterator current;
                C::const_iterator last;
            public:
                explicit fixed_container(C& container) : current(container.begin()), last(container.end()) {}

                template<typename T>
                void add(const T &value) {
                    if (current == last) {
                        throw std::out_of_range("okser::out::fixed_container: container is full");
                    }
                    *current = value;
                    current++;
                }
        };

        /**
         * A simple output to a C-style char buffer
         *
         * @warning This class does not check if the size of the buffer is enough to hold the serialized data.
         * The responsibility falls on the user to ensure that the serialized data cannot theoretically overflow.
         *
         * ## Example
         * \code
         * char buffer[100];
         * auto output = okser::out::cbuf{buffer};
         *
         * // ...
         *
         * okser::serialize<bundle>(output, 100, 200, 300);
         * size_t length = output.size;
         * \endcode
         */
        class cbuf {
        public:
            char *buf;

            /**
             * The number of bytes appended to the buffer so far
             */
            size_t size = 0;

            void add(char value) {
                buf[size] = value;
                size++;
            }

            void add(uint8_t value) {
                buf[size] = value;
                size++;
            }
        };
    }
}
