#ifndef OPTIONAL_HXX
#define OPTIONAL_HXX

/* This code tries to follow in some instance the following specification for optional type :
 * http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3793.html
 * Of course, it's also inspired by the reference TS implementation :
 * https://github.com/akrzemi1/Optional 
 * This implementation do not aim to be the same quality, this is only a study exercise.
 * Also based on the cppreference page : http://en.cppreference.com/w/cpp/experimental/optional
 * It also does not bother with compatibility, and assumes a C++14 compiler. These omissions
 * leads to a somewhat cleaner, albeit less compatible, code in some place.
 *
 * In some place, the code do not follow the proposal.
 * Notably, support for initializer list initialization in a natural way. To me, the only reason to
 * disallow the in place construction to happen automatically is, as it's described in the proposal,
 * that it might lead to surprising results along with the implicit boolean conversion.
 * However, I can't see an example in which the initializer list could be problematic, and it's a nice
 * syntactic sugar to have.
 *
 * Please, let me now if there's some misconception, bugs, or just to suggest some improvements :
 * urien.loic.cours@gmail.com
 */ 

#include <cstdint>
#include <stdexcept>
#include <type_traits>

#include <MetaUtils.hxx>

namespace
{
	
template<class T>
struct constexpr_validator
{
	static constexpr T m_{};
};

template<class T>
constexpr std::integral_constant<bool, (T{}, 1)> test_constexpr(int);

template<class T>
constexpr std::false_type test_constexpr(...);

class optional_access_failure : public std::logic_error
{
	using std::logic_error::logic_error;
};


template<class T>
struct is_constexpr_default_constructible_impl : decltype(test_constexpr<T>(0))
{
	//static constexpr bool value = decltype(test_constexpr<T>(0))::value;
};

template<class T>
using is_constexpr_default_constructible = std::integral_constant<bool, std::is_literal_type<T>::value && std::is_default_constructible<T>::value>;

// C++17 style alias.
template<class T>
static bool is_constexpr_default_constructible_v = is_constexpr_default_constructible<T>::value;

// Template metafunction used to modelise the "EqualityComparable" concept
// It assumes that the comparison is a well formed relation, that said, it is well founded, as it should be, and antisymetric.

template<class T1, class T2, class = void>
struct are_equal_comparable : std::false_type
{};

template<class T1, class T2>
struct are_equal_comparable<T1, T2, Meta::void_t<decltype(std::declval<T1>() == std::declval<T2>())>> : std::true_type
{};

// C++17 style alias
template<class T1, class T2>
static bool are_equal_comparable_v = are_equal_comparable<T1, T2>::value;

template<class T1, class T2, class = void>
struct are_less_than_comparable : std::false_type
{};

template<class T1, class T2>
struct are_less_than_comparable<T1, T2, Meta::void_t<decltype(std::declval<T1>() < std::declval<T2>())>> : std::true_type
{};

// Required to check if a type as overloaded operator&.
// If it does, we can't just use the "&" method to get back the address.
template<class T, class = void>
struct has_overloaded_address_operator : std::false_type
{};

template<class T>
struct has_overloaded_address_operator<T, Meta::void_t<decltype(std::declval<T>().operator&())>> : std::true_type
{};

// C++17 style alias.
template<class T>
static constexpr bool has_overloaded_address_operator_v = has_overloaded_address_operator<T>::value;

template<class T1, class T2>
using are_equal_and_less_than_comparable = std::integral_constant<bool, are_equal_comparable<T1, T2>::value && are_less_than_comparable<T1, T2>::value>;

// C++17 style alias.
template<class T1, class T2>
static constexpr bool are_equal_and_less_thant_comparable_v = are_equal_and_less_than_comparable<T1, T2>::value;

// So here are the addressof functions we will be using.
// Greatly inspired by the original functions in the reference implementation.
template<class T, std::enable_if_t<!has_overloaded_address_operator<T>::value>* = nullptr>
constexpr T* static_addressof(T& ref)
{
	return &ref;
}

template<class T, std::enable_if_t<has_overloaded_address_operator<T>::value>* = nullptr>
T* static_addressof(T& ref)
{
	return std::addressof(ref);
}

}

