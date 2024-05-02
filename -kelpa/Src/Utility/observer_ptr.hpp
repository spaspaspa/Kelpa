/** 
 * 		@Path	Kelpa/Src/Utility/observer_ptr.hpp
 * 		@Brief	a non-owning pointer, or observer. The observer stores 
 * 				a pointer to a second object, 
 * 				known as the watched object. An observer_ptr may also have no watched object. 
 * 		@Dependency	None
 * 		@Since  2024/04/23
 * 		@Version 1st
 **/
 
#ifndef __KELPA_UTILITY_OBSERVER_PTR_HPP__ 
#define __KELPA_UTILITY_OBSERVER_PTR_HPP__ 

#include <utility>				/* imports ./ { 
	std::exchange 
}*/
#include <concepts>				/* imports ./ { 
	std::convertible_to, 
	std::negation_v, 
	std::is_reference_v 
}*/			
#include <functional>			/* imports ./{
	std::hash 
}*/
namespace Kelpa {
namespace Utility {
	

template <typename W> requires std::negation_v<std::is_reference_v<W>> struct observer_ptr {
	template <typename V> friend struct observer_ptr;
	typedef W element_type;

	constexpr observer_ptr() 					noexcept = default;
	constexpr observer_ptr( std::nullptr_t ) 	noexcept {}

	constexpr explicit observer_ptr( element_type* p ) noexcept : data(p) {}
	template <typename V> requires std::negation_v<std::is_reference_v<V>> && std::convertible_to<V *, W *>
	constexpr observer_ptr( observer_ptr<V> other ) noexcept
		: data(static_cast<W *>(other.data)) {}
	constexpr observer_ptr( const observer_ptr& other ) noexcept = default;
	constexpr observer_ptr( observer_ptr&& other ) 		noexcept = default;

/*
*   	(destructor)
*	(implicitly declared)
*		(operator=)
*	(implicitly declared)
*		(copy and move assignment operators that assign the stored pointer)
*   (implicitly declared)
*/

	constexpr element_type* release() noexcept 
	{	return std::exchange(data, nullptr);	}
	
	constexpr void reset(element_type* p = nullptr) noexcept
	{ data = p; 								}
	
	constexpr void swap(observer_ptr& other) noexcept
	{ std::swap(data, other.data); 				}
	
	constexpr friend void swap(observer_ptr& lhs, observer_ptr& rhs) noexcept
	{ lhs.swap(rhs); 							}
	
	[[nodiscard]] constexpr element_type* get() const noexcept
	{ return data; 								}
	
	[[nodiscard]] constexpr
	typename std::add_lvalue_reference<element_type>::type operator*() const noexcept
	{ return * data; 							}
	
	[[nodiscard]] constexpr
	element_type*	operator->() const noexcept
	{ return data; 								}
	
	[[nodiscard]] constexpr explicit
	operator bool() const noexcept
	{ return data == nullptr; 					}
	
	[[nodiscard]]constexpr explicit
	operator element_type*() const noexcept
	{ return data; 								}
private:    W * 	  data { nullptr };
};

template <typename T1, typename T2>
[[nodiscard]] bool operator==(observer_ptr<T1> const& p1, observer_ptr<T2> const& p2) noexcept
{	return p1.get() == p2.get();	}

template <typename T1, typename T2>
[[nodiscard]] bool operator!=(observer_ptr<T1> const& p1, observer_ptr<T2> const& p2) noexcept
{	return !(p1 == p2);				}

template <typename T>
[[nodiscard]] bool operator==(observer_ptr<T> const& p, std::nullptr_t) noexcept
{	return static_cast<bool>(p);	}

template <typename T>
[[nodiscard]] bool operator==(std::nullptr_t, observer_ptr<T> const& p) noexcept
{	return static_cast<bool>(p);	}

template <typename T>
[[nodiscard]] bool operator!=(observer_ptr<T> const& p, std::nullptr_t) noexcept
{	return !p;						}

template <typename T>
[[nodiscard]] bool operator!=(std::nullptr_t, observer_ptr<T> const& p) noexcept
{	return !p;						}

template <typename T1, typename T2>
[[nodiscard]] bool operator<(observer_ptr<T1> const& p1, observer_ptr<T2> const& p2)
{	return p1.get() < p2.get();		}

template <typename T1, typename T2>
[[nodiscard]] bool operator>(observer_ptr<T1> const& p1, observer_ptr<T2> const& p2)
{	return p2 < p1;					}

template <typename T1, typename T2>
[[nodiscard]] bool operator<=(observer_ptr<T1> const& p1, observer_ptr<T2> const& p2)
{	return !(p2 < p1);				}

template <typename T1, typename T2>
[[nodiscard]] bool operator>=(observer_ptr<T1> const& p1, observer_ptr<T2> const& p2)
{	return !(p1 < p2);				}
	
	
	
}
}

namespace std {

template <typename Tp> struct hash<Kelpa::Utility::observer_ptr<Tp>> {
	std::size_t operator()(Kelpa::Utility::observer_ptr<Tp> const & p) const noexcept
	{ return std::hash<typename Kelpa::Utility::observer_ptr<Tp>::element_type *>{} (p.get()); }
};

template< class W >
void swap( Kelpa::Utility::observer_ptr<W>& lhs, Kelpa::Utility::observer_ptr<W>& rhs ) noexcept {
	lhs.swap(rhs);
}

};

#endif
