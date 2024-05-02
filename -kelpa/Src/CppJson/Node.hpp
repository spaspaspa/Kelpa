/** 
 * 		@Path 	Kelpa/Src/CppJson/Node.hpp
 * 		@Brief	Define the JSON data structure, a type-safe polymorphic Union type
 * 		@Dependency	../Utility/Ignore.hpp
 * 					../Utility/Error.hpp
 * 					../Utility/Concepts.hpp
 * 		@since	2024/04/24
 * 		@Version 1st
 **/
 
#ifndef __KELPA_CPPJON_NODE_HPP__
#define __KELPA_CPPJON_NODE_HPP__

#include <variant>								/* imports ./ { 
	std::variant, 
	std::get, std::holds_alternative
	std::monostate }*/
#include <vector>								/* imports ./ { 
	std::vector 
}*/
#include <string>								/* imports ./ { 
	std::string, 
	./string_view { std::string_view }
}*/	
#include <unordered_map>						/* imports ./ { 
	std::unordered_map
}*/
#include <utility>								/* imports ./ { 
	std::as_const 
}*/
#include <functional>							/* imports ./ { 
	std::invoke 
}*/
#include "../Utility/Ignore.hpp"				/* imports ./ { 
	struct Ignore,
	./tuple/ {std::ignore} 
}*/
#include "../Utility/Error.hpp"					/* imports ./ {
	#define Assert__()
}*/
#include "../Utility/Concepts.hpp"				/* imports ./ { 
	concept any_of
}*/
namespace Kelpa {
namespace CppJson {
	
struct Node {
	typedef /*Utility::Ignore*/struct panic {} 		Panic; 
	typedef std::monostate 							None;
	
	typedef std::nullptr_t 							Null;
	typedef bool 									Boolean;
	typedef int										Integer;
	typedef double									Double;
	typedef std::vector<Node>						Array;
	typedef std::string								String;
	typedef std::unordered_map<std::string, Node> 	Object;	
										
	typedef std::variant<
				std::monostate,	
				std::nullptr_t,
				bool, int, double,
				std::string,
				std::vector<Node>,
				std::unordered_map<std::string, Node>,
				Panic
	> 												Variant;
	
	Variant 										data;
	template <typename T> constexpr bool Holds() const noexcept
	{ 	return std::holds_alternative<T>(data); 	}
	
	template <typename T> constexpr std::add_pointer_t<T> 		GetIf() & noexcept 
	{ 	return std::get_if<T>(std::addressof(data));	}
	template <typename T> constexpr std::add_pointer_t<T const> GetIf() const& noexcept 
	{ 	return std::get_if<T>(std::addressof(data));	}
		
	template<typename T>
	constexpr T& 		As() &noexcept
	{    return std::get<T>(data);	 }
	template<typename T>
	constexpr T&& 		As() && noexcept
	{    return std::get<T>(data);	 }
	template<typename T>
	constexpr const T& 	As() const& noexcept
	{    return std::get<T>(data);	 }
	template<typename T>
	constexpr const T&& As() const&& noexcept
	{    return std::get<T>(data);	 }
	
	template <typename T> requires requires { typename T::value_type; } && std::constructible_from<Node, T>
	static constexpr Node CollectAs(std::initializer_list<typename T::value_type> init) noexcept 
	{	return Node { T {std::move(init)} };					}			
	
	template <typename T, typename... Args > requires (
		requires { typename T::value_type; }
	&&  (std::convertible_to<Args, typename T::value_type> && ...))
	static constexpr Node CollectAs(Args&& ...args) noexcept 
	{	return Node { T { std::initializer_list<typename T::value_type>{std::forward<Args>(args) ...} } };	}
	
	template <typename T> requires std::constructible_from<Node, T>
	static constexpr Node From(T&& value) noexcept 
	{ 	return Node { std::forward<T>(value) };	}

	Node& operator[](std::string const& key) 	noexcept 
	{	return std::get<Object>(data)[key];									}
	Node const& operator[](std::size_t index) 	const noexcept 
	{	return std::get<Array>(data)[index];								}
	Node& operator[](std::size_t index) 		noexcept 
	{	return const_cast<Node&>(std::as_const(*this).operator[](index));	}
	
	Node&& Expect(std::string_view message) && noexcept 
	{	return const_cast<Node&&>(static_cast<Node const&>(*this).Expect(message));		}
	Node const& Expect(std::string_view message) const& noexcept {
		Assert__(!std::holds_alternative<Node::Panic>(data), message.data())
		return *this;		
	}	
	Node&& Unwrap() && noexcept 
	{	return const_cast<Node&&>(std::as_const(*this).Unwrap());	}
	Node const& Unwrap() const& noexcept {
		Assert__(!std::holds_alternative<Node::Panic>(data), "JSON Parse fail") 
		return *this;
	}	
	template <typename F, typename... Args>
	Node&& OrElse(F&& f, Args&& ...args) && noexcept 
	{	return const_cast<Node&&>(std::as_const(*this).OrElse(std::forward<F>(f), std::forward<Args>(args) ...));	}
	template <typename F, typename... Args>
	Node const& OrElse(F&& f, Args&& ...args) const& noexcept {
		if(std::holds_alternative<Node::Panic>(data)) {
			(void) std::invoke(std::forward<F>(f), std::forward<Args>(args) ...);
			std::quick_exit(EXIT_SUCCESS);
		}
		return std::move(*this);
	}	
};

#define __KELPA_CPPJSON_NODE_DATA_FIELD_XMACRO__(UNARY_FUNCTION) \
	UNARY_FUNCTION(Array)									\
	UNARY_FUNCTION(Object)									\
	UNARY_FUNCTION(Integer)									\
	UNARY_FUNCTION(Double)									\
	UNARY_FUNCTION(Boolean)									\
	UNARY_FUNCTION(None)									\
	UNARY_FUNCTION(Null)									\
	UNARY_FUNCTION(String)									\
	UNARY_FUNCTION(Variant)

#define __KELPA_CPPJSON_NODE_DATA_DIELD_TYPEDEF__(__TYPE__) \
	typedef typename Node::__TYPE__ 				__TYPE__;
__KELPA_CPPJSON_NODE_DATA_FIELD_XMACRO__(__KELPA_CPPJSON_NODE_DATA_DIELD_TYPEDEF__)
#undef 	__KELPA_CPPJSON_NODE_DATA_DIELD_TYPEDEF__
template <typename T> concept DataType = Utility::any_of<T, 
#define __KELPA_CPPJSON_NODE_DATA_TYPE_CONCEPT__(__TYPE__) __TYPE__,
	__KELPA_CPPJSON_NODE_DATA_FIELD_XMACRO__(__KELPA_CPPJSON_NODE_DATA_TYPE_CONCEPT__)
	decltype(std::ignore)			
>;
#undef __KELPA_CPPJSON_NODE_DATA_FIELD_XMACRO__
}
}

#endif
