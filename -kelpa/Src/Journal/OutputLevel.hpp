/**
 * @Path		Kelpa/Src/Journal/OutputLevel.hpp
 * @Brief		Defines the output levels that control the individual output subsystems
 * @Dependency	None
 * @Since		2024/04/22
 * @Version     1st
 **/
 
 
#ifndef __KELPA_JOURNAL_OUTPUT_LEVEL_HPP__
#define __KELPA_JOURNAL_OUTPUT_LEVEL_HPP__

#include <iostream>						/* imports ./ { 
	std::ostream 
}*/
#include <format>						/* imports ./ { 
	std::format, 
	std::format_to, 
	std::format_context, ... 
}*/
namespace Kelpa {
namespace Journal {

#define __KELPA_OUTPUT_LEVEL_XMACRO__(UNARY_FUNCTION) \
	UNARY_FUNCTION(Info)								\
	UNARY_FUNCTION(Debug)								\
	UNARY_FUNCTION(Warning)								\
	UNARY_FUNCTION(Error)								\
	UNARY_FUNCTION(Fatal)

inline constexpr enum class OutputLevel: unsigned char {
#define __KELPA_OUTPUT_LEVEL_ENUM_DECLARE__(CONCRETE) CONCRETE,
	__KELPA_OUTPUT_LEVEL_XMACRO__(__KELPA_OUTPUT_LEVEL_ENUM_DECLARE__)
#undef	__KELPA_OUTPUT_LEVEL_ENUM_DECLARE__
} const Default { OutputLevel::Debug };

inline constexpr char const* NameOfLevel(OutputLevel level) 		noexcept {
#define __KELPA_OUTPUT_LEVEL_NAME__(CONCRETE) case(OutputLevel::CONCRETE): return #CONCRETE;
	switch(level) {	__KELPA_OUTPUT_LEVEL_XMACRO__(__KELPA_OUTPUT_LEVEL_NAME__) }
#undef	__KELPA_OUTPUT_LEVEL_NAME__
	return "UNKNOWN";
}
inline constexpr char const* NameOfLevel(std::underlying_type_t<OutputLevel> level) noexcept {
#define __KELPA_OUTPUT_LEVEL_NAME__(CONCRETE) case(OutputLevel::CONCRETE): return #CONCRETE;
	switch(static_cast<OutputLevel>(level)) {	__KELPA_OUTPUT_LEVEL_XMACRO__(__KELPA_OUTPUT_LEVEL_NAME__) 	}
#undef	__KELPA_OUTPUT_LEVEL_NAME__
	return "UNKNOWN";	
}
#undef 	__KELPA_OUTPUT_LEVEL_XMACRO__

std::ostream& operator<<(std::ostream& Os, OutputLevel level) noexcept {
	return (Os << NameOfLevel(level));
}

}	
}

namespace std {
	
template<>  struct formatter<Kelpa::Journal::OutputLevel> {  
    constexpr auto parse(std::format_parse_context& context) 					const noexcept 
	{  	return context.begin(); 							}  
	auto format(Kelpa::Journal::OutputLevel const& t, std::format_context& context) 	const 
	{  	return std::format_to(context.out(), "{}", Kelpa::Journal::NameOfLevel(t));  	}  
}; 


}


#endif
