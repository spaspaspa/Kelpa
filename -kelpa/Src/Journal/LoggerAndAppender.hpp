/** 
 * @Path		Kelpa/Src/Journal/Journal.hpp
 * @Brief		The appender is associated with a specific output device 
 * 				and outputs a given event message body formatted to 
 * 				the associated device using an internally held formatter object	
 * 			
 * 				The Logger acts as an intermediate bridge between 
 * 				the same event message body and multiple output-oriented device appenders		
 * @Dependency	./ 					{ Event.hpp, OutputLevel.hpp, Formatter.hpp }
 * 				../Utility/Macros 	{ #define DEFINES_STRUCT_MEMBER_TYPES }	
 * @Since 			2024/04/22
 * @Version			1st
 **/
 
#ifndef __KELPA_JOURNAL_LOGGERANDAPPENDER_HPP__
#define __KELPA_JOURNAL_LOGGERANDAPPENDER_HPP__
#include <syncstream>				/* imports syncstream { 
	std::osyncstream 
}*/
#include <iostream>					/* imports iostream { 
	extern std::cout 
}*/
#include <fstream>					/* imports ofstream { 
	std::ofstream 
}*/
#include <list>						/* list/ { 
	std::list 
}*/
#include <optional>					/* imports optional { 
	std::optional 
}*/
#include "../Utility/Macros.h" 		/* imports Macros/{
	#define __KELPA_DEFINES_STRUCT_MEMBER_TYPES__ 
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
#include "./Event.hpp" 				/* imports ./ { 
	struct Event 
} */
#include "./Formatter.hpp"			/* imports ./ { 
	struct Formatter
}*/
namespace Kelpa {
namespace Journal {
struct 		Appender;
struct 		Logger;
struct 		Logger: std::enable_shared_from_this<Logger>, public Utility::SharedFrom<Logger> {
__KELPA_DEFINES_STRUCT_MEMBER_TYPES__(Logger)
	static Logger::shared_pointer Shared() 										noexcept;
	static Logger::shared_pointer Shared(std::string_view) 						noexcept;
	static Logger::shared_pointer Shared(OutputLevel) 					noexcept;
	static Logger::shared_pointer Shared(std::string_view, OutputLevel) 	noexcept;

	Logger::reference BroadCast(Event::const_reference event) 	noexcept;
	~Logger() 										noexcept;
	Logger::reference PushAppender(std::shared_ptr<Appender>) 	noexcept;
	Logger::reference EraseAppender(std::shared_ptr<Appender>) 	noexcept;

	OutputLevel 							filter;
	std::string								name;
	std::list<std::shared_ptr<Appender>>	appenders;
};	 
struct 		Appender : public Utility::SharedFrom<Appender> {
__KELPA_DEFINES_STRUCT_MEMBER_TYPES__(Appender)
	virtual ~Appender() 							noexcept = default;
	virtual Appender::const_reference Append() 		const noexcept = 0;
	
	OutputLevel 									level;
	std::weak_ptr<Logger> 							logger;
	Formatter::shared_pointer						formatter;
	std::optional<Event::const_reference_wrapper> 	event { std::nullopt };
};	
Logger::shared_pointer Logger::Shared() 										noexcept 
{	return Shared("default", Default);		};

Logger::shared_pointer Logger::Shared(std::string_view __name) 					noexcept 
{	return Shared(__name.data(), Default);	};

Logger::shared_pointer Logger::Shared(OutputLevel __filter) 				noexcept 
{	return Shared("default", __filter);						}

Logger::shared_pointer Logger::Shared(std::string_view __name, OutputLevel __filter) noexcept {
	auto pointer { std::make_shared<Logger>() };
	(* pointer).filter 	= 	__filter;
	(* pointer).name	= 	__name.data();
	return pointer;	
};
Logger::reference Logger::BroadCast(Event::const_reference event) 	noexcept {
	for(auto& appender: appenders) {
		(* appender).event = std::ref(event);
		(* appender).Append();
		(* appender).event = std::nullopt;
	}
	return *this;
}
Logger::~Logger() 													noexcept 
{	for(auto& appender: appenders) std::weak_ptr<Logger>{}.swap((* appender).logger);		};

Logger::reference Logger::PushAppender(std::shared_ptr<Appender> appender) 	noexcept {
	appenders.emplace_back(appender);
	(* appender).logger = weak_from_this();
	return 		*this;
};
Logger::reference Logger::EraseAppender(std::shared_ptr<Appender> appender) 	noexcept {
	appenders.remove(appender);
	std::weak_ptr<Logger>{}.swap((* appender).logger);
	return 		*this;	
};
	
struct ConsoleAppender: Appender {
__KELPA_DEFINES_STRUCT_MEMBER_TYPES__(ConsoleAppender)
	static ConsoleAppender::shared_pointer Shared() 												noexcept;
	static ConsoleAppender::shared_pointer Shared(OutputLevel) 								noexcept;
	static ConsoleAppender::shared_pointer Shared(Formatter::shared_pointer) 						noexcept;
	static ConsoleAppender::shared_pointer Shared(OutputLevel, Formatter::shared_pointer) 	noexcept;

