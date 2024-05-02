/** 
 * 		@Path 	Kelpa/Src/Compress/Huffman.hpp
 * 		@Brief	This module compresses unsigned integers using 
 * 				protobuf's varint compression algorithm
 * 		@Dependency		None
 * 						
 *		@Since 	2024/04/25
 		@Version 1st
 **/
 
#ifndef __KELPA_COMPRESS_VARINT_HPP__
#define __KELPA_COMPRESS_VARINT_HPP__

#include <vector>			/* imports ./ { 
	std::vector, 
	std::begin, 
	std::end 
}*/
#include <concepts>			/* imports. /{ 
	std::signed_integral, 
	std::unsigned_integral, 
	std::make_signed, 
	std::make_unsigned, 
	./iterator/ { 
		std::iterator_traits 
		std::input_iterator
		std::output_iterator
	}
}*/
namespace Kelpa {
namespace Compress {
	
	
template <std::unsigned_integral T>	std::vector<unsigned char> ToVarint(T value) noexcept {
	std::vector<unsigned char> result;
	
	while(value) {
		unsigned char byte = value & 0x7F;
		value >>= 0x08;
		if(value) byte |= 0x80;
		result.emplace_back(byte);
	}
	return result;
}
template <std::unsigned_integral T, std::output_iterator<unsigned char> OutputIt> 
auto ToVarint(T value, OutputIt first, OutputIt last = OutputIt()) noexcept 
	-> typename std::iterator_traits<OutputIt>::difference_type {
	
	auto begin = first;
	while(value && first != last) {
		unsigned char byte = value & 0x7F;
		value >>= 0x08;
		if(value) byte |= 0x80;
		* first ++ = byte;
	}	
	return std::distance(begin, first);
}	
template <std::unsigned_integral T = unsigned int>	T FromVarint(std::vector<unsigned char> const& v) noexcept {
	unsigned int 	result 	{};
	signed int 		shift 	{};
	for(auto byte: v) {
		result |= (byte & 0x7F) << shift;
		shift += 0x08;
		if(!(byte & 0x80)) break;
	}
	return result;
}
template <std::unsigned_integral T = unsigned int, std::input_iterator InputIt> 
	requires std::convertible_to<typename std::iterator_traits<InputIt>::value_type, unsigned char>
T FromVarint(InputIt first, InputIt last = InputIt()) noexcept {
	unsigned int 	result 	{};
	signed int 		shift 	{};
	while(first != last) {
		auto byte = * first ++;
		result |= (byte & 0x7F) << shift;
		shift += 0x08;
		if(!(byte & 0x80)) break;
	}
	return result;	
}	
	
}
}

#endif
