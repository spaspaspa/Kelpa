/** 
 * 		@Path 	Kelpa/Src/CppJson/CppJson.hpp
 * 		@Brief	Integrate content under sibling files
 * 		@Dependency	./ { * } 
 * 		@since	2024/04/24
 * 		@Version 1st
 **/
 
#ifndef __KELPA_CPPJSON_CPPJSON_HPP__
#define __KELPA_CPPJSON_CPPJSON_HPP__
#include <ranges>
#include "./Node.hpp"				/* imports ./ { 
	struct Node 
}*/
#include "./Dump.hpp"				/* imports ./ * */
#include "./Load.hpp"				/* imports ./ * */
namespace Kelpa {
namespace CppJson {

template <typename Container> 			struct View 
{	static_assert(std::bool_constant<false>::value, "Container can't be iterated");	};

template <> struct View<Array>: public std::ranges::view_interface<View<Array>> {
	constexpr View() 					noexcept = default;
	constexpr View(Node const& jnode) 	noexcept
		: cbegin(std::get<Array>(jnode.data).cbegin())
		, cend  (std::get<Array>(jnode.data).cend()  ) {} 	
		
	constexpr auto begin() const noexcept { return 	cbegin; 	}	
	constexpr auto end()   const noexcept { return 	cend;	}	
	
	Array::const_iterator 			cbegin;
	Array::const_iterator 			cend;
};
template <> struct View<Object>: public std::ranges::view_interface<View<Object>> {
	View() 						noexcept = default;
	View(Node const& jnode) 	noexcept
		: cbegin(std::get<Object>(jnode.data).cbegin())
		, cend  (std::get<Object>(jnode.data).cend()  ) {} 	
	
	auto begin() const noexcept { return 	cbegin; 	}	
	auto end()   const noexcept { return 	cend;	}	
	
	Object::const_iterator 			cbegin;
	Object::const_iterator 			cend;
};
template <> struct View<String>: public std::ranges::view_interface<View<String>> {
	constexpr View() 					noexcept = default;
	constexpr View(Node const& jnode) 	noexcept
		: cbegin(std::get<String>(jnode.data).cbegin())
		, cend  (std::get<String>(jnode.data).cend()  ) {} 	
	
	constexpr auto begin() const noexcept { return 	cbegin; 	}	
	constexpr auto end()   const noexcept { return 	cend;	}	
	
	String::const_iterator 			cbegin;
	String::const_iterator 			cend;
};
	
}
}






#endif
