/** 
 * @Path		Kelpa/Src/Serde/VariableTraits.hpp 
 * @Brief		The metadata information of variable type is extracted 
 * 				by type extraction and SPFINEA template technology 
 * 				to help type coding in serialization process
 * @Dependency	../Utility/ { Ignore.hpp }
 * @Since		2024/04/22
 * @Version     1st
 **/

#ifndef __KELPA_SERDE_VARIABLETRAITS_HPP__ 
#define __KELPA_SERDE_VARIABLETRAITS_HPP__

#include "../Utility/Ignore.hpp"	/* imports ./ { 
	struct Ignore 
}*/

namespace Kelpa {
namespace Serde {
	
/*	UnitType {};	*/
typedef Utility::Ignore	UnitType;

template <typename T> struct VariableTraits {
	using variable_type = 			T;
	using variable_pointer_type = 	T *;
};

template <typename T> struct VariableTraits<T &> : VariableTraits<T> {};
template <typename T> struct VariableTraits<T const &> : VariableTraits<T> {};
template <typename T> struct VariableTraits<T volatile &> : VariableTraits<T> {};
template <typename T> struct VariableTraits<T const volatile &> : VariableTraits<T> {};
template <typename T> struct VariableTraits<T &&> : VariableTraits<T> {};
template <typename T> struct VariableTraits<T const &&> : VariableTraits<T> {};
template <typename T> struct VariableTraits<T volatile &&> : VariableTraits<T> {};
template <typename T> struct VariableTraits<T const volatile &&> : VariableTraits<T> {};

template <typename T, typename Class>
struct VariableTraits<T Class::*> : VariableTraits<T> {
	static constexpr UnitType 			is_member {};
	using class_type = 					Class;
	using variable_pointer_type = 		T Class::*;
};

#define UNDECAYED_MEMBER_VARIABLE_POINTER(SPECIFIER)\
template <typename T, typename Class>												\
struct VariableTraits<const T Class::* SPECIFIER> : VariableTraits<T Class::*> {	\
	static constexpr UnitType 	is_const {};								\
	using variable_pointer_type = 		const T Class::*;							\
};																					\
template <typename T, typename Class>												\
struct VariableTraits<volatile T Class::* SPECIFIER> : VariableTraits<T Class::*> {	\
	static constexpr UnitType 	is_volatile {};								\
	using variable_pointer_type = 		volatile T Class::*;						\
};																					\
template <typename T, typename Class>												\
struct VariableTraits<const volatile T Class::* SPECIFIER> : VariableTraits<T Class::*> {\
	static constexpr UnitType 	is_const {};								\
	static constexpr UnitType 	is_volatile {};								\
	using variable_pointer_type = 		const volatile T Class::*;					\
};

UNDECAYED_MEMBER_VARIABLE_POINTER()
UNDECAYED_MEMBER_VARIABLE_POINTER(const)
UNDECAYED_MEMBER_VARIABLE_POINTER(volatile)
UNDECAYED_MEMBER_VARIABLE_POINTER(const volatile)
#undef UNDECAYED_MEMBER_VARIABLE_POINTER	
	
}
}

#endif
 
