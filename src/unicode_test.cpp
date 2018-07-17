#include <gtest/gtest.h>
#include <unicode/schriter.h>
#include <unicode/uchar.h>
#include <unicode/unistr.h>

using namespace icu_55;

TEST(UNICODE, length)
{
    // UnicodeString::length(): The length is the number of char16_t code units are in the UnicodeString. If you want the number of code points, please use countChar32().
    // UnicodeString::countChar32: Count Unicode code points in the length char16_t code units of the string.
    // A code point may occupy either one or two char16_t code units.Counting code points involves reading all code units.

    char utf8_str[] = u8"abcd倚天屠龙1234!@#$❤🀄⌛🈚";
    UnicodeString ucs = UnicodeString::fromUTF8(StringPiece(utf8_str));
    printf("countChar32 = %d  length= %d\n", ucs.countChar32(), ucs.length());

    std::string str = utf8_str;
    const uint8_t *ptr = reinterpret_cast<const uint8_t *>(str.data());
    printf("index\t char \t alpha \t digit \t alnum \t length\t point \t\t utf-8\n");
    for (int i = 0, count = 0, length = str.length(); i < length; ++count)
    {
        int32_t offset = 0;
        UChar32 c = 0;
        U8_NEXT(ptr + i, offset, length, c);

        std::string sub = str.substr(i, offset);
        int w = 0;
        memcpy(&w, sub.data(), offset);

        printf("%2d \t %s \t %d \t %d \t %d \t %d \t %6x \t %x\n", count, sub.c_str(), u_isalpha(c), u_isdigit(c), u_isalnum(c), offset, c, w);
        i += offset;
    }
    // // Traverse code unit instead of code point. I still don't know how to traverse through code point
    // for (int i = 0, length = ucs.length(); i < length; ++i)
    // {
    //     UChar32 c = ucs.char32At(i); // In fact i is the offset of code unit, it is not code point

    //     UnicodeString temp(c);
    //     std::string str;
    //     temp.toUTF8String(str);

    //     printf("%2d: %s \t %d => %d => %d\n", i, str.c_str(), u_isalpha(c), u_isdigit(c), u_isalnum(c));
    // }

    // {
    //     StringCharacterIterator iter(ucs);
    //     int count = 0;
    //     while (iter.hasNext())
    //     {
    //         ++count;
    //         iter.next();
    //     }
    //     std::cout << count << std::endl;
    // }
}

const std::pair<UChar32, int32_t>
getFirstUTF8CodePoint(const std::string &str)
{
    const uint8_t *ptr = reinterpret_cast<const uint8_t *>(str.data());
    const int32_t length = str.length();
    int32_t offset = 0;
    UChar32 cp = 0;
    U8_NEXT(ptr, offset, length, cp);
    return std::make_pair(cp, offset);
}

TEST(UNICODE, firstLetter)
{
    std::string s = u8"倚天屠龙";
    auto cp = getFirstUTF8CodePoint(s);
    std::cout << s << "    starts with char: " << s.substr(0, cp.second) << " " << std::hex << static_cast<unsigned long>(cp.first) << std::endl;
}

TEST(UNICODE, unicode_str)
{
    char utf8_str[] = u8"abcd倚天屠龙1234!@#$";
    UnicodeString ucs = UnicodeString::fromUTF8(StringPiece(utf8_str));
    std::string str;
    ucs.toUTF8String(str);
    std::cout << str << '\n';
}
