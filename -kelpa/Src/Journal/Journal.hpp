/** 
 * 		@Path 	Kelpa/Src/Journal/Util
 * 		@Brief	Predefined user-friendly 
 * 				log library related macros to reduce usage costs
 * @Dependency	./ 			{ Manager.hpp, OutputLevel.hpp, Logger.hpp }
 * 				../Utility/ { Singleton.hpp }
 * @Since 			2024/04/22
 * @Version			1st		 
 **/
 
 #ifndef __KELPA_JOURNAL_JOURNAL_HPP__
 #define __KELPA_JOURNAL_JOURNAL_HPP__
#include "./Manager.hpp"				/* imports ./ { 
	struct Manager 
	./ Journal.hpp { 
		struct Event, 
		./Event.hpp { struct Event }
	}
}*/

#include "../Utility/Singleton.hpp" 	/* imports ./ { 
	struct Singleton 
}*/


/* ##Util Macros: */
#define GlobalManager__() 			Kelpa::Utility::Singleton<Kelpa::Journal::Manager>::Get()
#define Logger__(__NAME__)			GlobalManager__().TryGet(#__NAME__).operator *()
#define RootLogger__()				Logger__(root)
#define SystemLogger__()			Logger__(system)

#define Stream__(__LOGGER__)					std::osyncstream(static_cast<std::stringstream&>(Kelpa::Journal::EventWrap { __LOGGER__}))
#define StdSprintf__(__LOGGER__, __FMT__, ...) 	(* __LOGGER__).BroadCast(Kelpa::Journal::Event::FromStdSprintf(__FMT__, __VA_ARGS__))
#define StdFormat__(__LOGGER__, __FMT__, ...)  	(* __LOGGER__).BroadCast(Kelpa::Journal::Event::FromStdFormat(__FMT__, __VA_ARGS__))


namespace Kelpa {
namespace Journal {
	
struct EventWrap {
	EventWrap(
		Logger::shared_pointer 	__logger, 
		Event 					__event = Event::FromString(std::string {})) 
	noexcept: logger(__logger), event(std::move(__event)) {};
	~EventWrap() 								noexcept {
		event.message = 	std::move(buffer.str());
		(* logger)			.BroadCast(event);
	}
	constexpr operator std::stringstream&() 	noexcept 
	{	return buffer;			}
	std::stringstream 			buffer;
	Logger::shared_pointer 		logger;
	Event 						event;
};
	
	
}
}
 
 
 
 #endif
