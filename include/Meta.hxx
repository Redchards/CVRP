#ifndef META_HXX
#define META_HXX

#include <functional>

namespace Meta
{

template<class T>
using invoke = typename T::type;

template<class T>
struct always
{
    template<class ...>
    using type = T;  
};

template<class ... Args>
using void_t = typename always<void>::type<Args...>;

template<class ... Args>
using true_t = typename always<std::true_type>::type<Args...>;

template<class ... Args>
using false_t = typename always<std::false_type>::type<Args...>;

}

#endif // META_HXX