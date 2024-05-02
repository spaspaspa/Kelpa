/** 
 * 		@Path 	Kelpa/Src/Configure/Configure.hpp
 * 		@Brief	The configuration pool of configuration variables 
 * 				in the unified management system that supports CRUD operations
 * 		@Dependency		./ { Variable.hpp }
 * 						../CppJson/Node.hpp
 * 						../Utility/Error.h
 * 		@Since	2024/04/24
 * 		@Version 1st
 **/
 
#ifndef __KELPA_CONFIGURE_CONFIGURE_HPP__
#define __KELPA_CONFIGURE_CONFIGURE_HPP__
#include "../Utility/Functions.hpp"
#include <functional>				/* imports ./ { 
	std::reference_wrapper, 
	std::function
}*/
#include <optional>					/* imports ./ { 
	std::optional, 
	extern std::nullopt 
}*/
#include <algorithm>				/* imports ./ { 
	std::ranges::all_of 
	std::transform
}*/
#include <unordered_map>			/* imports ./ { 
	std::unordered_map 
}*/
#include <memory>					/* imports ./ { 
	std::unique_ptr, 
	std::make_unique 
}*/
#include "../Journal/Journal.hpp"		/* imports ./ { 
	#define Manager__(), 
	struct Logger, 
	struct Appender, 
	enum class OutputLevel 
}*/
#include "./Variable.hpp"			/* imports ./ { 
	struct Variable, 
	struct VariableBase 
}*/
#include "../CppJson/Node.hpp"		/* imports ./ { 
	struct Node 
}*/
#include "../Utility/Error.hpp"		/* imports ./ { 
	#define Assert__() 
}*/

namespace Kelpa {
namespace Configure {
	
struct 						Configure {
	template <typename T> static constexpr 
	std::reference_wrapper<Variable<T>> Emplace(
		std::string 			identifier, 
		T const& 				value,
		std::string const&		description
	) 														noexcept;
	
	static std::size_t 	Size() noexcept { 		return variables.size();		}
	static bool 		Empty() noexcept { 		return variables.empty();		}
	
	template <typename T> static constexpr 
	bool Remove(std::string const& identifier) 				noexcept;
	
	template <typename T> static constexpr 
	std::optional<std::reference_wrapper<Variable<T>>> LookUp(std::string identifier) noexcept;
	
	static constexpr void Load(CppJson::Node const& root) 	noexcept;

private:
	static std::unordered_map<std::string, std::unique_ptr<Detail::VariableBase>> 	variables;
};
std::unordered_map<std::string, std::unique_ptr<Detail::VariableBase>> 				Configure::variables {};




template <typename T> constexpr std::reference_wrapper<Variable<T>> Configure::Emplace(
	std::string 		identifier, 
	T const& 			value,
	std::string const&	description
) noexcept {
	Assert__(std::ranges::all_of(identifier, [](char c) { 
		return std::isdigit(c) || std::isalpha(c) || std::equal_to<>{} (c, '_') || std::equal_to<>{} (c, '.'); 
	}), "illegal name") 
	
	std::transform(identifier.begin(), identifier.end(), identifier.begin(), ::tolower);
	
	if(! variables.contains(identifier)) 
		variables.emplace(identifier, std::unique_ptr<Detail::VariableBase>(new Variable<T>{ identifier, value, description }));
	Assert__((* variables[identifier]).HashCode() == std::type_index(typeid(T)).hash_code(), "bad access") 
	
	return std::ref(reinterpret_cast<Variable<T> &>(* variables[identifier]));		
}

template <typename T> constexpr bool Configure::Remove(std::string const& identifier) noexcept {
	return static_cast<bool>(variables.erase(identifier));
}

template <typename T> constexpr std::optional<std::reference_wrapper<Variable<T>>> Configure::LookUp(std::string identifier) noexcept {
	std::transform(identifier.begin(), identifier.end(), identifier.begin(), ::tolower);
	
	if(! variables.contains(identifier) || (* variables[identifier]).HashCode() != std::type_index(typeid(T)).hash_code()) 	
		return std::nullopt;
	return std::ref(reinterpret_cast<Variable<T> &>(* variables[identifier])); 
}

constexpr void Configure::Load(CppJson::Node const& root) noexcept {
	auto recursivet = [&] (
		auto&& 						self,
		std::string const& 			preffix, 
		CppJson::Node const& 		root, 
		std::vector<std::pair<std::string, CppJson::Node>>& output
	) -> void {
		output.emplace_back(preffix, root);
		if(root.Holds<CppJson::Node::Object>()) 
			for(auto& [descriptor, jnode]: root.As<CppJson::Node::Object>())
				self(self, preffix.empty() ? descriptor: preffix + "." + descriptor, jnode, output);		
	};
	
	std::vector<std::pair<std::string, CppJson::Node>> output;
	recursivet(recursivet, std::string {}, root, output);
	
	for(auto& [descriptor, jnode]: output) 
		std::visit([&] (auto&& value) {
			using value_type = std::decay_t<decltype(value)>;
			if(auto reference = Configure::LookUp<value_type>(descriptor)) 
				(void) (* reference).get().Exchange(value);						
		}, jnode.data);
}	
	
	
}


/* 
	##namespace Kelpa::***Updater 
*/

struct LogConfigWatcher {
	static Configure::Variable<CppJson::Array> const& 	Handle;
	static std::size_t 									ListenID;

