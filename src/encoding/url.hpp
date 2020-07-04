#pragma once
#include <string>
#include <string_view>

namespace encoding {
    class url {
        // 编码转换表
        __attribute__(( __aligned__ (16)))
        static constexpr char HEX_CONVERT_TABLE[] = "0123456789ABCDEF";
    public:
        // 将范围内的文本进行 PERCENT 编码处理，输出至指定位置
        template <class InputIterator, class OutputIterator>
        static OutputIterator encode(InputIterator begin, InputIterator end, OutputIterator to) {
            auto j = to;
            for(auto i = begin; i!=end; ++i) {
                unsigned char c = *i;
                if(std::isalnum(c) || c=='-' || c== '.' || c=='_')
                    *j = c;
                else {
                    *j = '%';
                    ++j;
                    *j = HEX_CONVERT_TABLE[c >> 4];
                    ++j;
                    *j = HEX_CONVERT_TABLE[c & 0x0f];
                }
                ++j;
            }
            return j;
        }
        static unsigned char htoi(unsigned char s1, unsigned char s2) {
            unsigned char value;

            if (isupper(s1)) s1 = tolower(s1);
            value = (s1 >= '0' && s1 <= '9' ? s1 - '0' : s1 - 'a' + 10) << 4;

            if (isupper(s2)) s2 = tolower(s2);
            value |= s2 >= '0' && s2 <= '9' ? s2 - '0' : s2 - 'a' + 10;

            return value;
        }
        // 对范围内的文本进行 PERCENT 解码处理，输出至指定位置
        template <class InputIterator, class OutputIterator>
        static OutputIterator decode(InputIterator begin, InputIterator end, OutputIterator to) {
            auto j = to;
            for(auto i=begin; i!=end; ++i) {
                if(*i == '+') *j = ' ';
                else if(*i == '%' && i+1 != end && i+2 != end && std::isxdigit(static_cast<int>(*(i+1))) && std::isxdigit(static_cast<int>(*(i+2)))) {
                    *j = static_cast<char>(htoi(*(i+1), *(i+2)));
                    ++i;
                    ++i;
                }
                else *j = *i;
                ++j;
            }

            return j;
        }
        // 对字符串进行 PERCENT 编码
        inline static std::string encode(std::string_view raw) {
            std::string o;
            o.reserve(raw.size() * 2);
            encode(raw.begin(), raw.end(), 
                std::insert_iterator<std::string>(o, o.begin()));
            return o;
        }
        // 对字符串进行 PERCENT 解码（复制，若需就地转换，请使用上面 iterator 对应重载）
        inline static std::string decode(std::string_view enc) {
            std::string o;
            o.reserve(enc.size() / 2);
            decode(enc.begin(), enc.end(),
                std::insert_iterator<std::string>(o, o.begin()));
            return o;
        }
    };
}
