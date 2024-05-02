/** 
 * 		@Path 	Kelpa/Src/CppJson/Load.hpp
 * 		@Brief	Unsequence a json data node into a formatted string
 * 		@Dependency	../Utility/ { Error.hpp }
 * 					./Node.hpp
 * 		@since	2024/04/24
 * 		@Version 1st
 **/
 
 
#ifndef __KELPA_CPPJSON_DUMP_HPP__
#define __KELPA_CPPJSON_DUMP_HPP__

#include "./Node.hpp"					/* imports ./ { 
	struct Node 
}*/
#include "../Utility/Error.hpp"			/* imports ./ {
	#define Assert__()
}*/
#include <iomanip>						/* imports ./ {
	std::quoted
}*/
#include <set>							/* imports ./ {
	std::set
}*/
namespace Kelpa {
namespace CppJson {
namespace Detail {
template <typename... Ts> struct Dumper: public Ts ... {
	using 			Ts::operator() ...;
	explicit Dumper(
		std::stringstream& 	Os, 
		signed int 			__indents, 
		signed int 			__throttle, 
		bool 				__sort, 
		Ts ...) 	noexcept
	: OsRefer(std::ref(Os))
	, indents(__indents < 0? 4 : 	__indents)
	, throttle(__throttle < 0? -1 : __throttle)
	, sort(__sort) {}
	
	constexpr std::stringstream& Indent() noexcept {
		if(! asvalue) static_cast<std::stringstream&>(OsRefer) << std::string(depth * indents, ' ');
		else asvalue = false;
		return static_cast<std::stringstream&>(OsRefer);
	}
	
	constexpr void operator()(Node const& root) noexcept 
	{	std::visit(*this, root.data);	}
	
	constexpr void operator()(Integer value) noexcept 
	{	(void) (Indent() << value);		}

	constexpr void operator()(Double value) noexcept 
	{	(void) (Indent() << value);		}
	
	constexpr void operator()(Boolean value) noexcept 
	{	(void) (Indent() << std::boolalpha << value);	}
	
	constexpr void operator()(String const& value) noexcept 
	{	(void) (Indent() << std::quoted(value));		}
	
	constexpr void operator()(None const&) noexcept 
	{	(void) (Indent() << "none");					}		
	
	constexpr void operator()(Null const&) noexcept 
	{	(void) (Indent() << "null");					}	
	
	constexpr void operator()(Array const& array) noexcept {
		if(~throttle && !throttle) return (void) (static_cast<std::stringstream &>(OsRefer) << "[...]");
		(void) (Indent() << "[\n");		depth ++; 		
		throttle --;
		
		for(auto it { array.cbegin() }; it != array.cend(); ++ it) {
			operator()(* it);
			if(std::next(it) != array.cend()) static_cast<std::stringstream&>(OsRefer) << ", ";
			static_cast<std::stringstream&>(OsRefer) << "\n";
		}
		
		depth --;		(void) (Indent() << "]");
	}
	
	constexpr void operator()(Object const& object) noexcept {
		auto foreach = [&](typename std::decay_t<decltype(object)>::value_type const& pair) {
			operator()(pair.first);
		
			static_cast<std::stringstream&>(OsRefer) << ": ";
			
			(void) std::exchange(asvalue, true);
			operator()(pair.second);
		};
		
		if(~throttle && !throttle) return (void) (static_cast<std::stringstream &>(OsRefer) << "[...]");
		(void) (Indent() << "{\n");		depth ++; 		
		throttle --;
		
		if(sort) {
			typedef typename std::decay_t<decltype(object)>::const_iterator Iterator;
			std::set<Iterator, decltype([] (Iterator one, Iterator two) {
				return std::less<>{} ( (* one).first, (* two).first );
			})> Indexer;
			for(auto it { object.cbegin() }; it != object.cend(); it ++) Indexer.emplace(it);
			for(auto& Iter: Indexer) {
				foreach(* Iter);
				if(std::next(Iter) != object.cend()) static_cast<std::stringstream&>(OsRefer) << ", ";
				static_cast<std::stringstream&>(OsRefer) << "\n";
			}
			depth --;		return (void) (Indent() << "}");
		}
		for(auto it { object.cbegin() }; it != object.cend(); it ++) {
			foreach(* it);
			if(std::next(it) != object.cend()) static_cast<std::stringstream&>(OsRefer) << ", ";
			static_cast<std::stringstream&>(OsRefer) << "\n";
		}		
		
		depth --;		return (void) (Indent() << "}");
	}	
	
private:
	std::reference_wrapper<std::stringstream> 	OsRefer;	
	signed int 									depth 		{ 0 };
	signed int									indents  	{ 4 };
	signed int									throttle 	{ -1 };
	bool										sort 		{ false };
	bool 		 								asvalue 	{ false };
};
template <typename... Ts> Dumper(std::stringstream&, signed int, signed int, bool, Ts ...) -> Dumper<Ts ...>;


}	

std::string ToString(Node const& root, signed int indent = 4, signed int throttle = -1, bool sort = true) noexcept { 
	std::stringstream torrent {};
	std::visit(	
		Detail::Dumper {
			torrent, 
			indent, 
			throttle, 
			sort, [] (auto&&) { Assert__(false, "UNHANDLED PANIC OCURRED"); } 
		}
	, root.data);
	return torrent.str();
}

std::ostream& operator<<(std::ostream& Os, Node const& root) noexcept {
	return (Os << ToString(root));
}
	
	
}
}

namespace std {
	
template<>  struct formatter<Kelpa::CppJson::Node> {  
    constexpr auto parse(std::format_parse_context& context) 					const noexcept 
	{  	return context.begin(); 							}  
	auto format(Kelpa::CppJson::Node const& t, std::format_context& context) 	const 
	{  	return std::format_to(context.out(), "{}", Kelpa::CppJson::ToString(t));  	}  
};
 
}


#endif