	virtual Appender::const_reference Append() 	const noexcept override;		
};
struct FileAppender: Appender {
__KELPA_DEFINES_STRUCT_MEMBER_TYPES__(FileAppender)
	static FileAppender::shared_pointer Shared() 												noexcept;
	static FileAppender::shared_pointer Shared(OutputLevel) 								noexcept;
	static FileAppender::shared_pointer Shared(Formatter::shared_pointer) 						noexcept;
	static FileAppender::shared_pointer Shared(OutputLevel, Formatter::shared_pointer) 	noexcept;

	virtual Appender::const_reference Append() 	const noexcept override;	
	std::ofstream 		Ofs;	
};	

Appender::const_reference ConsoleAppender::Append() const noexcept {
	if((* logger.lock()).filter < level) 
		return dynamic_cast<Appender const&>(*this);	
	std::osyncstream(std::cout) <<  (* formatter).Format(
		Formatter::Context { 
			.logger = (* logger.lock()).name,
			.level = level,
			.event = * event
		}
	);
	return dynamic_cast<Appender const&>(*this);
}	
ConsoleAppender::shared_pointer ConsoleAppender::Shared() 										noexcept 
{	return Shared(Default, Formatter::Shared());			}

ConsoleAppender::shared_pointer ConsoleAppender::Shared(OutputLevel __level) 				noexcept 
{	return Shared(__level, Formatter::Shared());						}

ConsoleAppender::shared_pointer ConsoleAppender::Shared(Formatter::shared_pointer __formatter)  noexcept 
{	return Shared(Default, __formatter);						}

ConsoleAppender::shared_pointer ConsoleAppender::Shared(OutputLevel __level, Formatter::shared_pointer __formatter) 	noexcept {
	auto pointer { std::make_shared<ConsoleAppender>() };
	(* pointer).level = 		__level;
	(* pointer).formatter = 	__formatter;
	return pointer;	
}	
FileAppender::shared_pointer FileAppender::Shared() 										noexcept 
{	return Shared(Default, Formatter::Shared());			}

FileAppender::shared_pointer FileAppender::Shared(OutputLevel __level) 				noexcept 
{	return Shared(__level, Formatter::Shared());						}

FileAppender::shared_pointer FileAppender::Shared(Formatter::shared_pointer __formatter)  	noexcept 
{	return Shared(Default, __formatter);						}

FileAppender::shared_pointer FileAppender::Shared(OutputLevel __level, Formatter::shared_pointer __formatter) 	noexcept {
	auto pointer { std::make_shared<FileAppender>() };
	(* pointer).level = 		__level;
	(* pointer).formatter = 	__formatter;
	return pointer;	
}	
Appender::const_reference FileAppender::Append() const noexcept {
	if((* logger.lock()).filter < level) 
		return dynamic_cast<Appender const&>(*this);	
	std::string processed =  (* formatter).Format(
		Formatter::Context { 
			.logger = (* logger.lock()).name,
			.level = level,
			.event = * event
		}
	);
	std::osyncstream(const_cast<FileAppender&>(*this).Ofs).write(reinterpret_cast<char const *>(processed.data()), processed.size());
	return dynamic_cast<Appender const&>(*this);
}	


	
}
}




#endif
