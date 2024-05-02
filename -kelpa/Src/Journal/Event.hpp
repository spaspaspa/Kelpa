/**
 * @Path			Kelpa/Src/Journal/Event.hpp
 * @Brief			An event object that is passed to the logging system to format the data fields it holds
 * @Dependency		../Coroutine/Future { ThisFiber::GetID() }, 
 * 					../Utility/Macros 	{ #define DEFINES_STRUCT_MEMBER_TYPES }	
 * @Since 			2024/04/22
 * @Version			1st
 **/

#ifndef __KELPA_JOURNAL_EVENT_HPP__
#define __KELPA_JOURNAL_EVENT_HPP__

#include <chrono> 			/* imports chrono/ { 
	std::chrono::system_clock::now(), 
	std::chrono::time_point_cast
}*/
/* #include <format> <utility> */
#include <concepts> 		/* imports concepts/ { 
	std::convertible_to 
}*/
#include <source_location> 	/* imports source_location/ {
	std::source_location::current(), 
	std::int_least32_t 
}*/
#include <thread> 			/* imports thread/ {
	std::thread::id 
	std::this_thread::get_id()
}*/
#include <cstdarg> 			/* imports cstdarg/ { 
	va_list 
} */
#include "../Utility/Macros.h" /* imports Macros/{
	#define __KELPA_DEFINES_STRUCT_MEMBER_TYPES__ 
}*/
namespace Kelpa {
namespace Journal {

template <typename T> struct 			ValueWithLocation;
struct 									Event;

/** 
 * @Brief		Record the time stamp when the program loads and starts
 * @Type		std::chrono::milliseconds
 * @Attributes	static inline:	
 * 				Global life time, file-scaled context scope, the name is uniquely defined when linked			
 **/
static inline auto const SinceStart {
	std::chrono::time_point_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now()
	).time_since_epoch()	
};
/** 
 * @Brief		Holds a template type value, and initialized with the current code line/column and other additional informations
 **/
template <typename T> struct ValueWithLocation {
	template <typename U> requires std::convertible_to<U, T>
	consteval ValueWithLocation(
		U&& 					__value, 
		std::source_location 	__location = std::source_location::current() 
	) noexcept: value(std::forward<U>(__value)), location(std::move(__location)) {}
	T 							value;
	std::source_location 		location;
};

struct Event {
__KELPA_DEFINES_STRUCT_MEMBER_TYPES__(Event)
	typedef std::uint_least32_t 								position_type;
	typedef char const*											descriptor_type;
	typedef std::chrono::time_point<std::chrono::system_clock>	time_point_type;
	typedef std::chrono::milliseconds 							duration_type;
	typedef unsigned long long									fiberid_type;
	/* the concret fiberid_type definition depends on Kelpa/Src/Coroutine/Future implement */ 									
	typedef std::thread::id 									threadid_type;
	typedef std::string											message_type;

	position_type 												line;
	position_type												column;
	descriptor_type 											file 		{ nullptr };
	descriptor_type												function 	{ nullptr };
	time_point_type												now			{ 	std::chrono::system_clock::now() };
	duration_type 												elapse		{ 	
															std::chrono::time_point_cast<std::chrono::milliseconds>(
																std::chrono::system_clock::now()
															).time_since_epoch() - SinceStart 
																			};
	fiberid_type												fiberID     { /* Coroutine::this_fiber::GetID()*/ };
	threadid_type												threadID    { std::this_thread::get_id() };
	message_type 												message;
	
	static Event 	FromString(std::string_view, std::source_location = std::source_location::current() )	noexcept;
	static Event 	FromStdSprintf(ValueWithLocation<char const *>, ...) 									noexcept;
	template <typename ...Args> 
	static Event 	FromStdFormat(ValueWithLocation<std::format_string<Args ...>>, Args&& ...) 				noexcept;
};

Event Event::FromString(std::string_view message, std::source_location information) 		noexcept {
	return Event { 
		.line = 	information.line(), 
		.column = 	information.column(), 
		.file =		information.file_name(),
		.function =	information.function_name(),
		.message = 	message.data()
	};
}
/** 
 * @Brief			Initialize an Event object with a C-style formatted string
 **/
Event Event::FromStdSprintf(ValueWithLocation<char const *> meta, ...) 							noexcept {
	std::va_list 	al;
	va_start(al, meta);
	char 			buffer [64] {0};
	int 			length { vsprintf(buffer, meta.value, al) };
	va_end(al);	
	return Event::FromString(~length ? buffer : "Error: Vsprintf" , std::move(meta.location));	
}
/** 
 * @Brief			Initialize an Event object with a C++-style formatted string
 **/
template <typename ...Args> 
Event Event::FromStdFormat(ValueWithLocation<std::format_string<Args ...>> meta, Args&& ...args) noexcept {
	return Event::FromString(
		std::vformat(
			meta.value.get(), 
			std::make_format_args(std::forward<Args>(args) ...)), 
		std::move(meta.location)
	);	
}

	
}
}




#endif
