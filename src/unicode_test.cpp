#include <gtest/gtest.h>
#include <unicode/unistr.h>
#include <unicode/uchar.h>

using namespace icu_55;

TEST(UNICODE, length)
{
    char utf8_str[] = u8"abcd倚天屠龙1234!@#$";
    UnicodeString ucs = UnicodeString::fromUTF8(StringPiece(utf8_str));
    printf("countChar32 = %d  length= %d\n", ucs.countChar32(), ucs.length());

    for (int i=0, length=ucs.length(); i<length; ++i)
    {
        UChar32 c = ucs.char32At(i);
        printf("%d => %d => %d => %d\n", i, u_isalpha(c), u_isdigit(c), u_isalnum(c));
    }
}

const std::pair<UChar32, int32_t>
getFirstUTF8CodePoint(const std::string& str) {
    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(str.data());
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
    std::cout << s << "    starts with char: " << s.substr(0, cp.second) << " " << std::hex <<static_cast<unsigned long>(cp.first) << std::endl;
}
