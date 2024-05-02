/** 
 * 		@Path	Kelpa/Src/Utility/Ignore.hpp
 * 		@Brief	Provides an assignable and constructible object passed to a value of any type
 * 		@Dependency	None
 * 		@Since  2024/04/23
 * 		@Version 1st
 **/
#ifndef __KELPA_UTILITY_IGNORE_HPP__ 
#define __KELPA_UTILITY_IGNORE_HPP__

#include <tuple>			/* imports tuple { 
	std::ignore,
	std::decay_t
}*/
#include <iostream>			/* imports ./ { 
	std::ostream 
}*/
#include <format>			/* imports ./ { 
	std::format, 
	std::format_string, 
	std::format_to, ... 
}*/
namespace Kelpa {
namespace Utility {

struct Ignore: std::decay_t<decltype(std::ignore)> {	
	template <typename... Args> constexpr 	Ignore(Args&& ...) noexcept {}		
};

std::ostream& operator<<(std::ostream& Os, Ignore const&) noexcept 
{	return (Os << "ignore");					}

}
}

namespace std {  
template<>  struct formatter<Kelpa::Utility::Ignore> {  
    constexpr auto parse(std::format_parse_context& context) 					const noexcept 
	{  	return context.begin(); 							}  
	auto format(const Kelpa::Utility::Ignore& t, std::format_context& context) 	const 
	{  	return std::format_to(context.out(), "ignore");  	}  
};  
}


#endif
