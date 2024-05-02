/** 
 * @Path		Kelpa/Src/Serde/Codec.hpp 
 * @Brief		An abstract/singleton factory that encodes types 
 * 				and records other ancillary information
 * @Dependency	None
 * @Since		2024/04/22
 * @Version     1st
 **/
 
#ifndef __KELPA_SERDE_CODEC_HPP__
#define __KELPA_SERDE_CODEC_HPP__
#include <string>					/* imports string/ { 
	std::string, 
	std::string_view 
}*/
#include <unordered_map>			/* imports unordered_map/ { 
	std::unordered_map
}*/
#include <typeindex>				/* imports typeindex/ { 
	std::typeindex 
}*/
namespace Kelpa {
namespace Serde {
	
struct CodecTraits {
	using code_type = 		unsigned int;
	using size_type = 		unsigned long long;
	using extent_type = 	unsigned short;
	using name_type = 		std::string_view;
};	

template <typename T> struct 		Information;
struct CodecFactory {
template <typename T> struct 		RegisterType {
	RegisterType() noexcept {
		(void) CodecFactory::nameOf.emplace(Information<T>::code, Information<T>::name);
		(void) CodecFactory::sizeOf.emplace(Information<T>::code, sizeof std::declval<T>());
	}
};
	template <typename T>
	constexpr static CodecTraits::code_type HashCode() noexcept {
		std::size_t 	code { std::type_index(typeid(std::decay_t<T>)).hash_code() };	
		return static_cast<CodecTraits::code_type>(code >> 32) ^ static_cast<CodecTraits::code_type>(code);
	}
	
	static std::unordered_map<CodecTraits::code_type, CodecTraits::name_type> nameOf;
	static std::unordered_map<CodecTraits::code_type, CodecTraits::size_type> sizeOf;
};
std::unordered_map<CodecTraits::code_type, CodecTraits::name_type> CodecFactory::nameOf {};
std::unordered_map<CodecTraits::code_type, CodecTraits::size_type> CodecFactory::sizeOf {};

	
	
}
}


#endif
