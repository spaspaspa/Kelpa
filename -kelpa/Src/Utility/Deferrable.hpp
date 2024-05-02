/** 
 * 		@Path	Kelpa/Src/Utility/Deferrable.hpp
 * 		@Brief	Encapsulate a type that is not initialized after it is constructed
 * 		@Dependency	./Error.hpp
 * 		@Since  2024/04/23
 * 		@Version 1st
 **/
 
#ifndef __KELPA_UTILITY_DEFERRABLE_HPP__
#define __KELPA_UTILITY_DEFERRABLE_HPP__

#include <variant>								/* imports ./ { 
	std::variant, 
	std::get_if, 
	std::holds_alternative 
}*/
#include <utility>								/* imports ./ { 
	std::as_const 
}*/
#include "./Error.hpp"							/* imports ./ { 
	#define Assert__() 
}*/					
namespace Kelpa {
namespace Utility {
	
template <typename T> struct Deferrable {
	typedef T 					value_type;
	typedef T * 				pointer;
	typedef T const* 			const_pointer;
	typedef T& 					reference;
	typedef T const& 			const_reference;
		
	template <typename... Args> requires std::constructible_from<T, Args ...>
	constexpr decltype(auto) Emplace(Args&& ...args) noexcept(noexcept(	T { 	std::forward<Args>(args) ...	})) 
	{	return value. template emplace<T>(std::forward<Args>(args) ...);	}
	
	constexpr T Move() noexcept(std::is_trivially_move_constructible<T>::value) {
		Assert__(std::holds_alternative<T>(value), "Uninitialized Deferrable not moveable");

		auto 	pointer { std::get_if<T>(std::addressof(value)) };
		T 		result { std::move(* pointer) };

		std::destroy_at(pointer);
		value = std::monostate {};
	
		return result;	
	}

	constexpr decltype(auto) Refer() const noexcept {
		Assert__(std::holds_alternative<T>(value), "Uninitialized Deferrable not referrable");
		return std::as_const(* std::get_if<T>(std::addressof(value)));
	}

	constexpr decltype(auto) Refer() noexcept 
	{	return const_cast<T &>(std::as_const(* this).Refer());	}
	
	constexpr std::add_pointer_t<T> Getif() noexcept 
	{	return std::get_if<T>(std::addressof(value));	}

	constexpr std::add_pointer_t<T const> Getif() const noexcept 
	{	return std::get_if<T>(std::addressof(value));	}	
	
	constexpr bool Holds() const noexcept 
	{	return std::holds_alternative<T>(value);	}

	constexpr operator bool() const noexcept 
	{	return std::holds_alternative<T>(value);	}	
protected:
	std::variant<std::monostate, T> value {};
};

template <typename T> struct Deferrable<T const>: Deferrable<T> {};
template <typename T> struct Deferrable<T &>: Deferrable<std::reference_wrapper<T>> {};

template <> struct Deferrable<void> {
	template <typename... Args> constexpr void Emplace(Args&& ...args) const noexcept {}
	constexpr void Move() 		const noexcept {}
	constexpr void Refer() 		const noexcept {}
	constexpr void Getif() 		const noexcept {}
	constexpr bool Holds() 		const noexcept { return true; }
	constexpr operator bool() 	const noexcept { return true; }
};	
	
}
}


#endif
