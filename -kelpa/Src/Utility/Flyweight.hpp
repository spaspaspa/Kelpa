/** 
 * @Path   		Kelpa/Src/Utility/Flyweight.hpp
 * @Brief			The calculation results corresponding to different parameters 
 * 					are cached and then repeated calculation is not performed
 * @Dependency	./ 			{ Concepts.hpp }
 * 				../Serde/ 	{ FunctionTraits.hpp }
 * @Since		2024/04/22
 * @Version     1st
 **/
 
#ifndef __KELPA_UTILITY_FLYWEIGHT_HPP__
#define __KELPA_UTILITY_FLYWEIGHT_HPP__
#include <iostream>
#include <unordered_map>			/* imports unordered_map { 
	std::unordered_map 
	std::hash
}*/ 
#include <tuple>					/* imports tuple { 
	std::tuple 
}*/ 

#include "./Concepts.hpp"			/* imports ./ { 
	non_reference, 
	non_void 
}*/
#include "../Serde/FunctionTraits.hpp" /* imports ./ { 
	struct FunctionTraits 
}*/

namespace std {  
template <typename... Args> 	struct hash<std::tuple<Args...>> {  
    size_t operator()(std::tuple<Args...> const& tup) const noexcept {  
        std::size_t 	result {};  
        return std::apply([&](auto&&... args) mutable -> std::size_t {  
            ((result = 	Combine(result, std::hash<std::decay_t<decltype(args)>>{}(args))), ...);  
            return 		result;
        }, 				tup);  
    }  
    template <typename T>  static constexpr std::size_t Combine(size_t hash_code, T const& value) noexcept 
	{  	return hash_code ^ (std::hash<T>{}(value) + (hash_code << 6) + (hash_code >> 2));  }  
}; 	}; 

namespace Kelpa {
namespace Utility {
	
template <typename> struct Flyweight { static_assert(std::bool_constant<false>::value, "non-void/non-reference return required"); };
template <typename R, typename... Args> requires (non_void<R> && non_reference<R>) 
struct Flyweight<R(Args ...)>: Serde::FunctionTraits<R(Args ...)> {
	template <typename F> constexpr Flyweight(F&& __f) noexcept: f(std::forward<F>(__f)) {}
	constexpr R& operator()(Args ...args) noexcept {
		auto hash_code { std::hash<std::tuple<Args ...>>{} (std::forward_as_tuple(args ...)) };

		if(m.contains(hash_code)) 		
			return m[hash_code]; 
		return (* m.emplace(hash_code, std::invoke(f, std::forward<Args>(args) ...)).first).second;
	}
private:
	std::function<R(Args ...)>						f;
	std::unordered_map<std::size_t, R> 				m;
};
template <typename F> Flyweight(F&&) -> Flyweight<typename Serde::FunctionTraits<std::decay_t<F>>::function_type>;	
	
}
}

#endif
