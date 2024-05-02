/** 
 * 		@Path	Kelpa/Src/Utility/SelfWrap.hpp
 * 		@Brief	The type itself is wrapped by the variant type, 
 * 				which acts as a return value for possible uploaded errors
 * 		@Dependency	None
 * 		@Since  2024/04/23
 * 		@Version 1st
 **/
 
#ifndef __KELPA_UTILITY_SELFWRAP_HPP__
#define __KELPA_UTILITY_SELFWRAP_HPP__
#include <exception>						/* imports ./ { 
	std::exception 
}*/
#include <string>							/* imports ./string_view/ { 
	std::string_view 
}*/								
#include <variant>							/* imports ./ { 
	std::variant,
	std::get
}*/	
#include <utility>							/* imports ./ { 
	std::as_const 
}*/		
#include <functional>						/* imports ./ { 
	std::invoke 
}*/
#include "./Concepts.hpp"					/* importt ./ { 
	concept non_reference 
}*/
namespace Kelpa {
namespace Utility {
	
struct Panic : std::exception {
	Panic(std::string_view __warning) noexcept: warning(__warning) {}
	char const *what() const noexcept { return warning.data(); }
private:	std::string_view warning;
};

template <Kelpa::Utility::non_reference T> struct SelfWrap {
	typedef T 					value_type;
	typedef T * 				pointer;
	typedef T const* 			const_pointer;
	typedef T& 					reference;
	typedef T const& 			const_reference;	
	
	constexpr T const& Unwrap() const& noexcept 						{
		if(std::holds_alternative<Panic>(Wrapper)) {
			std::fprintf(stderr, std::get<Panic>(Wrapper).what());
			std::quick_exit(EXIT_SUCCESS);
		}
		return std::get<std::reference_wrapper<T>>(Wrapper).get();
	}
	constexpr T& Unwrap() 		&	noexcept 
	{	return const_cast<T&>(std::as_const(*this).Unwrap());					}
	constexpr T  Unwrap() 		&& noexcept 
	{	return std::move(const_cast<T&>(std::as_const(*this).Unwrap()));		}
	
	constexpr T const& Expect(std::string_view message) const& noexcept {
		if(std::holds_alternative<Panic>(Wrapper)) {
			std::fprintf(stderr, message.data());
			std::quick_exit(EXIT_SUCCESS);
		}
		return std::get<std::reference_wrapper<T>>(Wrapper).get();
	}
	constexpr T& Expect(std::string_view message) 		& noexcept 
	{	return const_cast<T&>(std::as_const(*this).Expect(message));			}	
	constexpr T  Expect(std::string_view message) 		&& noexcept 
	{	return std::move(const_cast<T&>(std::as_const(*this).Expect(message)));	}	
	
	template<typename F> requires std::invocable<F, T&>
	constexpr decltype(auto) AndThen(F&& f) & noexcept {
		if (!std::holds_alternative<Panic>(Wrapper))
		    return std::invoke(std::forward<F>(f), std::get<std::reference_wrapper<T>>(Wrapper).get());
		else
		    return std::remove_cvref_t<std::invoke_result_t<F, T&>>{};		
	}
	template<typename F> requires std::invocable<F, T const&>
	constexpr decltype(auto) AndThen(F&& f) const& noexcept {
		if (!std::holds_alternative<Panic>(Wrapper))
		    return std::invoke(std::forward<F>(f), std::get<std::reference_wrapper<T>>(Wrapper).get());
		else
		    return std::remove_cvref_t<std::invoke_result_t<F, T const&>>{};		
	}
	template<typename F> requires std::invocable<F, T>
	constexpr decltype(auto) AndThen(F&& f) && noexcept {
		if (!std::holds_alternative<Panic>(Wrapper))
		    return std::invoke(std::forward<F>(f), std::move(std::get<std::reference_wrapper<T>>(Wrapper).get()));
		else
		    return std::remove_cvref_t<std::invoke_result_t<F, T>>{};		
	}
	template<typename F> requires std::invocable<F, T const>
	constexpr decltype(auto) AndThen(F&& f) const&& noexcept {
		if (!std::holds_alternative<Panic>(Wrapper))
		    return std::invoke(std::forward<F>(f), std::move(std::get<std::reference_wrapper<T>>(Wrapper).get()));
		else
		    return std::remove_cvref_t<std::invoke_result_t<F, T const>>{};		
	}	
	
	template <typename F> requires std::is_invocable_r_v<T, F>
	constexpr T OrElse(F&& f) const& noexcept {
		return !std::holds_alternative<Panic>(Wrapper) ? std::get<std::reference_wrapper<T>>(Wrapper).get() : std::forward<F>(f)();
	}
	template <typename F> requires std::is_invocable_r_v<T, F>
	constexpr T OrElse(F&& f) && noexcept {
		return !std::holds_alternative<Panic>(Wrapper) ? std::move(std::get<std::reference_wrapper<T>>(Wrapper).get()) : std::forward<F>(f)();
	}	

	template<typename U> requires std::convertible_to<U, T>
	constexpr T ValueOr(U&& default_value) const& noexcept {
		return !std::holds_alternative<Panic>(Wrapper) ? std::get<std::reference_wrapper<T>>(Wrapper).get() : static_cast<T>(std::forward<U>(default_value));
	}
	template<typename U> requires std::convertible_to<U, T>
	constexpr T ValueOr(U&& default_value) && noexcept {
		return !std::holds_alternative<Panic>(Wrapper) ? std::move(std::get<std::reference_wrapper<T>>(Wrapper).get()) : static_cast<T>(std::forward<U>(default_value));
	}
	
	static constexpr SelfWrap<T> Arouse(std::string_view warning = "unknown") 	noexcept { return SelfWrap<T> { Panic{warning} }; 		}
	static constexpr SelfWrap<T> Enwrap(T& reference) 				noexcept { return SelfWrap<T> { std::ref(reference) }; 	}
	
	std::variant<std::reference_wrapper<T>, Panic> 	Wrapper;
};	
	
	
}
}


#endif
