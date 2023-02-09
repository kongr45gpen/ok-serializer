#pragma once

namespace okser {
    namespace out {
        class stdstring {
        public:
            std::reference_wrapper<std::string> str;

            template<typename T>
            void add(const T &value) {
                str += value;
            }
        };

        class cstring {
        public:
            char *buf;

            void add(char value) {
                *buf = value;
                buf++;
            }

            void add(uint8_t value) {
                *buf = value;
                buf++;
            }
        };
    }
}
