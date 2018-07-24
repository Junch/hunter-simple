#include <gtest/gtest.h>
#include <gmock/gmock.h>
#define U_STATIC_IMPLEMENTATION
#include <unicode/uchar.h>
#include <unicode/unistr.h>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace icu_55;

/*
   https://www.zhihu.com/question/55601459 by vczh
   C++11出了几个针对unicode的字符类型，你可以用，就是你还得注意你的C++源文件的格式
  （如果你一定要在代码里面写中文）。你有可能把C++源文件的格式保存成UTF-8，但是Windows
   不存在UTF-8的codepage，所以你不加BOM就GG。鉴于clang++是支持BOM的，所以你应该永远
   使用BOM。
  */

TEST(UNICODE, length)
{
    // UnicodeString::length(): The length is the number of char16_t code units
    // are in the UnicodeString. If you want the number of code points, please
    // use countChar32(). UnicodeString::countChar32: Count Unicode code points
    // in the length char16_t code units of the string. A code point may occupy
    // either one or two char16_t code units.Counting code points involves
    // reading all code units.

    char utf8_str[] = u8"abcd倚天屠龙1234!@#$❤🀄⌛🈚";
    UnicodeString ucs = UnicodeString::fromUTF8(StringPiece(utf8_str));
    printf("countChar32 = %d  length= %d\n", ucs.countChar32(), ucs.length());

    std::string str = utf8_str;
    const uint8_t *ptr = reinterpret_cast<const uint8_t *>(str.data());
    printf("index\t char \t alpha \t digit \t alnum \t length\t point \t\t "
           "utf-8\n");
    for (int i = 0, count = 0, length = str.length(); i < length; ++count)
    {
        int32_t offset = 0;
        UChar32 c = 0;
        U8_NEXT(ptr + i, offset, length, c);

        std::string sub = str.substr(i, offset);
        int w = 0;
        memcpy(&w, sub.data(), offset);

        printf("%2d \t %s \t %d \t %d \t %d \t %d \t %6x \t %x\n", count, sub.c_str(), u_isalpha(c),
               u_isdigit(c), u_isalnum(c), offset, c, w);
        i += offset;
    }
    // // Traverse code unit instead of code point. I still don't know how to
    // traverse through code point for (int i = 0, length = ucs.length(); i <
    // length; ++i)
    // {
    //     UChar32 c = ucs.char32At(i); // In fact i is the offset of code unit,
    //     it is not code point

    //     UnicodeString temp(c);
    //     std::string str;
    //     temp.toUTF8String(str);

    //     printf("%2d: %s \t %d => %d => %d\n", i, str.c_str(), u_isalpha(c),
    //     u_isdigit(c), u_isalnum(c));
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

const std::pair<UChar32, int32_t> getFirstUTF8CodePoint(const std::string &str)
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
    std::cout << s << "    starts with char: " << s.substr(0, cp.second) << " " << std::hex
              << static_cast<unsigned long>(cp.first) << std::endl;
}

#ifdef _WIN32
// https://gist.github.com/Junch/672870ed53cf1465832ae89cf0c89c20

std::string utf8_encode(const std::wstring &wstr)
{
    int size_needed =
        WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

// Convert an UTF8 string to a wide Unicode String
std::wstring utf8_decode(const std::string &str)
{
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}
#endif

TEST(UNICODE, unicode_str)
{
    char utf8_str[] = u8"abcd倚天屠龙1234!@#$❤🀄⌛🈚";
    UnicodeString ucs = UnicodeString::fromUTF8(StringPiece(utf8_str));
    std::string str;
    ucs.toUTF8String(str);
    std::cout << str << '\n';

#ifdef _WIN32
    std::wstring title = L"中文";
    std::wstring content = utf8_decode(str);

    ::MessageBoxW(NULL, content.c_str(), title.c_str(), MB_OK);
#endif
}

std::string getInitials(const std::string &displayName)
{
    std::string initials;
    bool append = true;

    const uint8_t *ptr = reinterpret_cast<const uint8_t *>(displayName.data());
    for (int i = 0, count = 0, length = displayName.length(); i < length; ++count)
    {
        int32_t offset = 0;
        UChar32 c = 0;
        U8_NEXT(ptr + i, offset, length, c);
        if (c == ' ')
        {
            append = true;
        }
        else if (!u_isalnum(c))
        {
            break;
        }
        else if (append)
        {
            std::string sub = displayName.substr(i, offset);
            initials.append(sub);
            append = false;
        }

        i += offset;
    }

    return initials;
}

class InitialsUtilsUnitTest : public testing::TestWithParam<std::pair<std::string, std::string>>
{
  public:
    void SetUp() {}
    void TearDown() {}

    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
};

// clang-format off
INSTANTIATE_TEST_CASE_P
(
    UNICODE, InitialsUtilsUnitTest, ::testing::Values
    (
        std::pair<std::string, std::string>("", ""),
        std::pair<std::string, std::string>("李", "李"),
        std::pair<std::string, std::string>("李 白", "李白"),
        std::pair<std::string, std::string>("白 居易", "白居"),
        std::pair<std::string, std::string>("八大-山人", "八"),
        std::pair<std::string, std::string>("李 白 Li", "李白L"),
        std::pair<std::string, std::string>("李 白 (唐代诗人)", "李白"),
        std::pair<std::string, std::string>("李白  唐代诗人", "李唐"),
        std::pair<std::string, std::string>("李 🈚白", "李")
	)
);
// clang-format on

TEST_P(InitialsUtilsUnitTest, getInitials)
{
    ASSERT_THAT(getInitials(GetParam().first), ::testing::StrEq(GetParam().second));
}
