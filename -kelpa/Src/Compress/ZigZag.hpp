/** 
 * 		@Path 	Kelpa/Src/Compress/Huffman.hpp
 * 		@Brief	The signed negative numbers are converted to unsigned numbers 
 * 				by the zigzag algorithm of protobuf, and it is convenient to 
 * 				compress bytes by varint algorithm
 * 		@Dependency		None
 * 						
 *		@Since 	2024/04/25
 		@Version 1st
 **/
 
#ifndef __KELPA_COMPRESS_ZIGZAG_HPP__
#define __KELPA_COMPRESS_ZIGZAG_HPP__
#include <concepts>			/* imports ./ { 
	std::signed_integral, 
	std::unsigned_integral, 
	std::make_signed, 
	std::make_unsigned 
}*/
namespace Kelpa {
namespace Compress {
	
template <std::signed_integral T> auto ToZigZag(T value) noexcept
	-> typename std::make_unsigned<T>::type {		
	return (value << 1) ^ (value >> (sizeof value * 8 - 1));		
}
template <std::unsigned_integral U> auto ToZigZag(U value) noexcept 
{ 	return value;		}


template <std::unsigned_integral U> auto FromZigZag(U value) noexcept 
	-> typename std::make_signed<U>::type {		
	return (value >> 1) ^ - static_cast<std::make_signed_t<U>>(value & 1);		
}
template <std::signed_integral T> auto FromZigZag(T value) noexcept 
{ 	return value; 		}
	
}
}

#endif
