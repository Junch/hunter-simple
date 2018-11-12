#include <gtest/gtest.h>

// Use !heap to debug the memory leak
// https://codemany.com/blog/memory-leak-detection-using-windbg/
// https://www.codeproject.com/Articles/31382/Memory-Leak-Detection-Using-WinDbg

class BigData
{
  public:
    BigData() = default;
    char data_[1024 * 1024 * 3];
    virtual ~BigData() = default;
};

TEST(memory, outofmemory)
{
    try
    {
        while (true)
        {
            auto *p = new BigData();
        }
    }
    catch (std::exception &e)
    {
#ifdef _WIN32
        __debugbreak();
#endif
    }
}
