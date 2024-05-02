/** 
 * @Path		Kelpa/Src/Serde/Prelude.hpp 
 * @Brief		Pre-import basic registration type information to reduce the mental burden of 
 * 				additional registration basic types
 * @Dependency	./ { Util.hpp }
 * @Since		2024/04/22
 * @Version     1st
 **/
 
#ifndef __KELPA_SERDE_PRELUDE_HPP__
#define __KELPA_SERDE_PRELUDE_HPP__
#include <limits>						/* imports ./ { 
	std::numeric_limits 
}*/
#include "./Util.hpp"					/* imports ./ { 
	#define XXXRegister__()
	./Codec.hpp/ { struct CodecTraits, struct CodecFactory }
}*/

PrimitiveRegister__(char)				
PrimitiveRegister__(unsigned char)		
PrimitiveRegister__(short)				
PrimitiveRegister__(unsigned short)			
PrimitiveRegister__(int)				
PrimitiveRegister__(unsigned int)		
PrimitiveRegister__(long)				
PrimitiveRegister__(unsigned long)		
PrimitiveRegister__(long long)			
PrimitiveRegister__(unsigned long long)	
PrimitiveRegister__(bool)				
PrimitiveRegister__(float)				
PrimitiveRegister__(double)				
PrimitiveRegister__(std::nullptr_t)

#define DynamicCodecInfos__() \
Kelpa::Serde::CodecFactory::RegisterType<char> 				{};		\
Kelpa::Serde::CodecFactory::RegisterType<unsigned char> 	{};		\
Kelpa::Serde::CodecFactory::RegisterType<short> 			{};		\
Kelpa::Serde::CodecFactory::RegisterType<unsigned short> 	{};		\
Kelpa::Serde::CodecFactory::RegisterType<int> 				{};		\
Kelpa::Serde::CodecFactory::RegisterType<unsigned int> 		{};		\
Kelpa::Serde::CodecFactory::RegisterType<long> 				{};		\
Kelpa::Serde::CodecFactory::RegisterType<unsigned long> 	{};	  	\
Kelpa::Serde::CodecFactory::RegisterType<long long> 		{};		\
Kelpa::Serde::CodecFactory::RegisterType<unsigned long long>{};		\
Kelpa::Serde::CodecFactory::RegisterType<bool> 				{};		\
Kelpa::Serde::CodecFactory::RegisterType<float> 			{};		\
Kelpa::Serde::CodecFactory::RegisterType<double> 			{};		\
Kelpa::Serde::CodecFactory::RegisterType<std::nullptr_t>	{};		\
Kelpa::Serde::CodecFactory::nameOf.emplace(							\
	Kelpa::Serde::CodecFactory::HashCode<Kelpa::Serde::ArrayDigest>(), 			\
	"array"); 														\
Kelpa::Serde::CodecFactory::sizeOf.emplace(							\
	Kelpa::Serde::CodecFactory::HashCode<Kelpa::Serde::ArrayDigest>(), 			\
	std::numeric_limits<Kelpa::Serde::CodecTraits::size_type>::max());	\
Kelpa::Serde::CodecFactory::nameOf.emplace(							\
	Kelpa::Serde::CodecFactory::HashCode<Kelpa::Serde::DictionaryDigest>(), 		\
	"dictionary"); 													\
Kelpa::Serde::CodecFactory::sizeOf.emplace(							\
	Kelpa::Serde::CodecFactory::HashCode<Kelpa::Serde::DictionaryDigest>(), 		\
	std::numeric_limits<Kelpa::Serde::CodecTraits::size_type>::max());	



#endif