struct nullopt_t
{
	public:
	constexpr nullopt_t() = default;
	nullopt_t(const nullopt_t&) = default;
	nullopt_t(nullopt_t&&) = default;
	nullopt_t& operator=(const nullopt_t&) = delete;
	nullopt_t& operator=(nullopt_t&&) = delete;
};

constexpr nullopt_t nullopt{};


struct in_place_t
{
	public:
	constexpr in_place_t() = default;
	in_place_t(const in_place_t&) = default;
	in_place_t(in_place_t&&) = default;
	in_place_t& operator=(const in_place_t&) = delete;
	in_place_t& operator=(in_place_t&&) = delete;
};

constexpr in_place_t in_place{};

// The "dummy_" field is here to make a false initialization when the optional is disengaged,
// because we don't want to initialize the true object.

// That's quite a mess, we don't want anybody to see that ...
namespace
{
	
	template<class T, class = void>
	union basic_constexpr_storage_layout
	{
		private:
		uint8_t dummy_;
		
		public:
		T value_;
		
		public:
		constexpr basic_constexpr_storage_layout() noexcept 
		: dummy_{}
		{}
		
		constexpr basic_constexpr_storage_layout(const T& other) noexcept(std::is_nothrow_copy_constructible<T>::value)
		: value_{other}
		{}
		
		constexpr basic_constexpr_storage_layout(T&& other) noexcept(std::is_nothrow_move_constructible<T>::value)
		: value_{std::move(other)}
		{}
		
		~basic_constexpr_storage_layout(){}
	};
	
	template<class T>
	union basic_constexpr_storage_layout<T, Meta::void_t<std::enable_if_t<std::is_trivially_destructible<T>::value>>>
	{
		private:
		uint8_t dummy_;
		
		public:
		T value_;
		
		public:
		constexpr basic_constexpr_storage_layout() noexcept 
		: dummy_{}
		{}
		
		constexpr basic_constexpr_storage_layout(const T& other) noexcept(std::is_nothrow_copy_constructible<T>::value)
		: value_{other}
		{}
		
		constexpr basic_constexpr_storage_layout(T&& other) noexcept(std::is_nothrow_move_constructible<T>::value)
		: value_{std::move(other)}
		{}
	
		~basic_constexpr_storage_layout() = default;
	};

// Now let's build our real storage class
/*template<class T>
using basic_storage_layout = std::conditional_t<!std::is_default_constructible<T>::value,
												basic_storage_layout_no_default<T>,
												std::conditional_t<std::is_literal_type<T>::value,
																   basic_constexpr_storage_layout_default<T>,
																   basic_storage_layout_default<T>>>;*/
															
// Dummy alias.																   
template<class T>
using basic_storage_layout = basic_constexpr_storage_layout<T>;

// All constructors marked explicit becaue we don't want them to be converting constructors.
// So indicating here that we do not want is is always good to prevent errors.

// This class is for the case where our T type is not a literal type.
// Thus, the storage can't be a literal type, and we don't need constexpr constructors.
// Moreover, calling a non constexpr constructor inside a constexpr constructor can actually
// lead to subtle error messages about "constructor which will never produce constant expression".
// TODO : Hide this in an anonymous namespace.
template<class T, class = void>
struct basic_storage
{
	public:
	bool initialized_;
	basic_storage_layout<T> storage_;

	public:
	basic_storage() noexcept(std::is_nothrow_default_constructible<T>::value) 
	: initialized_{false},
	  storage_{}
	{}

	explicit basic_storage(const T& other) noexcept(std::is_nothrow_copy_constructible<T>::value)
	: initialized_{true},
	  storage_{other}
	{}
	
	explicit basic_storage(T&& other) noexcept(std::is_nothrow_move_constructible<T>::value)
	: initialized_{true},
	  storage_{std::move(other)}
	{}

	template<class ... TArgs, std::enable_if_t<std::is_constructible<T, TArgs...>::value>* = nullptr>
	explicit basic_storage(TArgs&&... args) noexcept(noexcept(T{std::forward<TArgs>(args)...}))
	: initialized_{true},
	  storage_{std::forward<TArgs>(args)...}
	{}
	
