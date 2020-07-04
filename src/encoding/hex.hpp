#pragma once
#include <string>
#include <string_view>

namespace encoding {
    // 编码：HEX
    class hex {
        __attribute__(( __aligned__ (16))) 
        static constexpr char HEX_ENCODE_TABLE[] = "0123456789ABCDEF";
    public:
        // 对制定区域数据进行编码，将结果写入目标位置
        template <class InputIterator, class OutputIterator>
        static OutputIterator encode(InputIterator begin, InputIterator end, OutputIterator to) {
            auto j = to;
            for(auto i=begin; i!=end; ++i) {
                unsigned char c = *i;
                *j = HEX_ENCODE_TABLE[c >> 4]; ++j;
                *j = HEX_ENCODE_TABLE[c & 0x0f]; ++j;
            }
            return j;
        }
        // 解码对应区域字符串数据，并将结果写入目标位置
        template <class InputIterator, class OutputIterator>
        static OutputIterator decode(InputIterator begin, InputIterator end, OutputIterator to) {
            auto j = to;
            for(auto i=begin; i!=end;) {
                // 以下代码参考 PHP 对应 php_hex2bin 的实现略作调整
                // https://github.com/php/php-src/blob/master/ext/standard/string.c
                unsigned char c = static_cast<unsigned char>(*i); ++i;
                unsigned char l = c & ~0x20;
                int is_letter = ((unsigned int) ((l - 'A') ^ (l - 'F' - 1))) >> (8 * sizeof(unsigned int) - 1);
                unsigned char d;
                // 不太明白运算策略：(c >= '0' && c <= '9') || (l >= 'A' && l <= 'F')
                if ((((c ^ '0') - 10) >> (8 * sizeof(unsigned int) - 1)) | is_letter) {
                    d = (l - 0x10 - 0x27 * is_letter) << 4;
                } else {
                    return j;
                }
                c = static_cast<unsigned char>(*i); ++i;
                l = c & ~0x20;
                is_letter = ((unsigned int) ((l - 'A') ^ (l - 'F' - 1))) >> (8 * sizeof(unsigned int) - 1);
                if ((((c ^ '0') - 10) >> (8 * sizeof(unsigned int) - 1)) | is_letter) {
                    d |= l - 0x10 - 0x27 * is_letter;
                } else {
                    return j;
                }
                *j = d; ++j;
            }
            return j;
        }
        // 对字符数据进行编码，返回编码后的结果
        inline static std::string encode(std::string_view raw) {
            std::string o;
            o.reserve(raw.size() * 2);
            // 借助通用实现
            encode(raw.begin(), raw.end(), std::insert_iterator<std::string>(o, o.begin()));
            return o;
        }
        // 对字符数据进行解码，返回结果（若需在源地址空进进行解码，请使用上述 iterator 形式的重载）
        inline static std::string decode(std::string_view hex) {
            std::string o;
            o.reserve(hex.size() / 2);
            // 借助通用实现
            decode(hex.begin(), hex.end(), std::insert_iterator<std::string>(o, o.begin()));
            return o;
        }
    };
}