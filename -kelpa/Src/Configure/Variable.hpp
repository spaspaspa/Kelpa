/** 
 * 		@Path 	Kelpa/Src/Configure/Varianble.hpp
 * 		@Brief	Defines the type of variable that describes the current configuration information
 * 		@Dependency		None
 * 		@Since	2024/04/24
 * 		@Version 1st
 **/
 
#ifndef __KELPA_CONFIGURE_VARIABLE_HPP__
#define __KELPA_CONFIGURE_VARIABLE_HPP__
#include <typeindex>					/* imports ./ { 
	std::typeindex::hash_code, 
	std::size_t 
}*/
#include <string>						/* imports ./ {
	std::string
	./string_view { std::string_view }
}*/
#include <functional>					/* imports ./ { 
	std::function, 
	std::invoke
}*/
#include <utility>						/* imports ./ { 
	std::exchange 
}*/
namespace Kelpa {
namespace Configure {
namespace Detail {
struct						 VariableBase { 
	virtual ~VariableBase() 		noexcept 		= default;
	virtual std::size_t HashCode() 	const noexcept 	= 0;
};
}
	
template <typename T> struct Variable: Detail::VariableBase {
	typedef T 					value_type;
	typedef T * 				pointer;
	typedef T const* 			const_pointer;
	typedef T& 					reference;
	typedef T const& 			const_reference;	

	constexpr Variable(
		std::string const& 		__identifier, 
		T const& 				__value,
		std::string const& 		__description
	) noexcept: identifier(__identifier), value(__value), description(__description) {}
	
	template <typename U> T Exchange(U&& new_value) 
	noexcept(
	    std::is_nothrow_move_constructible_v<T> 
	&&  std::is_nothrow_assignable_v<T&, U>
	) {
		for(auto& [dummy, func]: callbacks) 
			std::invoke(func, value, std::forward<U>(new_value));
		return std::exchange(value, std::forward<U>(new_value));
	}
	
	virtual std::size_t 	HashCode() const noexcept override 
	{  	return std::type_index(typeid(T)).hash_code();			}
	
	template <typename F> requires std::invocable<F, T const&, T const&>
	constexpr std::size_t 	Listen(F&& f) const noexcept {
		callbacks.emplace(count, [&] (T const& old_value, T const& new_value) { 
			(void) std::invoke(std::forward<F>(f), old_value, new_value);
		});
		return std::exchange(count, count + 1);
	} 
	constexpr bool		 	Cancel(std::size_t indexer) const noexcept 
	{ 	return static_cast<bool>(callbacks.erase(indexer));		}

	constexpr operator T const&() const noexcept 	{ return value;	}
	constexpr operator T &() noexcept 				{ return value;	}
	
	mutable std::size_t 				count {};
	mutable std::unordered_map<std::size_t, std::function<void(T const&, T const&)>> 
										callbacks;
										
	std::string 						identifier;	
	T 									value;
	std::string							description;
};	
	
	
	
	
}
}


#endif