	template<class U, class ... TArgs, std::enable_if_t<std::is_constructible<T, std::initializer_list<U>, TArgs...>::value>* = nullptr>
	explicit constexpr basic_storage(std::initializer_list<U> list, TArgs&&... args) noexcept(noexcept(T{list, std::forward<TArgs>(args)...}))
	: initialized_{true},
	  storage_{list, std::forward<TArgs>(args)...}
	{}
	
	
	basic_storage& operator=(const T& other) noexcept(std::is_nothrow_copy_assignable<T>::value)
	{
		if(!initialized_)
		{
			new(&storage_.value_) T(other);
			initialized_ = true;
		}
		else
		{
			storage_.value_ = other;
		}
		
		return *this;
	}
	
	basic_storage& operator=(T&& other) noexcept(std::is_nothrow_move_assignable<T>::value)
	{
		if(!initialized_)
		{
			new(&storage_.value_) T(std::move(other));
			initialized_ = true;
		}
		else
		{
			storage_.value_ = std::move(other);
		}
		
		return *this;
	}
	
	~basic_storage() noexcept
	{
		if(initialized_)
		{
			storage_.value_.~T();
		}
	}
};

// If our T type is a literal type, then we can use constexpr constructors.
template<class T>
struct basic_storage<T, Meta::void_t<std::enable_if_t<std::is_trivially_destructible<T>::value>>>
{
	public:
	bool initialized_;
	basic_storage_layout<T> storage_;

	public:
	constexpr basic_storage() noexcept(std::is_nothrow_default_constructible<T>::value)
	: initialized_{false},
	  storage_{}
	{}
	
	explicit constexpr basic_storage(const T& other) noexcept(std::is_nothrow_copy_constructible<T>::value)
	: initialized_{true},
	  storage_{other}
	{}
	
	explicit constexpr basic_storage(T&& other) noexcept(std::is_nothrow_move_constructible<T>::value) 
	: initialized_{true},
	  storage_{std::move(other)}
	{}

	template<class ... TArgs, std::enable_if_t<std::is_constructible<T, TArgs...>::value>* = nullptr>
	explicit constexpr basic_storage(TArgs&&... args) noexcept(noexcept(T{std::forward<TArgs>(args)...}))
	: initialized_{true},
	  storage_{std::forward<TArgs>(args)...}
	{}
	
	template<class U, class ... TArgs, std::enable_if_t<std::is_constructible<T, std::initializer_list<U>, TArgs...>::value>* = nullptr>
	explicit constexpr basic_storage(std::initializer_list<U> list, TArgs&&... args) noexcept(noexcept(T{list, std::forward<TArgs>(args)...}))
	: initialized_{true},
	  storage_{list, std::forward<TArgs>(args)...}
	{}
	
	constexpr basic_storage& operator=(const T& other) noexcept(std::is_nothrow_copy_assignable<T>::value)
	{
		if(!initialized_)
		{
			new(&storage_.value_) T(other);
			initialized_ = true;
		}
		else
		{
			storage_.value_ = other;
		}
		
		return *this;
	}
	
	constexpr basic_storage& operator=(T&& other) noexcept(std::is_nothrow_move_assignable<T>::value)
	{
		if(!initialized_)
		{
			new(&storage_.value_) T(std::move(other));
			initialized_ = true;
		}
		else
		{
			storage_.value_ = std::move(other);
		}
		
		return *this;
	}
	
	~basic_storage() noexcept = default;
};

}

template<class T>
class optional : basic_storage<T>
{
	using Base = basic_storage<T>;
	
	private : 
	static_assert(!std::is_same<nullopt_t, T>::value, "Optional type is ill formed, cannot use a null option type as underlying type !");

	public:
	constexpr optional() = default;
	explicit constexpr optional(nullopt_t) noexcept {};
	
