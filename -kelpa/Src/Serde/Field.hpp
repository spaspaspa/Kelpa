/** 
 * @Path		Kelpa/Src/Serde/Field.hpp 
 * @Brief		The type and modification information of a given member pointer variable
 * 				are extracted by type extraction to form a metadata cluster, 
 * 				which is convenient for serialization
 * @Dependency	./ { FunctionTraits.hpp, VariableTraits.hpp }
 * 				../Utility/ { Concepts.hpp }
 * @Since		2024/04/22
 * @Version     1st
 **/
 
#ifndef __KELPA_SERDE_FIELD_HPP__
#define __KELPA_SERDE_FIELD_HPP__
#include <string>							/* imports string/string_view { 
	std::string_view 
}*/
#include <typeindex>						/* imports ./ { 
	std::typeindex::hash_code, 
	std::typeid, 
	std::typeinfo 
}*/
#include <iostream>							/* imports ./ { 
	std::ostream 
}*/
#include "./FunctionTraits.hpp"				/* imports ./ { 
	struct FunctionTraits
	typedef UnitType
}*/
#include "./VariableTraits.hpp"				/* imports ./ { 
	struct VariableTraits 
	typedef UnitType
}*/
#include "../Utility/Concepts.hpp"			/* imports ./ { 
	concept function_pointer
}*/
namespace Kelpa {
namespace Serde {
//UnitType {};

template <typename T, bool is_function> struct  FieldBase;
template <typename F> struct FieldBase<F, true>: FunctionTraits<F> {
	using traits = 					FunctionTraits<F>;
	static constexpr UnitType 		is_function {};
};
template <typename V> struct FieldBase<V, false>: VariableTraits<V> {
	using traits = 					VariableTraits<V>;
	static constexpr UnitType 		is_variable {};
};

template <typename T> struct Field;

template <typename T>	struct Field: FieldBase <
	std::decay_t<std::remove_pointer_t<T>>,
	static_cast<bool> (Utility::function_pointer<T>)
> {
	template <typename U> 
	constexpr Field(U&& __pointer, std::string_view __name) noexcept
		: pointer(std::forward<U>(__pointer))
		, identifier(__name.substr(__name.find_last_of(":") + 1)) {}
	T 					pointer 	{ nullptr };
	std::string_view 	identifier	{ "default" };
};
template <typename U> Field(U&&, std::string_view) -> Field<U>;

template <typename T> std::ostream& operator<<(std::ostream& Os, Field<T> const& field) noexcept 
{		return (Os << std::hex << field.pointer << "  " << field.identifier);		}

}
}

namespace std {

template <typename T> struct hash<Kelpa::Serde::Field<T>> {
	constexpr std::size_t operator()(Kelpa::Serde::Field<T> const& field) const noexcept {
		std::size_t result { std::hash<T>{} (field.pointer) | std::hash<std::string_view>{} (field.idnetifier) };
		if constexpr(requires { typename Kelpa::Serde::Field<T>::class_type; }) 
			result |= std::type_index(typeid(typename Kelpa::Serde::Field<T>::class_type)).hash_code();
		return result;	
	}	
};

template<typename T>  struct formatter<Kelpa::Serde::Field<T>> {  
    constexpr auto parse(std::format_parse_context& context) 		const noexcept 
	{  	return context.begin(); 							}  
	auto format(Kelpa::Serde::Field<T> const& field, std::format_context& context) 	const 
	{  	return std::format_to(context.out(), "{:X}  {}", field.pointer, field.identifier);  	}  
}; 

}




#endif
