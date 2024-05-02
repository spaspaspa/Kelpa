/** 
 * 		@Path		Kelpa/Src/Journal/Manager.hpp
 * 		@Brief		Store and manage a series of named logger objects in batches, 
 * 					using json configuration files to feed specified Logger instances 
 * 					to the manager when the program starts to load and execute.
 * 		@Dependency	./ { Logger.hpp }
 * 					../Utility/Macros {  #define DEFINES_STRUCT_MEMBER_TYPES }
 * 					../Utility/Singleton { struct Singleton }
 * 		@Since 		2024/04/22
 * 		@Version	1st		
 **/
 
#ifndef __KELPA_JOURNAL_MANAGER_HPP__
#define __KELPA_JOURNAL_MANAGER_HPP__
#include <optional>					/* imports optional { 
	std::optional 
}*/
#include "../Utility/Macros.h" 		/* imports Macros/{
	#define __KELPA_DEFINES_STRUCT_MEMBER_TYPES__ 
}*/
#include "./LoggerAndAppender.hpp"			/* imports ./ { 
	struct Logger 
}*/
namespace Kelpa {
namespace Journal {
	
	
struct Manager {
__KELPA_DEFINES_STRUCT_MEMBER_TYPES__(Manager)
	Manager()			noexcept;
	std::optional<Logger::shared_pointer>	TryGet(std::string_view = "root") 	const noexcept;
	Manager::reference						Create(std::string_view) 			noexcept;
	Manager::reference						Bind(Logger::shared_pointer) 		noexcept;
	Manager::reference						Erase(std::string_view) 			noexcept;
	std::size_t 							Size() 								const noexcept;
	bool 									Contains(std::string_view) 			const noexcept;
	static std::unordered_map<std::string, Logger::shared_pointer>				Sink;
};
std::unordered_map<std::string, Logger::shared_pointer>				Manager::Sink {};
Manager::Manager() noexcept {
	Sink.reserve(8);
	(void) 	Create("root").Create("system");
}
std::optional<Logger::shared_pointer>	Manager::TryGet(std::string_view name) 			const noexcept {
	return Sink.contains(name.data()) ? 
			std::make_optional(const_cast<std::decay_t<decltype((* this).Sink)> &>(Sink)[name.data()]) 
		: 	std::nullopt;
}
Manager::reference						Manager::Create(std::string_view name) 			noexcept {
	auto logger = 	Logger::Shared(name);
	(* logger).PushAppender(ConsoleAppender::Shared());
	return 			Bind(logger);	
}
Manager::reference						Manager::Bind(Logger::shared_pointer logger) 	noexcept {
	(void)   Sink.emplace((* logger).name, logger);
	return * this;
}
Manager::reference						Manager::Erase(std::string_view name) 			noexcept {
	(void)	Sink.erase(name.data());
	return * this;
}
std::size_t 							Manager::Size() 								const noexcept {
	return Sink.size();
}
bool 									Manager::Contains(std::string_view name) 		const noexcept {
	return Sink.contains(name.data());
}
	
	
	
	
	
	
	
}
}

#endif
