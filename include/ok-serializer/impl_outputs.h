#pragma once

namespace okser {
    namespace out {
        class stdstring {
        public:
            std::reference_wrapper<std::string> str;

            template<typename T>
            void add(const T &value) {
                str.get().append(value);
            }
        };

        template<>
        void stdstring::add(const unsigned char &value) {
            str.get().push_back(value);
        }

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
