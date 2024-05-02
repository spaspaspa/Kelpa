/** 
 * 		@Path	Kelpa/Src/Utility/StringLiteral.hpp
 * 		@Brief	Compiler constants that can be constructed from literals as template parameters
 * 		@Dependency	None
 * 		@Since  2024/04/23
 * 		@Version 1st
 **/
 
#ifndef __KELPA_UTILITY_STRINGLITERAL_HPP__
#define __KELPA_UTILITY_STRINGLITERAL_HPP__

#include <algorithm>				/* imports algorithm / { 
	std::size_t, 
	std::copy_n, 
}*/
#include <span>						/* imports span / { 
	std::begin, 
	std::end,
	./concept/concept std::same_as
}*/
#include <string_view>				/* imports ./ { 
	std::string_view 
}*/
#include <format>			/* imports ./ { 
	std::format, 
	std::format_string, 
	std::format_to, ... 
}*/
namespace Kelpa {
namespace Utility {
	
template <std::size_t N, typename Value = char> struct StringLiteral {
	typedef Value 			value_type;
	typedef Value * 		pointer;
	typedef Value const* 	const_pointer;
	typedef Value * 		iterator;
	typedef Value * 		const_iterator;
	typedef std::ptrdiff_t 	difference_type;
	typedef std::size_t		size_type;
	typedef Value& 			reference;
	typedef Value const& 	const_reference;

	template <std::size_t M>
	/*consteval*/constexpr StringLiteral(Value const (&__value)[M]) noexcept 
	{	std::copy_n(std::begin(__value), M, std::begin(value));		}

	/* constexpr */operator pointer () 		noexcept 
	{	return value;				}

	/* constexpr */operator const_pointer() const noexcept 
	{	return value;				}

	/* constexpr*/operator std::basic_string_view<Value>() 	const noexcept 
	{	return value;				}

	/* constexpr*/operator std::span<Value, N>() 			const noexcept 
	{	return value;				}

	/* constexpr */size_type length() 		const noexcept 
	{ 	return N; 					}

	/* constexpr */size_type size() 		const noexcept 
	{ 	return N; 					}	

	/* constexpr */const_reference front() 	const noexcept 
	{ 	return value[0];			}

	/* constexpr */reference front() 		noexcept 
	{ 	return value[0];			}

	/* constexpr */const_reference back() 	const noexcept 
	{ 	return value[N - 1];		}

	/* constexpr */reference back() 		noexcept 
	{ 	return value[N - 1];		}

	/* constexpr */decltype(auto) cbegin() 	const noexcept 
	{	return std::cbegin(value);	}

	/* constexpr */decltype(auto) begin() 	noexcept 
	{	return std::begin(value);	}

	/* constexpr */decltype(auto) cend() 	const noexcept 
	{	return std::cend(value);	}

	/* constexpr */decltype(auto) end() 	noexcept 
	{	return std::end(value);		}

	/* cronstexpr */decltype(auto) crbegin() const noexcept 
	{	return std::crbegin(value);	}

	/* cronstexpr */decltype(auto) rbegin() noexcept 
	{	return std::rbegin(value);	}

	/* cronstexpr */decltype(auto) crend() 	const noexcept 
	{	return std::crend(value);	}

	/* cronstexpr */decltype(auto) rend() 	noexcept 
	{	return std::rend(value);	}
	Value value[N];
};
template <std::size_t M, typename Value> StringLiteral(Value const (&)[M]) -> StringLiteral<M, Value>;

}
}

namespace std {

template <std::size_t N, typename Value> struct hash<Kelpa::Utility::StringLiteral<N, Value>> {
	constexpr std::size_t operator()(Kelpa::Utility::StringLiteral<N, Value> const& literal) 			const noexcept 
	{	return std::hash<std::basic_string_view<Value>>{} (literal);			}
};

template<std::size_t N>  struct formatter<Kelpa::Utility::StringLiteral<N, char>> {  
    constexpr auto parse(std::format_parse_context& context) 	noexcept 
	{  	return context.begin(); 								}  

	constexpr auto format(Kelpa::Utility::StringLiteral<N, char> const& t, std::format_context& context) const noexcept
	{  	return std::format_to(context.out(), "{}", t.value);  	}  
};  


}

#endif
