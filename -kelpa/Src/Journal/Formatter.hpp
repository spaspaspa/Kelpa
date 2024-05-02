/** 
 * @Path		Kelpa/Src/Journal/Formatter.hpp
 * @Brief		A encapsulated object that holds a string 
 * 				arranged by a formatting-placeholders combination 
 * 				to format the output message
 * @Dependency	./ { Logger.hpp, Event.hpp, OutputLevel.hpp }
 * 				../Utility/Macros { Flyweight.hpp #define DEFINES_STRUCT_MEMBER_TYPES }	
 * @Since 			2024/04/22
 * @Version			1st
 **/

#ifndef __KELPA_JOURNAL_FORMATTER_HPP__
#define __KELPA_JOURNAL_FORMATTER_HPP__
#include <iostream>					/* imports iostream { 
	extern std::cout 
}*/
#include <algorithm>				/* imports algorithm { 
	std::ranges::sort, 
	std::unique, 
	std::erase 
}*/
#include <regex>					/* imports regex { 
	std::regex_iterator 
	std::regex_search/replace 
}*/
#include <utility>					/* imports utlity { 
	std::exchange 
}*/
#include <vector>					/* imports vector { 
	std::vector 
}*/
#include "../Utility/Macros.h" 		/* imports Macros/{
	#define __KELPA_DEFINES_STRUCT_MEMBER_TYPES__ 
}*/
#include "../Utility/Flyweight.hpp" /* imports Flyweight { 
	struct Flyweight 
}*/
#include "../Utility/Interfaces.hpp"/* imports ./ {
	struct SharedFrom
} */
#include "./OutputLevel.hpp" 		/* imports ./ { 
	struct OutputLevel::{
		Enum, 
		Default() 
	}
}*/
#include "./Event.hpp"				/* imports ./{ 
	struct Event 
} */

