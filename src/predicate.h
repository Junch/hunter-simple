#include <functional>
#include <memory>

class predicate
{
  public:
    predicate(std::function<bool()> fxn, unsigned int timeoutMs);
    bool evaluate();

  private:
    std::function<bool()> _predicateFxn;
    unsigned int _timeoutMs;
};
