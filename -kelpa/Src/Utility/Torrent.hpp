/** 
 * 		@Path	Kelpa/Src/Utility/Torrent.hpp
 * 		@Brief	Character stream class at the bit level
 * 		@Dependency	None
 * 		@Since  2024/04/23
 * 		@Version 1st
 **/
 
#ifndef __KELPA_UTILITY_TORRENT_HPP__
#define __KELPA_UTILITY_TORRENT_HPP__

#include <bitset>					/* imports ./ { 
	std::bitset, std::size_t
}*/
#include <utility>					/* imports ./ { 
	std::exchange 
}*/
#include "./Macros.h"				/* imports./ {
	#define __KELPA_DEFINES_STRUCT_MEMBER_TYPES__
}*/
namespace Kelpa {
namespace Utility {
	
	
template <std::size_t N = 2048> struct Torrent;
constexpr bool GET_BYTE(unsigned char byte, std::size_t index) 				noexcept 
{	return byte & (1u << (index ^ 0x7));	}
constexpr void SET_BYTE(unsigned char& byte, std::size_t index, bool value) 	noexcept {
	if(value) byte |= 1u << (index ^ 0x7);
	else byte &= ~(1u << (index ^ 0x7));
}


template <std::size_t N>	 struct AsBitArrayHandle {
	typedef bool value_type;
	
	std::reference_wrapper<Torrent<N>> 		proxy;
	
	AsBitArrayHandle const& Put(value_type value) 	const noexcept;
	AsBitArrayHandle const& Get(value_type &value) 	const noexcept;
	AsBitArrayHandle const& Ignore() 			const noexcept;
	AsBitArrayHandle const& Unput() 			const noexcept;
	AsBitArrayHandle const& Unget() 			const noexcept;
	value_type 				Get() 				const noexcept;
};

template <std::size_t N> struct AsByteArrayHandle {
	typedef unsigned char value_type;
	
	std::reference_wrapper<Torrent<N>> 		proxy;
	
	AsByteArrayHandle const& Put(value_type value) 		const noexcept;
	AsByteArrayHandle const& Get(value_type &value) 	const noexcept;
	AsByteArrayHandle const& Ignore() 			const noexcept;
	AsByteArrayHandle const& Unput() 			const noexcept;
	AsByteArrayHandle const& Unget() 			const noexcept;
	value_type 				 Get() 				const noexcept;	 
};
template <std::size_t N> 
struct Torrent {
__KELPA_DEFINES_STRUCT_MEMBER_TYPES__(struct Torrent)
	
	static constexpr std::size_t throttle { N > 8 ? N: 8 };	
	typedef std::ptrdiff_t								PositionType;
	typedef typename AsBitArrayHandle<N>::value_type 	BitType;
	typedef typename AsByteArrayHandle<N>::value_type 	ByteType;
	
	constexpr AsBitArrayHandle<N> AsBitArray() 	noexcept 
	{	return {std::ref(*this)};  	}

	constexpr AsByteArrayHandle<N> AsByteArray() noexcept 
	{	return {std::ref(*this)};  	}

	constexpr std::ptrdiff_t Tellp() const noexcept 
	{	return putSentry; 			}

	constexpr std::ptrdiff_t Tellg() const noexcept 
	{	return getSentry;			}

	constexpr std::ptrdiff_t Redirectp(std::ptrdiff_t position = -1) noexcept 
	{	return std::exchange(putSentry, position);	}		

	constexpr std::ptrdiff_t Redirectg(std::ptrdiff_t position = -1) noexcept 
	{	return std::exchange(getSentry, position);	}

	constexpr bool Empty() const noexcept 
	{	return getSentry >= putSentry;		}

	constexpr std::size_t WhereAlignp() const noexcept 
	{ 	return (putSentry + 1 + 7)/ 8; 		}

	constexpr std::size_t WhereAligng() const noexcept 
	{ 	return (getSentry + 1 + 7)/ 8; 		}

	constexpr bool HasFringep() const noexcept 
	{	return putSentry + 1 == throttle; 	}