namespace Kelpa {
namespace Journal {


struct Formatter : public Utility::SharedFrom<Formatter> {
__KELPA_DEFINES_STRUCT_MEMBER_TYPES__(Formatter)
struct Context {
	std::string_view					logger;
	OutputLevel					level;
	Event::const_reference_wrapper 		event;
};
struct Cell {
__KELPA_DEFINES_STRUCT_MEMBER_TYPES__(Cell)	
	virtual ~Cell() 													noexcept = default;
	virtual Cell::const_reference Format(std::ostream&, Context const&) const noexcept;
};
	static Formatter::shared_pointer Shared(
		std::string_view  =	"%d{%Y-%m-%d %H:%M:%S}"
							"%T%r%T[%n - %p]"
							"%T%f:%l:%c%T%F%"
							"T%t|%v%T%m%N" ) 	noexcept;
	std::string Format(Context const&) 			const noexcept;	
	std::vector<Cell::const_reference_wrapper>		cells;
	std::string 									pattern;	
};	
	
/*
#formatting-placeholders:
	#m#, Event::message		(std::string)					
	#p#, OutputLevel::Name	(char const*)				
	#n#, Logger::name		(std::string)				
	#r#, Event::elapse		(std::duration)					
	#N#, Enter				("\r\n")						
	#T#, Indent by Tab		("\t")					
	#d#, Date				(std::time_point)						
	#t#, ThreadID			(std::thread::id)					
	#v#, FiberID			(Coroutine::id)					
	#f#, File				(char const*)						
	#F#, Function			(char const*)					
	#l#, Line				(std::int_least32_t)						
	#c#, Column				(std::int_least32_t)					
	#s#, Forward			(std::string)					
*/	
std::string Formatter::Format(Context const& context) const noexcept {
	std::stringstream s;
	for(auto const& refer: cells) (void) static_cast<Cell::const_reference>(refer).Format(s, context);
	return s.str();
}

struct MessageCell: Formatter::Cell {
	constexpr MessageCell(std::string const&) 													noexcept {}
	virtual Cell::const_reference Format(std::ostream& Os, Formatter::Context const& context) 	const noexcept override {
		(void) (Os << std::quoted(static_cast<Event::const_reference>(context.event).message));
		return dynamic_cast<Cell::const_reference>(*this);
	}
};
struct LevelCell: Formatter::Cell {
	constexpr LevelCell(std::string const&) 													noexcept {}
	virtual Cell::const_reference Format(std::ostream& Os, Formatter::Context const& context) 	const noexcept override {
		(void) (Os << NameOfLevel(context.level));
		return dynamic_cast<Cell::const_reference>(*this);
	}
};
struct LoggerCell: Formatter::Cell {
	constexpr LoggerCell(std::string const&) 													noexcept {}
	virtual Cell::const_reference Format(std::ostream& Os, Formatter::Context const& context) 	const noexcept override {
		(void) (Os << context.logger);
		return dynamic_cast<Cell::const_reference>(*this);
	}
};	
struct ElapseCell: Formatter::Cell {
	constexpr ElapseCell(std::string const&) 													noexcept {}
	virtual Cell::const_reference Format(std::ostream& Os, Formatter::Context const& context) 	const noexcept override {
		(void) (Os << static_cast<Event::const_reference>(context.event).elapse);
		return dynamic_cast<Cell::const_reference>(*this);
	}
};
struct EnterCell: Formatter::Cell {
	constexpr EnterCell(std::string const&) 													noexcept {}
	virtual Cell::const_reference Format(std::ostream& Os, Formatter::Context const& context) 	const noexcept override {
		(void) endl(Os);
		return dynamic_cast<Cell::const_reference>(*this);
	}
};
struct IndentCell: Formatter::Cell {
	constexpr IndentCell(std::string const&) 													noexcept {}
	virtual Cell::const_reference Format(std::ostream& Os, Formatter::Context const& context) 	const noexcept override {
		(void) Os.put('\t');
		return dynamic_cast<Cell::const_reference>(*this);
	}
};
struct DateCell: Formatter::Cell {
	constexpr DateCell(std::string const& __fmts) 												noexcept: fmts(__fmts.empty() ? std::string{"%Y-%m-%d %H:%M:%S"} : std::move(__fmts)) {}
	virtual Cell::const_reference Format(std::ostream& Os, Formatter::Context const& context) 	const noexcept override {
		std::time_t 	time { std::chrono::system_clock::to_time_t(static_cast<Event::const_reference>(context.event).now) };
    	char 			buffer[64] {};
		std::strftime(std::data(buffer), std::size(buffer), fmts.c_str(), std::gmtime(&time));
        (void) (Os << buffer);
		return dynamic_cast<Cell::const_reference>(*this);
	}
	std::string 		fmts;
};
struct ThreadIDCell: Formatter::Cell {
	constexpr ThreadIDCell(std::string const&) 													noexcept {}
	virtual Cell::const_reference Format(std::ostream& Os, Formatter::Context const& context) 	const noexcept override {
		(void) (Os << static_cast<Event::const_reference>(context.event).threadID);
		return dynamic_cast<Cell::const_reference>(*this);
	}
};
struct FiberIDCell: Formatter::Cell {
	constexpr FiberIDCell(std::string const&) 													noexcept {}
	virtual Cell::const_reference Format(std::ostream& Os, Formatter::Context const& context) 	const noexcept override {
		(void) (Os << static_cast<Event::const_reference>(context.event).fiberID);
		return dynamic_cast<Cell::const_reference>(*this);
	}
};
struct FileCell: Formatter::Cell {
	constexpr FileCell(std::string const&) 														noexcept {}
	virtual Cell::const_reference Format(std::ostream& Os, Formatter::Context const& context) 	const noexcept override {
		(void) (Os << static_cast<Event::const_reference>(context.event).file);
		return dynamic_cast<Cell::const_reference>(*this);
	}
};
struct FunctionCell: Formatter::Cell {
	constexpr FunctionCell(std::string const&) 													noexcept {}
	virtual Cell::const_reference Format(std::ostream& Os, Formatter::Context const& context) 	const noexcept override {
		(void) (Os << static_cast<Event::const_reference>(context.event).function);
		return dynamic_cast<Cell::const_reference>(*this);
	}
};
struct LineCell: Formatter::Cell {
	constexpr LineCell(std::string const&) 														noexcept {}
	virtual Cell::const_reference Format(std::ostream& Os, Formatter::Context const& context) 	const noexcept override {
		(void) (Os << static_cast<Event::const_reference>(context.event).line);
		return dynamic_cast<Cell::const_reference>(*this);
	}
};
struct ColumnCell: Formatter::Cell {
	constexpr ColumnCell(std::string const&) 														noexcept {}
	virtual Cell::const_reference Format(std::ostream& Os, Formatter::Context const& context) 	const noexcept override {
		(void) (Os << static_cast<Event::const_reference>(context.event).column);
		return dynamic_cast<Cell::const_reference>(*this);
	}
};
struct ForwardCell: Formatter::Cell {
	constexpr ForwardCell(std::string const& __s) 													noexcept:s(std::move(__s))  {}
	virtual Cell::const_reference Format(std::ostream& Os, Formatter::Context const& context) 	const noexcept override {
		(void) (Os << s);
		return dynamic_cast<Cell::const_reference>(*this);
	}
	std::string 		s;
};

struct CellFactory {
	enum class 		Category: unsigned char { CURLY, ALPHA, ESCAPE, FORWARD };
	typedef 		Formatter::Cell::unique_pointer 							pointer;
	typedef 		Formatter::Cell::const_reference_wrapper 					const_reference;
	static  inline  const_reference FetchOne(std::string_view, Category) 		noexcept;
	static 	std::unordered_map<char, std::function<pointer(std::string_view)>>	Generator;
};
std::unordered_map<char, std::function<CellFactory::pointer(std::string_view)>>	CellFactory::Generator { 
#define __KELPA_FORMATTER_CELL_GENERATOR_XMACRO__(BINARY_FUNCTION) \
	BINARY_FUNCTION('m', MessageCell)					\
	BINARY_FUNCTION('p', LevelCell)						\
	BINARY_FUNCTION('n', LoggerCell)					\
	BINARY_FUNCTION('r', ElapseCell)					\
	BINARY_FUNCTION('N', EnterCell)						\
	BINARY_FUNCTION('T', IndentCell)					\
	BINARY_FUNCTION('d', DateCell)						\
	BINARY_FUNCTION('t', ThreadIDCell)					\
	BINARY_FUNCTION('v', FiberIDCell)					\
	BINARY_FUNCTION('f', FileCell)						\
	BINARY_FUNCTION('F', FunctionCell)					\
	BINARY_FUNCTION('l', LineCell)						\
	BINARY_FUNCTION('s', ForwardCell)					\
	BINARY_FUNCTION('c', ColumnCell)					
#define __KELPA_FORMATTER_CELL_DECLARE__(CHAR, CLASS)  \
	{	CHAR, [] (std::string_view placeholder){ return CellFactory::pointer(new CLASS(placeholder.data())); 	} },	
	__KELPA_FORMATTER_CELL_GENERATOR_XMACRO__(__KELPA_FORMATTER_CELL_DECLARE__)
#undef __KELPA_FORMATTER_CELL_DECLARE__
};
#undef __KELPA_FORMATTER_CELL_GENERATOR_XMACRO__


inline CellFactory::const_reference CellFactory::FetchOne(std::string_view pattern, Category category) noexcept {
	static Utility::Flyweight Despatcher( 
	[] (std::string_view pattern, Category category) {
		if(category == Category::ESCAPE || category == Category::FORWARD) 
			return std::invoke(Generator['s'], pattern.data());

		std::string 	temporary { pattern.data() };
	    char 			indicator { std::exchange(* (++ temporary.begin()), '*') };
		if(!temporary.starts_with("%*") || !Generator.contains(indicator)) 
			return std::invoke(Generator['s'], pattern.data());

		std::size_t 	begin = pattern.find_first_of('{');  
	    std::size_t 	end = 	pattern.find_last_of('}');  
	    std::string 	content {};
		
		if (!(begin == std::string::npos || end == std::string::npos || begin > end)) 
			content = pattern.substr(begin + 1, end - begin - 1);
	        
	    return std::invoke(Generator[indicator], content);		
	});
	return std::ref(* Despatcher(pattern, category));
}

Formatter::shared_pointer Formatter::Shared(std::string_view __pattern) noexcept {
	auto pointer { std::make_shared<Formatter>() };
	(* pointer).pattern = __pattern.data();
	typedef CellFactory::Category Category;
	std::string 	temporary 		{ (* pointer).pattern };
	std::regex 		curly_regex		(R"(\%[a-zA-Z]\{.*?\})");
	std::regex 		alpha_regex		(R"(\%[a-zA-Z])");
	std::regex 		escape_regex	(R"(%%)");
	std::regex 		decarative_regex(R"((?!^[*]*$)[^*]+)");
	
	std::vector<std::tuple<std::size_t, std::size_t, Category>> coordinates;
	
	for(std::sregex_iterator iterator { temporary.cbegin(), temporary.cend(), curly_regex };
		iterator != std::sregex_iterator{}; 
		iterator ++
	) {
		temporary.replace(temporary.begin() + (* iterator).position(), temporary.begin() + (* iterator).position() + (* iterator).length(), (* iterator).length(), '*');
		coordinates.emplace_back((* iterator).position(), (* iterator).position() + (* iterator).length(), Category::CURLY);
	} 
	for(std::sregex_iterator iterator { temporary.cbegin(), temporary.cend(), alpha_regex };
		iterator != std::sregex_iterator{}; 
		iterator ++
	) {
		temporary.replace(temporary.begin() + (* iterator).position(), temporary.begin() + (* iterator).position() + (* iterator).length(), (* iterator).length(), '*');
		coordinates.emplace_back((* iterator).position(), (* iterator).position() + (* iterator).length(), Category::ALPHA);
	} 
	for(std::sregex_iterator iterator { temporary.cbegin(), temporary.cend(), escape_regex };
		iterator != std::sregex_iterator{}; 
		iterator ++
	) {
		temporary.replace(temporary.begin() + (* iterator).position(), temporary.begin() + (* iterator).position() + (* iterator).length(), (* iterator).length(), '*');
	 	coordinates.emplace_back((* iterator).position(), (* iterator).position() + (* iterator).length(), Category::ESCAPE);
	}
	for(std::sregex_iterator iterator { temporary.cbegin(), temporary.cend(), decarative_regex };
		iterator != std::sregex_iterator{}; 
		iterator ++
	) coordinates.emplace_back((* iterator).position(), (* iterator).position() + (* iterator).length(), Category::FORWARD);
	
	using const_reference = typename std::decay_t<decltype(coordinates)>::const_reference;
	
	std::ranges::sort(coordinates, [] (const_reference first, const_reference second) {
		if(!std::equal_to<>{} (std::get<0>(first), std::get<0>(second)))	
			return std::less<>{} (std::get<0>(first), std::get<0>(second));
		return std::less<>{} (std::get<1>(second), std::get<1>(first));
	}); 
		
	coordinates.erase(std::unique(coordinates.begin(), coordinates.end(), [] (const_reference first, const_reference second) {
			return std::equal_to<>{} (std::get<0>(first), std::get<0>(second));
	}), coordinates.end());
	
	for(auto& [begin, end, category]: coordinates) 
		(* pointer).cells.emplace_back(CellFactory::FetchOne((* pointer).pattern.substr(begin, end - begin), category));
	return pointer;
}



}	
}



#endif
