/** 
 * @Path		Kelpa/Src/Serde/Util.hpp 
 * @Brief		Pre-facility on using custom registration type informations
 * @Dependency	./ { Information.hpp, Field.hpp, Codec.hpp }
 * 				../Utility/ { Concepts.hpp, StringLiteral.hpp }
 * @Since		2024/04/22
 * @Version     1st
 **/
 
#ifndef __KELPA_SERDE_UTIL_HPP__
#define __KELPA_SERDE_UTIL_HPP__

#include "./Information.hpp"			/* imports ./ { 
	struct Information 
	./Codec.hpp/ { CodecTraits, CodecFactory }
	./Utility/StringLiteral.hpp { struct StringLiteral }
}*/
#include "./Field.hpp"					/* imports ./ { 
	struct Field, 
	./FunctionTraits.hpp { struct FunctionTraits },
	./VariableTraits.hpp { struct VariableTraits }
}*/
#include "../Utility/Functions.hpp"		/* imports ./ { 
	TypeName(...)
}*/
#include <format>						/* imports ./ { 
	std::format
} */
#include <iostream>						/* imports ./ { 
	std::ostream 
}*/
#include <memory>						/* imports ./ { 
	std::unique_ptr,
	std::make_unique
}*/
#define PrimitiveRegister__(__TYPE__) \
template <> struct Kelpa::Serde::Information<__TYPE__> { 													\
	using Catetory = 												Kelpa::Serde::PrimitiveTag;				\
	static constexpr typename Kelpa::Serde::CodecTraits::name_type 	name = #__TYPE__;						\
	static typename Kelpa::Serde::CodecTraits::code_type 			code;									\
}; typename Kelpa::Serde::CodecTraits::code_type Kelpa::Serde::Information<__TYPE__>::code {				\
	Kelpa::Serde::CodecFactory::HashCode<__TYPE__>() 														\
};