	constexpr optional(const optional& other) noexcept(std::is_nothrow_copy_constructible<T>::value)
	: Base{other.storage_.value_}
	{
	    if(!other.initialized())
	    {
	        this->initialized_ = false;
	    }
		/*if(other.initialized())
		{
			*static_cast<Base*>(this) = other.storage_.value_;
		}*/
	}
	
	constexpr optional(optional&& other) noexcept(std::is_nothrow_move_constructible<T>::value)
	: Base{std::move(other.storage_.value_)}
	{
	    if(!other.initialized())
	    {
	        this->initialized_ = false;
	    }
		/*if(other.initialized())
		{
			*static_cast<Base*>(this) = 
		}*/
		other.clear();
	}
	
	constexpr optional(const T& value) noexcept(std::is_nothrow_copy_constructible<T>::value)
	: Base(value)
	{}
	
	constexpr optional(T&& value) noexcept(std::is_nothrow_move_constructible<T>::value)
	: Base(std::move(value))
	{}
	
	template<class ... TArgs>
	constexpr explicit optional(in_place_t, TArgs&& ... args)
	: Base(std::forward<TArgs>(args)...)
	{}
	
	template<class U, std::enable_if_t<std::is_constructible<T, std::initializer_list<U>>::value>* = nullptr>
	constexpr optional(std::initializer_list<U> list) noexcept(noexcept(T{list}))
	: Base(list)
	{}
	
	template<class U, class ... TArgs, std::enable_if_t<std::is_constructible<T, std::initializer_list<U>, TArgs...>::value>* = nullptr>
	constexpr optional(std::initializer_list<U> list, TArgs&&... args) noexcept(noexcept(T{list, std::forward<TArgs>(args)...}))
	: Base(list, std::forward<TArgs>(args)...)
	{}
	
	~optional() noexcept = default;
	
	optional& operator=(const optional& other) noexcept(std::is_nothrow_copy_constructible<T>::value && std::is_nothrow_copy_assignable<T>::value)
	{
		optional tmp{other};
		this->swap(tmp);
		
		return *this;
	}
	
	optional& operator=(optional&& other) noexcept(std::is_nothrow_move_assignable<T>::value)
	{
		this->swap(other);
		
		return *this;
	}
	
	/* The two following constructors are using template to disable the overload mechanism.
	*  If we were to write for the nullopt something like this :
	*  operator& operator=(nullopt_t){...}
	*  A line like this one :
	*  optional<int> opt;
	*  opt = {};
	*  Would yield an ambiguous overload error, because of the overload taking optional<T> rvalue overload
	*/
	template<class U>
	auto operator=(U) noexcept
	-> std::enable_if_t<std::is_same<U, nullopt_t>::value, optional&>
	{
		clear();
		
		return *this;
	}
	
	template<class U>
	auto operator=(U&& value) noexcept(std::is_nothrow_copy_assignable<T>::value)
	-> std::enable_if_t<std::is_same<std::decay_t<U>, T>::value, optional&>
	{
		*static_cast<Base*>(this) = std::forward<U>(value);
		
		return *this;
	}
	
	template<class ... TArgs>
	auto emplace(TArgs&& ... args) noexcept(std::is_nothrow_copy_constructible<T>::value)
	-> std::enable_if_t<std::is_constructible<T, TArgs...>::value, void>
	{
		new(&Base::storage_.value_) T{std::forward<TArgs>(args)...};
	}
	
	template<class AltRetType>
	constexpr auto value_or(AltRetType&& default_value) noexcept
	-> std::enable_if_t<std::is_convertible<AltRetType, T>::value, T>
	{
		return initialized() ? Base::storage_.value_ : static_cast<T>(default_value);
	}
	
	constexpr operator bool() const noexcept
	{
		return initialized();
	}
	
	// Should think about how to make in place construction.
	// Converting constructor is out of question !
	
	// Noexcept because the destructor should not throw any exception anyway.
	void clear() noexcept
	{
		if(initialized())
		{
			Base::storage_.value_.T::~T();
		}
		Base::initialized_ = false;
	}
	
	constexpr bool initialized() const noexcept
	{
		return Base::initialized_;
	}
	
