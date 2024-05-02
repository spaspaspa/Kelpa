/** 
 * @Path		Kelpa/Src/Utility/Concepts.hpp
 * @Brief		Define some common conceptual constraints
 * @Dependency	None
 * @Since		2024/04/22
 * @Version     1st
 **/

#ifndef __KELPA_UTILITY_CONCEPTS_HPP__
#define __KELPA_UTILITY_CONCEPTS_HPP__

#include <concepts>	
#include <coroutine>
#include <format>
#include <iostream>
#include <sstream>

namespace Kelpa {
namespace Utility {
	
	
template <typename T> concept non_void = 		std::negation_v<std::bool_constant<std::same_as<void, T>>>;
template <typename T> concept non_reference = 	std::negation_v<std::bool_constant<std::is_reference_v<T>>>;
	
template <typename T> concept function_pointer = 
	(	std::is_pointer_v<T> 
	||  std::is_member_pointer_v<T>) && (
     	std::is_function_v<std::decay_t<std::remove_pointer_t<T>>>
    ||	std::is_member_function_pointer_v<std::decay_t<T>>);
    
template	<typename T, typename ... U> concept any_of = 	(std::same_as<T, U> || ...);
template	<typename T, typename ... U> concept all_of = 	(std::same_as<T, U> && ...);
template 	<typename T, typename ... U> concept none_of = 	((!std::same_as<T, U>) && ...);

template <typename T> concept Awaiter = 
(
		std::same_as<decltype(std::declval<T>().await_ready()), bool>
&&  (	
		std::convertible_to<decltype(std::declval<T>().await_suspend(std::coroutine_handle<> {})), std::coroutine_handle<> >
	||	std::convertible_to<decltype(std::declval<T>().await_suspend(std::coroutine_handle<> {})), bool>
	||	std::same_as<decltype(std::declval<T>().await_suspend(std::coroutine_handle<> {})), void> 
	)
&&  	requires { (void) std::declval<T>().await_resume(); }		
);

template <typename T> concept Awaitable = Awaiter<T> || 
( 
		requires { operator co_await(std::declval<T&&>()); } 
	|| 	requires { (void) std::declval<T>().operator co_await(); }
);

template <typename T> concept Displayable = requires { 
	std::formatter<T>::parse; std::formatter<T>::format;
} || requires {
	{ (std::cout << std::declval<T>()) } -> std::convertible_to<std::ostream &>;
};


template <typename T> concept Scalable = requires { 
	{ std::to_string(std::declval<T>()) } -> std::convertible_to<std::string>;	
} || requires { 
	std::declval<std::stringstream>().operator<<(std::declval<T>()); 
} || requires { 
	{ std::declval<T>().to_string() } -> std::convertible_to<std::string>; 
} || requires {
	 std::formatter<T>::parse; std::formatter<T>::format;	
};

template <typename T> concept Hashable = requires { 
	{std::declval<std::hash<T>>()(std::declval<T>())} -> std::convertible_to<std::size_t>; 
}; 

}
}

#endif
