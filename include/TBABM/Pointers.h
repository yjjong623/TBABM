#include <memory>

template <typename T>
using shared_p = std::shared_ptr<T>;

template <typename T>
using weak_p = std::weak_ptr<T>;
