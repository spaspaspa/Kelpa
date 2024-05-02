/** 
 * 		@Path	Kelpa/Src/Utility/UniqueAny.hpp
 * 		@Brief	Generalized RAII lifecycle management class types
 * 		@Dependency	None
 * 		@Since  2024/04/23
 * 		@Version 1st
 **/
 
#ifndef __KELPA_UTILITY_UNIQUEANY_HPP__
#define __KELPA_UTILITY_UNIQUEANY_HPP__
#include <concepts>
#include <functional>

namespace Kelpa {
namespace Utility {

template <typename T, typename F> requires std::invocable<F, T> struct  UniqueAny {
	template <typename U> requires std::convertible_to<U, T>
	constexpr UniqueAny(U&& __value) noexcept : value(std::forward<U>(__value)) {}
	template <typename U, typename W> 
	constexpr UniqueAny(U&& __value, W&& __function) noexcept
		: value(std::forward<U>(__value))
		, function(std::forward<W>(__function)) {} 
	~UniqueAny() noexcept {
		std::invoke(function, value);
	}
	T value;
	F function;
};
template <typename U, typename W> UniqueAny(U&& , W&&) -> UniqueAny<U, W>;	
	
}	
}


#endif
