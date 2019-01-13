#ifndef META_UTILS_HXX
#define META_UTILS_HXX

#include <Configuration.hxx>

// Some metaprogramming utilities, reduced to the bare minimum for the library.

namespace Meta
{
template<class ...>
using void_t = void;


template<class T, class Iterator, class = void>
struct is_unchecked_iterator_of
: std::false_type
{};

template<class T, class Iterator>
struct is_unchecked_iterator_of<T, Iterator, void> : std::conditional_t<
								  	std::is_same<typename T::unchecked_iterator, Iterator>::value
								||	std::is_same<typename T::unchecked_const_iterator, Iterator>::value
								||  std::is_same<typename T::unchecked_reverse_iterator, Iterator>::value
								||  std::is_same<typename T::unchecked_const_reverse_iterator, Iterator>::value,
									std::true_type, std::false_type>
{};
	
template<class T, class Iterator, class = void>
struct is_iterator_of
: std::false_type
{};

template<class T, class Iterator>
struct is_iterator_of<T, Iterator, void> : std::conditional_t<
								  	std::is_same<typename T::iterator, Iterator>::value
								||	std::is_same<typename T::const_iterator, Iterator>::value
								||  std::is_same<typename T::reverse_iterator, Iterator>::value
								||  std::is_same<typename T::const_reverse_iterator, Iterator>::value
								||  is_unchecked_iterator_of<T, Iterator>::value,
									std::true_type, std::false_type>
{};

template<size_type ... N>
using index_sequence = std::integer_sequence<size_type, N...>;

template<size_type N>
using make_index_sequence = std::make_integer_sequence<size_type, N>;

}

#endif // META_UTILS_HXX