	/* Initially, the noexcept specification was the following :
	*  noexcept(std::is_nothrow_copy_assignable<T>::value && std::is_nothrow_move_assignable<T>::value)
	*  but a swap function should never throw anyway, or algorithms would be compromised.
	*  So, we make the maybe false promise here, that it will not throw, and check by a static assert
	*/
	void swap(optional<T>& other) noexcept 
	{
		// Do we really want that ? However, if the type swap is throwing, the std algorithms could fail,
		// so it might be wise anyway.
		//static_assert(std::is_nothrow_copy_assignable<T>::value && std::is_nothrow_move_assignable<T>::value,
		//			  "Throwing swap function !");
					  
		if(!initialized() && other.initialized())
		{
			*static_cast<Base*>(this) = other.storage_.value_;
			other.clear();
		}
		else if(initialized() && !other.initialized())
		{
			static_cast<Base&>(other) = std::move(this->storage_.value_);
			clear();
		}
		else if(initialized() && other.initialized())
		{
			// Enabling correct ADL.
			using std::swap;
			swap(Base::storage_.value_, other.storage_.value_);
		}
	}
	
	void ensure_initialized() const
	{
		if(!initialized())
		{
			throw optional_access_failure("Accessed uninitialized optional value");
		}
	}
	
	constexpr T& value() &
	{
		ensure_initialized();
		return Base::storage_.value_;
	}
	
	constexpr const T& value() const&
	{
		ensure_initialized();
		return Base::storage_.value_;
	}
	
	// Constexpr here ?!? Ref qualifier say "rvalue reference", and literal type cannot be moved
	// Some use for this additional notation then ?
	constexpr T&& value() &&
	{
		ensure_initialized();
		return std::move(Base::storage_.value_);
	}
	
	constexpr const T* operator->() const
	{
		ensure_initialized();
		return static_addressof(Base::storage_.value_);
	}
	
	constexpr T* operator->()
	{
		ensure_initialized();
		return static_addressof(Base::storage_.value_);
	}
	
	constexpr const T& operator*() const& noexcept
	{
		return value();
	}
	
	constexpr T& operator*() & noexcept
	{
		return value();
	}
	
	constexpr T&& operator*() && noexcept
	{
		return std::move(value());
	}
};

template<class T>
class optional<T&>
{
	static_assert(!std::is_same<T, nullopt_t>::value, "Optional type is ill formed, cannot use a null option type as underlying type !");

	private:
	// Naked pointer to simulate the reference, but allowing modification.
	T* ref;
	
	public:
	constexpr optional() noexcept
	: ref{nullptr}
	{}
	constexpr optional(nullopt_t) noexcept
	: ref{nullptr}
	{}
	
	constexpr optional(T&& other) noexcept = delete;
	
	constexpr optional(const optional& other) noexcept
	: ref{other.ref}
	{}
	
	constexpr optional(T& other) noexcept
	: ref{static_addressof(other)}
	{}
	
	~optional() noexcept = default;
	
	template<class U>
	auto operator=(U) noexcept
	-> std::enable_if_t<std::is_same<U, nullopt_t>::value, optional&>
	{
		ref = nullptr;
		
		return *this;
	}
	
	template<class U>
	auto operator=(U&& refVal) noexcept
	-> std::enable_if_t<std::is_same<std::decay_t<U>, T>::value, optional&>
	{
		ref = static_addressof(refVal);
		
		return *this;
	}
	
	optional& operator=(const optional& other) noexcept
	{
		ref = other.ref;
		
		return *this;
	}
	
	void emplace(T& refVal) noexcept
	{
		ref = static_addressof(refVal);
	}
	
	constexpr T& value()
	{
		if(initialized())
		{
			return *ref;
		}
		throw optional_access_failure("Accessed uninitialized optional value");
	}
	
	constexpr const T& value() const
	{
		return value();
	}
	
	constexpr const T* operator->() const noexcept
	{
		return ref;
	}
	
	constexpr T* operator->() noexcept
	{
		return ref;
	}
	
	constexpr const T& operator*() const
	{
		return value();
	}
	
	constexpr T& operator*()
	{
		return value();
	}
	