#define BeginStructureRegister__(__STRUCT__)\
template <> struct Kelpa::Serde::Information<__STRUCT__> {													\
	using Catetory = 												Kelpa::Serde::StructureTag;				\
	static constexpr typename Kelpa::Serde::CodecTraits::name_type 	name { #__STRUCT__ };					\
	static typename Kelpa::Serde::CodecTraits::code_type 			code;			
#define Field__(__POINTER__) 					Kelpa::Serde::Field { __POINTER__, #__POINTER__ }
#define STRUCT_VARIABLE_METADATAS(...) static constexpr auto variables = std::make_tuple(__VA_ARGS__);
#define STRUCT_FUNCTION_METADATAS(...) static constexpr auto functions = std::make_tuple(__VA_ARGS__);
#define EndStructureRegister__(__STRUCT__) 				};													\
typename Kelpa::Serde::CodecTraits::code_type Information<__STRUCT__>::code { 								\
	Kelpa::Serde::CodecFactory::HashCode<__STRUCT__>() 														\
};

#define TypeReflectRegister__(__TYPE__) typename<> Kelpa::Serde::TypeReflect<#__TYPE__> { typedef __TYPE__ type; }

#define __KELPA_TYPE_ATTRIBUTIVE_CONCEPT_DECLARE__(__ATTRIBUTE__) \
template <typename T> concept __ATTRIBUTE__ 													\
	= 	std::same_as<typename Information<T>::Catetory,  		struct __ATTRIBUTE__##Tag> 		\
|| 		std::derived_from<typename Information<T>::Category, 	struct __ATTRIBUTE__##Tag>;

#define __KELPA_MEMBER_FIELD_ATTRIBUTIVE_CONCEPT_DECLARE__(__ATTRIBUTE__)\
template <typename F, typename = void> struct is_##__ATTRIBUTE__##_field : std::false_type {};									\
template <typename F> struct is_##__ATTRIBUTE__##_field<F, std::void_t<decltype(F::is_##__ATTRIBUTE__)>> : std::true_type {};	\
template <typename F> constexpr bool is_##__ATTRIBUTE__##_field_v = is_##__ATTRIBUTE__##_field<F>::value;						\
template <typename F> concept __ATTRIBUTE__##_field = is_##__ATTRIBUTE__##_field_v<std::decay_t<F>>;


namespace Kelpa {
namespace Serde {
namespace Detail {	
__KELPA_TYPE_ATTRIBUTIVE_CONCEPT_DECLARE__(Primitive)
__KELPA_TYPE_ATTRIBUTIVE_CONCEPT_DECLARE__(Complex)
__KELPA_TYPE_ATTRIBUTIVE_CONCEPT_DECLARE__(Structure)
__KELPA_TYPE_ATTRIBUTIVE_CONCEPT_DECLARE__(Array)
__KELPA_TYPE_ATTRIBUTIVE_CONCEPT_DECLARE__(Dictionary)
#undef __KELPA_TYPE_ATTRIBUTIVE_CONCEPT_DECLARE__
	
__KELPA_MEMBER_FIELD_ATTRIBUTIVE_CONCEPT_DECLARE__(function)
__KELPA_MEMBER_FIELD_ATTRIBUTIVE_CONCEPT_DECLARE__(variable)
__KELPA_MEMBER_FIELD_ATTRIBUTIVE_CONCEPT_DECLARE__(member)
__KELPA_MEMBER_FIELD_ATTRIBUTIVE_CONCEPT_DECLARE__(const)
__KELPA_MEMBER_FIELD_ATTRIBUTIVE_CONCEPT_DECLARE__(volatile)
__KELPA_MEMBER_FIELD_ATTRIBUTIVE_CONCEPT_DECLARE__(noexcept)
__KELPA_MEMBER_FIELD_ATTRIBUTIVE_CONCEPT_DECLARE__(functor)
#undef __KELPA_MEMBER_FIELD_ATTRIBUTIVE_CONCEPT_DECLARE__

std::ostream& MakeAlignAt(std::ostream& Os, std::size_t tabs) noexcept {
	for(std::size_t count {}; count < tabs; count ++) (void) (Os << '\t');
	return Os;
}

}

template <Detail::Primitive T> 	std::ostream& print(std::ostream& Os, std::size_t tabs) noexcept;
template <Detail::Array T> 		std::ostream& print(std::ostream& Os, std::size_t tabs) noexcept;
template <Detail::Dictionary T> std::ostream& print(std::ostream& Os, std::size_t tabs) noexcept;
template <Detail::Structure T> 	std::ostream& print(std::ostream& Os, std::size_t tabs) noexcept;

template <Detail::Primitive T> 	std::ostream& print(std::ostream& Os, std::size_t tabs) noexcept {
	return Detail::MakeAlignAt(Os, tabs) << std::format("primitive {} <{}>", Information<T>::name, Information<T>::code);
}
template <Detail::Array T> 		std::ostream& print(std::ostream& Os, std::size_t tabs) noexcept {
	return print<typename T::value_type>(Detail::MakeAlignAt(Os, tabs) << "array\n", tabs + 1);
}
template <Detail::Dictionary T> std::ostream& print(std::ostream& Os, std::size_t tabs) noexcept {
	(void) (Detail::MakeAlignAt(Os, tabs) << std::format("primitive {} <{}>\n", Information<T>::name, Information<T>::code));
	(void) print<typename T::key_type>(Detail::MakeAlignAt(Os, tabs) << "key:\n", tabs + 1);
	endl(Os);
	(void) print<typename T::mapped_type>(Detail::MakeAlignAt(Os, tabs) << "mapped:\n", tabs + 1);
	return Os;
}

template <Detail::Structure T> std::ostream& print(std::ostream& Os, std::size_t tabs) noexcept {
	(void) (Detail::MakeAlignAt(Os, tabs) << std::format("structure {} <{}>\n", Information<T>::name, Information<T>::code));
	(void) (Detail::MakeAlignAt(Os, tabs) << "variables\n");
	std::apply([&]<typename... Pointers>(Field<Pointers> const&... fs) {
		([&]<typename Pointer>(Field<Pointer> const& f) {
			(void) (Detail::MakeAlignAt(Os, tabs + 1) << f.identifier);
			if constexpr(Detail::const_field<Field<Pointer>>) 		(void) (Os << " const");
			if constexpr(Detail::volatile_field<Field<Pointer>>)  	(void) (Os << " volatile");
			(void) (print<typename Field<Pointer>::traits::variable_type>(Os << '\n', tabs + 1) << '\n');
		}(fs), ...);
	}, Information<T>::variables);
	(void) (Detail::MakeAlignAt(Os, tabs) << "functions\n");
	std::apply([&]<typename... Pointers>(Field<Pointers> const&... fs) {
		([&]<typename Pointer>(Field<Pointer> const& f) {
			(void) (Detail::MakeAlignAt(Os, tabs + 1) << f.identifier);
			if constexpr(Detail::const_field<Field<Pointer>>) 		(void) (Os << " const");
			if constexpr(Detail::volatile_field<Field<Pointer>>)  	(void) (Os << " volatile");
			if constexpr(Detail::noexcept_field<Field<Pointer>>)  	(void) (Os << " noexcept");
			(void) (Detail::MakeAlignAt(Os, tabs + 1) << '\n' << Utility::TypeName<typename Field<Pointer>::traits::function_type>() << '\n');
		}(fs), ...);
	}, Information<T>::functions);
	return Os;
}
template <typename T> std::ostream& operator<<(std::ostream& Os, Information<T> const&) noexcept 
{	return print<T>(Os, std::size_t {});	}

template <Utility::StringLiteral string, typename... Args>
constexpr auto Construct(Args&&... args) noexcept 
{		return std::make_unique<typename TypeReflect<string>::type>(std::forward<Args>(args) ...);	}

}
}
 
 
 
#endif
