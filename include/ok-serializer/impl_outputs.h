#pragma once

#include <functional>
#include <string>

namespace okser {
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