	void clear() noexcept
	{
		ref = nullptr;
	}
	
	constexpr bool initialized() noexcept
	{
		return ref != nullptr;
	}
	
	constexpr operator bool() noexcept
	{
		return initialized();
	}
	
	void swap(optional& other) noexcept
	{
		using std::swap;
		swap(ref, other.ref);
	}
	
	template<class AltRetType>
	constexpr auto value_or(AltRetType& default_value) noexcept
	-> std::enable_if_t<std::is_same<AltRetType&, T&>::value, T&>
	{
		return initialized() ? value() : static_cast<T&>(default_value);
	}
};

template<class T>
void swap(optional<T>& lhs, optional<T>& rhs)
{
	lhs.swap(rhs);
}

/* This function is a bit contrived due to the one line.
*  As the goal of this implementation is to be clear, here is the version with classical if statements
*  
*   if(bool{lhs} == bool{rhs})
*	{
*		if(bool{lhs}) return lhs == rhs;
*		else 		  return true;
*	}
*   else
*	{
*		return false;
*	}
*/
// Concept : EqualityComparable
template<class T1, class T2, std::enable_if_t<are_less_than_comparable<T1, T2>::value>* = nullptr>
constexpr bool operator==(const optional<T1>& lhs, const optional<T2>& rhs)
{
	return (bool{lhs} != bool{rhs}) ? false : (bool{lhs} ? *lhs == *rhs : true);
}

template<class T>
constexpr bool operator==(const optional<T>& lhs, nullopt_t)
{
	return !bool{lhs};
}

template<class T>
constexpr bool operator==(nullopt_t, const optional<T>& rhs)
{
	return !bool{rhs};
}

template<class T1, class T2>
constexpr bool operator!=(const optional<T1>& lhs, const optional<T2>& rhs)
{
	return !(lhs == rhs);
}

template<class T>
constexpr bool operator!=(const optional<T>& lhs, nullopt_t)
{
	return !(lhs == nullopt);
}

template<class T>
constexpr bool operator!=(nullopt_t, const optional<T>& rhs)
{
	return !(rhs == nullopt);
}

// Concept : LessThanComparable
template<class T1, class T2, std::enable_if_t<are_less_than_comparable<T1, T2>::value>* = nullptr>
constexpr bool operator<(const optional<T1>& lhs, const optional<T2>& rhs)
{
	return !bool{rhs} ? false : (!bool{lhs} ? true : *lhs < *rhs); 
}

template<class T>
constexpr bool operator<(const optional<T>&, nullopt_t)
{
	return false;
}

template<class T>
constexpr bool operator<(nullopt_t, const optional<T>& rhs)
{
	return bool{rhs};
}

template<class T1, class T2>
constexpr bool operator>(const optional<T1>& lhs, const optional<T2>& rhs)
{
	return rhs < lhs;
}

template<class T>
constexpr bool operator>(const optional<T>& lhs, nullopt_t)
{
	return nullopt < lhs;
}

template<class T>
constexpr bool operator>(nullopt_t, const optional<T>& rhs)
{
	return rhs < nullopt;
}

template<class T1, class T2>
constexpr bool operator<=(const optional<T1>& lhs, const optional<T2>& rhs)
{
	return !(lhs > rhs);
}

template<class T>
constexpr bool operator<=(const optional<T>& lhs, nullopt_t)
{
	return lhs == nullopt;
}

template<class T>
constexpr bool operator<=(nullopt_t, const optional<T>&)
{
	return true;
}

template<class T1, class T2>
constexpr bool operator>=(const optional<T1>& lhs, const optional<T2>& rhs)
{
	return !(lhs < rhs);
}

template<class T>
constexpr bool operator>=(const optional<T>& lhs, nullopt_t)
{
	return nullopt <= lhs;
}

template<class T>
constexpr bool operator>=(nullopt_t, const optional<T>& rhs)
{
	return rhs <= nullopt;
}

template<class T>
constexpr optional<std::decay_t<T>> make_optional(T&& value)
{
	return {std::forward<T>(value)};
}

#endif // OPTIONAL_HXX