	constexpr bool HasFringeg() const noexcept 
	{	return getSentry + 1 == throttle; 	}

	constexpr std::string ToString() const noexcept 
	{	return buffer.to_string().substr(throttle - putSentry - 1, throttle - getSentry - 1);	}
	
	std::bitset<N>	buffer 		{};
	std::ptrdiff_t				putSentry 	{ -1 };
	std::ptrdiff_t				getSentry 	{ -1 };
};
template <std::size_t N>
AsBitArrayHandle<N> const& AsBitArrayHandle<N>::Put(AsBitArrayHandle<N>::value_type value) const noexcept {
	auto& reference = static_cast<Torrent<N>&>(proxy);
	
	reference.buffer[++ reference.putSentry] = value;
	
	if(reference.HasFringep()) 
		reference.Redirectp();
	return *this;
}
template <std::size_t N>
AsBitArrayHandle<N> const& AsBitArrayHandle<N>::Get(AsBitArrayHandle<N>::value_type& value) const noexcept {
	auto& reference = static_cast<Torrent<N>&>(proxy);
	
	value = reference.buffer[++ reference.getSentry];
	
	if(reference.HasFringeg()) 
		reference.Redirectg();
	return *this;
}

template <std::size_t N>
AsBitArrayHandle<N>::value_type AsBitArrayHandle<N>::Get() const noexcept {
	return static_cast<Torrent<N>&>(proxy).buffer[++ static_cast<Torrent<N>&>(proxy).getSentry];
}

template <std::size_t N>
AsBitArrayHandle<N> const& AsBitArrayHandle<N>::Ignore() const noexcept {
	(void) (std::ignore = Get());
	return *this;
}

template <std::size_t N>
AsBitArrayHandle<N> const& AsBitArrayHandle<N>::Unput() const noexcept {
	auto& reference = static_cast<Torrent<N>&>(proxy);
	if(~reference.putSentry) 
		reference.buffer[reference.putSentry --] = false;
	return *this;
}

template <std::size_t N>
AsBitArrayHandle<N> const& AsBitArrayHandle<N>::Unget() const noexcept {
	auto& reference = static_cast<Torrent<N>&>(proxy);
	if(~reference.getSentry) reference.getSentry --;
	return *this;
}

template <std::size_t N>
AsByteArrayHandle<N> const& AsByteArrayHandle<N>::Put(AsByteArrayHandle<N>::value_type value) const noexcept {
	for(std::size_t index {}; index < 8; index ++)
		AsBitArrayHandle<N> { proxy }.Put(GET_BYTE(value, 8 - index - 1));
	return *this;	
}

template <std::size_t N>
AsByteArrayHandle<N> const& AsByteArrayHandle<N>::Get(AsByteArrayHandle<N>::value_type& value) const noexcept {
	for(std::size_t index {}; index < 8; index ++)
		SET_BYTE(value, 8 - index - 1, AsBitArrayHandle<N> { proxy }.Get());
	return *this;
}

template <std::size_t N>
AsByteArrayHandle<N> const& AsByteArrayHandle<N>::Ignore() const noexcept {
	(void) (std::ignore = Get());
	return *this;
}	

template <std::size_t N>
AsByteArrayHandle<N>::value_type AsByteArrayHandle<N>::Get() const noexcept {
	value_type value {};
	(void) Get(value);
	return value;
}
template <std::size_t N>
AsByteArrayHandle<N> const& AsByteArrayHandle<N>::Unput() const noexcept {
	auto& reference = static_cast<Torrent<N>&>(proxy);
	if(reference.putSentry + 1 >= 8) 
		for(std::size_t index {}; index < 8; index ++) reference.buffer[reference.putSentry --] = false;
	return *this;
}
template <std::size_t N>
AsByteArrayHandle<N> const& AsByteArrayHandle<N>::Unget() const noexcept {
	auto& reference = static_cast<Torrent<N>&>(proxy);
	if(reference.getSentry + 1 >= 8) reference.getSentry -= 8; 
	return *this;
}		
	
}
}



#endif