	constexpr LogConfigWatcher() 	noexcept;
};
Configure::Variable<CppJson::Array> const& LogConfigWatcher::Handle = Configure::Configure::Emplace<CppJson::Array>(
	"logs",
	CppJson::Array {
		CppJson::Node::CollectAs<CppJson::Object>({
			{"name", 		CppJson::Node { "fuck you"} },
			{"filter", 		CppJson::Node { 0 } },
			{"appenders", 	CppJson::Node::CollectAs<CppJson::Array>({
					CppJson::Node::CollectAs<CppJson::Object>({
						{"outputlevel", CppJson::Node { 0 }},
						{"type", 		CppJson::Node { 0 }},
						{"formatter", 	CppJson::Node { 
								"%d{%Y-%m-%d %H:%M:%S}%T%r%T[%n - %p]%T%f:%l:%c%T%F%T%t|%v%T%m%N" 
							}
						}
					})
				}) 
			}
		}),
	},
	"log description"
).get();

std::size_t LogConfigWatcher::ListenID = LogConfigWatcher::Handle.Listen(
	[] (CppJson::Node::Array const& old_members, CppJson::Node::Array const& new_members) {
		std::puts("GlobalLogListen CallBack Function");
	});	
	
constexpr LogConfigWatcher::LogConfigWatcher() noexcept {
	auto ParseAppender = [&] (CppJson::Object const& object) mutable -> Journal::Appender::shared_pointer {
		Assert__(object.contains("type"), "CONFIG FILE CONTENT MISSING TYPE FILED"); 
		Assert__(object.at("type").Holds<CppJson::Integer>(), "CONFIG FILE TYPE FIELD NOT INTEGER"); 

		auto updateAppender = [&] (Journal::Appender::shared_pointer abstractAppender) mutable {
			for(auto const& [descriptor, jnode]: object) {
				if(!descriptor.compare("type")) continue;
				
				if(!descriptor.compare("outputlevel")) {
					Assert__(jnode.Holds<CppJson::Integer>(), "CONFIG FILE OUTPUTLEVEL FIELD NOT INTEGER") 

					(* abstractAppender).level = static_cast<Journal::OutputLevel>(jnode.As<CppJson::Integer>());
				} else if(!descriptor.compare("formatter")) {
					Assert__(jnode.Holds<CppJson::String>(), "CONFIG FILE FORMATTER FIELD NOT STRING")
					
					(* abstractAppender).formatter = Journal::Formatter::Shared(jnode.As<CppJson::String>());
				} 		
			}
		};  
		if(object.at("type").As<CppJson::Integer>() == 0) {
			auto consoleAppender = Journal::ConsoleAppender::Shared();
			updateAppender(consoleAppender);
			return consoleAppender;
		} else if(object.at("type").As<CppJson::Integer>() == 1) {
			auto fileAppender = Journal::FileAppender::Shared();

			updateAppender(fileAppender);

			Assert__(object.contains("file"), "CONFIG FILE CONTENT MISSING FILE FILED") 
			Assert__(object.at("file").Holds<CppJson::String>(), "CONFIG FILE FILE FIELD NOT STRING") 		

			std::ofstream { object.at("file").As<CppJson::String>() }.swap((* fileAppender).Ofs);
			return fileAppender;	
		} else 
			Assert__(false, "CONFIG FILE HAS INVALID TYPE VALUE")			
	};
	auto ParseLogger = [&] (CppJson::Object const& object) mutable {
		auto logger = Journal::Logger::Shared();
		
		for(auto const& [descriptor, jnode]: object) {
			if(!descriptor.compare("name")) {
				Assert__(jnode.Holds<CppJson::String>(), "CONFIG FILE NAME FIELD NOT STRING")
				(* logger).name = jnode.As<CppJson::String>();
			} else if(!descriptor.compare("filter")) {
				Assert__(jnode.Holds<CppJson::Integer>(), "CONFIG FILE FILTER FIELD NOT INTEGER") 	
				(* logger).filter = static_cast<Journal::OutputLevel>(jnode.As<CppJson::Integer>());			
			} else if(!descriptor.compare("appenders")) {
				Assert__(jnode.Holds<CppJson::Array>(), "CONFIG FILE APPENDERS FIELD NOT ARRAY") 	
				for(auto const& appender: jnode.As<CppJson::Array>()) {
					Assert__(appender.Holds<CppJson::Object>(), "CONFIG FILE APPENDER FIELD NOT OBJECT") 
					(* logger).PushAppender(ParseAppender(appender.As<CppJson::Object>()));
				}
			}  
		}
		return logger;
	};
	for(auto const& object: Handle.value) 
		GlobalManager__().Bind(ParseLogger(object.As<CppJson::Object>()));
}	


}


#endif
