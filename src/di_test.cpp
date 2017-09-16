#include <boost/di.hpp>
#include <gmock/gmock.h>
namespace di = boost::di;

// Example is from
// http://boost-experimental.github.io/di/try_it/index.html

struct iworld {
    virtual ~iworld() noexcept = default;
};

struct world : iworld {
    world() { std::cout << " world!" << std::endl; }
};

struct hello {
    explicit hello(int i) {
        assert(42 == i);
        std::cout << "hello";
    }
};

class Greater;

/// aggregate initialization `example{hello, world}`
template <class T = Greater>
struct example {
    T h;
    iworld& w;
};

TEST(DI, simple) {
    const auto injector = di::make_injector(
        di::bind<iworld>().to<world>(),    // bind interface to implementation
        di::bind<>().to(42),               // bind int to value 42
        di::bind<Greater>().to<hello>()    // bind template to type
    );

    injector.create<example>();
}
