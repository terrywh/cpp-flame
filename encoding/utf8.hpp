#ifndef WCCS_ENCODING_UTF8_H
#define WCCS_ENCODING_UTF8_H
#include <cstdint>
#include <string>
#include <locale>

namespace wccs::encoding {
    
#define ONEMASK ((size_t)(-1) / 0xFF)

    class utf8 {
    public:
        utf8(const std::locale& locale = std::locale("zh_CN.UTF-8")) 
        : locale_(locale)
        , facet_(std::use_facet<std::codecvt<char16_t, char, mbstate_t>>(locale_)) {

        }
        // 以 UTF8 编码计算字符个数
        std::size_t length(const std::string& text) {
            return cp_strlen_utf8(text.c_str());
        }
        // 以 UTF8 编码截取偏移 offset 个字符长度 length 个字符
        std::string substr(const std::string& text, std::size_t offset, std::size_t length) {
            std::mbstate_t mb {};
            const char* begin = text.data(),
                *end = text.data() + text.size();
            // 截取长度起点终点
            if(offset > 0) {
                offset = facet_.length(mb, begin, end, offset);
            }
            if(length > 0) {
                length = facet_.length(mb, begin + offset, end, length);
            }
            return text.substr(offset, length);
        }
    private:
        std::locale locale_;
        const std::codecvt<char16_t, char, std::mbstate_t> &facet_;
        // 来源:
        // http://www.daemonology.net/blog/2008-06-05-faster-utf8-strlen.html
        static size_t cp_strlen_utf8(const char * _s) {
            const char * s;
            size_t count = 0;
            size_t u;
            unsigned char b;

            /* Handle any initial misaligned bytes. */
            for (s = _s; (uintptr_t)(s) & (sizeof(size_t) - 1); s++) {
                b = *s;

                /* Exit if we hit a zero byte. */
                if (b == '\0')
                    goto done;

                /* Is this byte NOT the first byte of a character? */
                count += (b >> 7) & ((~b) >> 6);
            }

            /* Handle complete blocks. */
            for (; ; s += sizeof(size_t)) {
                /* Prefetch 256 bytes ahead. */
                __builtin_prefetch(&s[256], 0, 0);

                /* Grab 4 or 8 bytes of UTF-8 data. */
                u = *(size_t *)(s);

                /* Exit the loop if there are any zero bytes. */
                if ((u - ONEMASK) & (~u) & (ONEMASK * 0x80))
                    break;

                /* Count bytes which are NOT the first byte of a character. */
                u = ((u & (ONEMASK * 0x80)) >> 7) & ((~u) >> 6);
                count += (u * ONEMASK) >> ((sizeof(size_t) - 1) * 8);
            }

            /* Take care of any left-over bytes. */
            for (; ; s++) {
                b = *s;

                /* Exit if we hit a zero byte. */
                if (b == '\0')
                    break;

                /* Is this byte NOT the first byte of a character? */
                count += (b >> 7) & ((~b) >> 6);
            }

        done:
            return ((s - _s) - count);
        }
    };
}

#endif // WCCS_ENCODING_UTF8_H