/** 
 * @Path		Kelpa/Src/Utility/Functions.hpp
 * @Brief		A collection of useful functions
 * @Dependency	None
 * @Since		2024/04/22
 * @Version     1st
 **/
 
#ifndef __KELPA_UTILITY_FUNCTIONS_HPP__
#define __KELPA_UTILITY_FUNCTIONS_HPP__

#include <source_location>				/* imports ./ { 
	std::source_location::current()
} */
#include <string>						/* imports ./string_view { 
	std::string_view 
}*/
#include <optional>						/* imports ./ { 
	std::optional, 
	std::nullopt 
}*/
#include <charconv>						/* imports ./ { 
	std::from_chars
}*/
#include <system_error>					/* imports ./ { 
	std::errc()
}*/
#include <chrono>						/* imports ./ { 
	std::duration_cast, 
	std::time_point, 
	std::now 
}*/
#include <bitset>						/* imports ./ { 
	std::bitset 
}*/
#include <sstream>						/* imports ./ { 
	std::stringstream 
}*/
namespace Kelpa {
namespace Utility {
	
template <typename Tp>
constexpr std::string_view TypeName() noexcept {
	std::string_view signature { std::source_location::current().function_name() };

	signature.remove_prefix(std::min(signature.find_first_of("=") + 2u, signature.size()));
	signature.remove_suffix(signature.size() - signature.find(';'));

	return signature;
}
template <typename __ENUM__, __ENUM__ __TYPE__>
constexpr std::string_view EnumName() noexcept {
	std::string_view view(std::source_location::current().function_name());

	auto first = view.find("__TYPE__ = ") + std::size("__TYPE__ = ") - 1;
	auto last = view.substr(first).find_first_of(';');

	return view.substr(first, last); 
}
template <typename T> std::optional<T> FromChars(std::string_view haystack) noexcept {
	T 		value {};
	auto [ptr, ec] = std::from_chars(haystack.data(), haystack.data() + haystack.size(), value);
	if(ec == std::errc() && ptr == haystack.data() + haystack.size())
		return value;
	return std::nullopt;
}	

template<typename Rep = unsigned long long, typename Period = std::milli>
std::chrono::duration<Rep, Period> Now() 			noexcept {
	return std::chrono::time_point_cast<std::chrono::duration<Rep, Period>>(
		std::chrono::system_clock::now()
	).time_since_epoch();
}

template <typename _Callable, typename... _Args>
constexpr auto Curry(_Callable&& f, _Args&&... args) noexcept {
    return [=]<typename... _OtherArgs>
        (_OtherArgs&&... otherArgs)mutable
        requires std::invocable<std::decay_t<_Callable>,
			_Args..., _OtherArgs...>
    {
        return f(std::forward<_Args>(args)...,
			     std::forward<_OtherArgs>(otherArgs)...);
    };
}

constexpr std::string_view Trim(std::string_view s) 			noexcept{
	using std::operator""sv;

	constexpr auto white_spaces { "\f\n\r\v\t "sv };

	auto first { s.find_first_not_of(white_spaces) };
	auto last  { s.find_last_not_of(white_spaces) + 1 };

	return s.substr(first, last);
}
template <std::unsigned_integral T>
constexpr bool GetBitAt(T value, std::size_t index) 			noexcept {
	return std::bitset<std::numeric_limits<T>::digits>(value)[index];
}

template <std::unsigned_integral T>
constexpr void SetBitAt(T& value, std::size_t index, bool b) 	noexcept {
	if(b) 
		value |= 1u << (index ^ (std::numeric_limits<T>::digits - 1));
	else 
		value &= ~(1u << (index ^ (std::numeric_limits<T>::digits - 1)));	
}

	
}
}

#endif
