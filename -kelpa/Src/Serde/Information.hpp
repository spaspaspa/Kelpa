/** 
 * @Path		Kelpa/Src/Serde/Information.hpp 
 * @Brief		A structure that stores all the registered metadata of a class, 
 * 				type known at compile time
 * @Dependency	./ { Field.hpp, Codec.hpp }
 * 				../Utility/ { Concepts.hpp, StringLiteral.hpp }
 * @Since		2024/04/22
 * @Version     1st
 **/
 
#ifndef __KELPA_SERDE_INFORMATION_HPP__
#define __KELPA_SERDE_INFORMATION_HPP__

#include <vector>										/* vector/ { 
	std::vector 
}*/
#include <unordered_map>								/* unordered_map/ { 
	std::unordered_map 
}*/
#include "../Utility/StringLiteral.hpp"					/* imports ./ { 
	struct StringLiteral 
}*/
#include "./Codec.hpp"									/* imports ./ { 
	struct CodecTraits, 
	struct CodecFactory 
}*/
namespace Kelpa {
namespace Serde {

template <Utility::StringLiteral Type> struct 	TypeReflect;

template <typename T> struct 					Information;
template <typename T> struct 					Information<std::vector<T>>;
template <typename T, typename K> struct 		Information<std::unordered_map<T, K>>;

struct 											PrimitiveTag	{};
struct 											ComplexTag		{};
struct 											StructureTag: 	ComplexTag	{};
struct 											ArrayTag: 		ComplexTag	{};
struct 											DictionaryTag: 	ComplexTag	{};

struct 											ArrayDigest		{};
struct 											DictionaryDigest{};

/** 
 * <Primitive/Structure DataType Register Macro> 
 * 			is defined in ./Util	
 **/
template <typename T> struct Information<std::vector<T>> {
	using Catetory = 									ArrayTag;	
	static constexpr typename CodecTraits::name_type 	name = "array";
	static typename CodecTraits::code_type 				code;				
}; 	
template <typename T>
typename CodecTraits::code_type Information<std::vector<T>>::code { CodecFactory::HashCode<ArrayDigest>() };

template <typename T, typename K> struct Information<std::unordered_map<T, K>> {
	using Catetory = 										DictionaryTag;
	static constexpr typename CodecTraits::name_type 		name = "dictionary";
	static typename CodecTraits::code_type 					code;
};
template <typename T, typename K> CodecTraits::code_type Information<std::unordered_map<T, K>>::code { CodecFactory::HashCode<DictionaryDigest>() };

}
}

#endif
