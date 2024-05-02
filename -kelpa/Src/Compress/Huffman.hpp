/** 
 * 		@Path 	Kelpa/Src/Compress/Huffman.hpp
 * 		@Brief	Using huffman algorithm to compress binary data
 * 		@Dependency		../Utility/ {Torrent.hpp, SelfWrap.hpp, Interfaces.hpp, Functions.hpp }
 * 						
 *		@Since 	2024/04/25
 		@Version 1st
 **/
 
 
#ifndef __KELPA_COMPRESS_HUFFMAN_HPP__
#define __KELPA_COMPRESS_HUFFMAN_HPP__
#include <memory>							/* imports./ { 
	std::unique_ptr, 
	std::make_unique 
}*/
#include <unordered_map>					/* imports./ { 
	std::unordered_map
}*/
#include <queue>							/* imports ./ { 
	std::priority_queue, 
	std::deque
}*/
#include <functional>						/* imports ./ { 
	std::greater 
	std::hash
}*/
#include <concepts>							/* imports ./ { 
	std::input_iterator, 
	./iterator/{ std::iterator_traits }
}*/
#include <format>							/* imports ./ { 
	std::format_to, 
	std::format_string, ... 
}*/
#include <string>							/* imports ./{ 
	std::string, 
	std::string_view, 
	std::char_traits 
}*/
#include <sstream>							/* imports. / { 
	std::stringstream 
}*/
#include <numeric>							/* imports ./ { 
	std::reduce 
}*/
#include "../Utility/SelfWrap.hpp"			/* imports ./ { 
	struct SelfWrap 
	./exceptions/{ std::exception }
}*/
#include "../Utility/Torrent.hpp"			/* imports ./ { 
	struct Torrent 
}*/
#include "../Utility/Functions.hpp"			/* imports. / { 
	SetBitAt, 
	GetBitAt 
}*/
#include "../Utility/Interfaces.hpp"		/* imports ./ { 
	struct Writer, 
	struct Reader 
}*/
namespace Kelpa {
namespace Compress {
namespace Detail {

struct Node {
	typedef unsigned int 				frequency_type;
	typedef unsigned char 				character_type;
	typedef Node * 						pointer;
	typedef Node const *				const_pointer;
	typedef Node&						reference;
	typedef Node const&					const_reference;
	typedef std::unique_ptr<Node> 		unique_pointer;
	
	unique_pointer					left;
	unique_pointer					right;
	frequency_type					frequency {};
	character_type					character { '#' };
	
	static void Iterate(Node const& external, void(* Visit)(Node const&)) noexcept {
		Visit(external);
		if(Dangling(external)) 
			return;
		if(external.left) 
			Iterate(* external.left.get(), Visit);
		if(external.right) 
			Iterate(* external.right.get(), Visit);
	}
	
	static bool Dangling(Node const& external) noexcept 
	{	return !external.left && !external.right;		}
};	


	
}	//namespace Detail	
namespace Huffman {
	
	
struct CodingSheet {
	typedef typename Detail::Node::character_type 		character_type;
	typedef typename Detail::Node::frequency_type 		frequency_type;
	typedef typename Detail::Node::pointer 				pointer;
	typedef typename Detail::Node::const_pointer 		const_pointer;
	typedef typename Detail::Node::reference 			reference;
	typedef typename Detail::Node::const_reference 		const_reference;
	typedef typename Detail::Node::unique_pointer 		unique_pointer;	
	typedef struct code_type 		
	{ 
		unsigned short 				bytes; 
		unsigned char 				length; 		
	}													code_type;
	
	unsigned char 										Trailing;
	std::unordered_map<character_type, code_type>		Sheet;	
	unique_pointer										Root;
};

std::ostream& operator<<(std::ostream& Os, typename CodingSheet::code_type const& code) noexcept {
	return (
		Os << std::bitset<std::numeric_limits<unsigned short>::digits>(code.bytes)
			.to_string()
			.substr(
				std::numeric_limits<unsigned short>::digits - code.length, 
				std::numeric_limits<unsigned short>::digits
		)
	);
}

template <std::input_iterator InputIt> 
	requires std::convertible_to<
		typename std::iterator_traits<InputIt>::value_type, 
		typename Detail::Node::character_type
	> 
struct Encoder {
	typedef typename Detail::Node::character_type 		character_type;
	typedef typename Detail::Node::frequency_type 		frequency_type;
	typedef typename Detail::Node::pointer 				pointer;
	typedef typename Detail::Node::const_pointer 		const_pointer;
	typedef typename Detail::Node::reference 			reference;
	typedef typename Detail::Node::const_reference 		const_reference;	
	typedef typename Detail::Node::unique_pointer 		unique_pointer;
	typedef 			InputIt							input_iterator;

	template <std::input_iterator _InputIt>
	explicit constexpr Encoder(_InputIt __first, _InputIt __last) noexcept
		: first(__first)
		, last(__last) {}
	
	CodingSheet Encode() noexcept;
	
	std::unordered_map<character_type, frequency_type>	
						count;
	
	InputIt 			first;
	InputIt 			last;
};
template <std::input_iterator _InputIt> Encoder(_InputIt, _InputIt) -> Encoder<_InputIt>;

template <std::input_iterator InputIt> 
	requires std::convertible_to<
		typename std::iterator_traits<InputIt>::value_type, 
		typename Detail::Node::character_type
	>
CodingSheet Encoder<InputIt>::Encode() 		noexcept {
	while(first != last) count[* first ++] ++;
	
	std::priority_queue<pointer, std::deque<pointer>,
		decltype([] (const_pointer const x, const_pointer const y) {
			return std::greater<>{} ((* x).frequency, (* y).frequency);
		})>				Heap;
	for(auto [character, frequency]: count) {
		pointer 		p { new Detail::Node };
		
		(* p).character = character;
		(* p).frequency = frequency;
		
		Heap.emplace(p);
	}
	
	while(Heap.size() != 1) {
		pointer p { new Detail::Node };
		
		(* p).left.reset(Heap.top());		Heap.pop();
		(* p).right.reset(Heap.top());		Heap.pop();
		(* p).frequency = (*(*p).left).frequency + (*(*p).right).frequency;
		
		Heap.emplace(p);
	}
	
	CodingSheet 	sheet;
	sheet.Root.reset(Heap.top()); 			Heap.pop();
	
	auto recursivet = [&] (auto&& self, const_pointer const pointer, Utility::Torrent<> torrent) mutable {
		if(!pointer) 
			return;
		if(Detail::Node::Dangling(* pointer)) {
			while(!torrent.Empty()) 
				Utility::SetBitAt(
					sheet.Sheet[(* pointer).character].bytes, 
					std::numeric_limits<unsigned short>::digits - 1 - sheet.Sheet[(* pointer).character].length ++, 
					torrent.AsBitArray().Get()
				);
			return;
		}
		(void) torrent.AsBitArray()			.Put(false);
		self(self, (* pointer).left.get(), torrent);
		(void) torrent.AsBitArray().Unput()	.Put(true);
		self(self, (* pointer).right.get(), torrent);
	};
	recursivet(recursivet, sheet.Root.get(), Utility::Torrent<>{});
	
	std::size_t rest (
		std::accumulate(
			sheet.Sheet.cbegin(), sheet.Sheet.cend(), 0ull, 
			[&] (std::size_t const& init, auto&& entry) mutable {
				auto& 	[character, code] = entry;
				return (init + count[character] * code.length) & 0x00FFull;
	}));
	sheet.Trailing = static_cast<unsigned char>(rest & 0x7 ? rest & 0x7 : 8);	
	return sheet;
}
	
template <
	std::input_iterator InputIt,	
	std::output_iterator<unsigned char> OutputIt>
requires std::convertible_to<
	typename std::iterator_traits<InputIt>					::value_type, 
	typename std::char_traits<CodingSheet::character_type>	::char_type
> 	
struct Writer : Utility::Writer<Writer<InputIt, OutputIt>>	{
	typedef typename Detail::Node::character_type 		character_type;
	typedef typename Detail::Node::frequency_type 		frequency_type;
	typedef typename Detail::Node::pointer 				pointer;
	typedef typename Detail::Node::const_pointer 		const_pointer;
	typedef typename Detail::Node::reference 			reference;
	typedef typename Detail::Node::const_reference 		const_reference;
	typedef typename Detail::Node::unique_pointer 		unique_pointer;
	typedef unsigned char								ByteT;
	typedef 		InputIt								input_iterator;
	typedef 		OutputIt							output_iterator;
	
	template <typename InputIt_, typename OutputIt_>
	explicit constexpr Writer(InputIt_ __first, InputIt_ __last, OutputIt_ __out) noexcept
		: first(__first)
		, last(__last)
		, out(__out) {}
		
	auto Write(CodingSheet const& sheet) noexcept -> typename std::iterator_traits<OutputIt>::difference_type;
	
	InputIt 		first;
	InputIt 		last;
	OutputIt 		out;	
};
template <typename InputIt_, typename OutputIt_> Writer(InputIt_, InputIt_, OutputIt_) -> Writer<InputIt_, OutputIt_>;
template <
	std::input_iterator InputIt,	
	std::output_iterator<unsigned char> OutputIt>
requires std::convertible_to<
	typename std::iterator_traits<InputIt>					::value_type, 
	typename std::char_traits<CodingSheet::character_type>	::char_type
> 	 
auto Writer<InputIt, OutputIt>::Write(CodingSheet const& sheet) noexcept 
	 -> typename std::iterator_traits<OutputIt>::difference_type {
	auto 				out_ = out;
	* out ++ = static_cast<ByteT>( sheet.Trailing );
	
	Utility::Torrent<> 	torrent;
	
	auto recursivet = [&] (auto&& self, const_pointer const pointer) mutable {
		if(!pointer) 
			return;
		if(Detail::Node::Dangling(* pointer)) {
			(void) torrent.AsBitArray()	.Put(true);
			(void) torrent.AsByteArray().Put((* pointer).character);
			return;
		}		
		(void) torrent.AsBitArray().Put(false);
		
		self(self, (* pointer).left	.get());
		self(self, (* pointer).right.get());
	};
	recursivet(recursivet, sheet.Root.get());
	
	auto size 	= torrent.WhereAlignp();
	* out ++ 	= static_cast<ByteT>( size );
	
	for(unsigned char i{}; i < size; i ++) 
		* out ++ = static_cast<ByteT>(torrent.AsByteArray().Get());
	
	ByteT 					byte {};
	std::size_t 			index {};
	while(first != last) 	{
		for(auto iterator {0u}; iterator < sheet.Sheet.at(* first).length; iterator ++) {
			Utility::SetBitAt(byte, index, Utility::GetBitAt(sheet.Sheet.at(* first).bytes, iterator));
			if(!(index = (index + 1) & 0x7)) 
				* out ++ = byte;
		}
		first ++;
	}	
	if(index) 		{
		while(index < 8) 
			Utility::SetBitAt(byte, index ++, 0x00);
		* out ++ = byte;
	}
	return std::distance(out_, out);
}

template <
	std::input_iterator InputIt,	
	std::output_iterator<typename std::char_traits<CodingSheet::character_type>::char_type> OutputIt>
requires std::convertible_to<
	typename std::iterator_traits<InputIt>::value_type, 
	unsigned char
> 	
struct Reader : Utility::Reader<Reader<InputIt, OutputIt>> {
	typedef typename Detail::Node::character_type 		character_type;
	typedef typename Detail::Node::frequency_type 		frequency_type;
	typedef typename Detail::Node::pointer 				pointer;
	typedef typename Detail::Node::const_pointer 		const_pointer;
	typedef typename Detail::Node::unique_pointer 		unique_pointer;
	typedef 		unsigned char						ByteT;
	typedef 		InputIt								input_iterator;
	typedef 		OutputIt							output_iterator;
	
	template <typename InputIt_, typename OutputIt_>
	explicit constexpr Reader(InputIt_ __first, InputIt_ __last, OutputIt_ __out) noexcept
		: first(__first)
		, last(__last)
		, out(__out) {}	
	auto Read() noexcept -> typename std::iterator_traits<OutputIt>::difference_type;
	
	InputIt 		first;
	InputIt 		last;
	OutputIt 		out;	
}; 	
template <typename InputIt_, typename OutputIt_> Reader(InputIt_, InputIt_, OutputIt_) -> Reader<InputIt_, OutputIt_>;

template <
	std::input_iterator InputIt,	
	std::output_iterator<typename std::char_traits<CodingSheet::character_type>::char_type> OutputIt>
requires std::convertible_to<
	typename std::iterator_traits<InputIt>::value_type, 
	unsigned char
> 
auto Reader<InputIt, OutputIt>::Read() noexcept 
	 -> typename std::iterator_traits<OutputIt>::difference_type {
	auto 				out_ 	= out;
	CodingSheet 		sheet;
	
	sheet.Trailing 				= * first ++;
	unsigned char 		size 	= * first ++;
	
	Utility::Torrent<> torrent;
	
	for(auto i {0u}; i < size; i ++) 
		torrent.AsByteArray().Put(* first ++);
	
	auto recursivet = [&](auto&& self) mutable {
		if(torrent.AsBitArray().Get()) 
			return new Detail::Node { .character =  torrent.AsByteArray().Get() };

		pointer p = new Detail::Node;
		(* p).left.reset(self(self));
		(* p).right.reset(self(self));
		
		return p;
	}; 
	sheet.Root.reset(recursivet(recursivet));
	
	const_pointer 	p 		{ sheet.Root.get()	};
	std::size_t 	index 	{};
	ByteT 			byte   {};
	while(byte = * first, first ++ != last) while(true) {
		if(Detail::Node::Dangling(* p)) {
			* out ++ = (* p).character;
			p = sheet.Root.get();
		}
		p = !Utility::GetBitAt(byte, 8 - index - 1) 
			? (* p).left	.get() 
			: (* p).right	.get();
		if(	(	first == last && index + 1 == sheet.Trailing)
		|| !(index = (index + 1) & 0x07)
		) 	break;
	}
	if(Detail::Node::Dangling(* p)) 
		* out ++ = (* p).character;
		
	return std::distance(out_, out);
}
}	//namespace Huffman	
}	//namespace Compress	
}	//namespace Kelpa

namespace std {

template <> struct hash<typename Kelpa::Compress::Huffman::CodingSheet::code_type> {
	std::size_t operator()(typename Kelpa::Compress::Huffman::CodingSheet::code_type const& code) const noexcept {
		return std::hash<unsigned short>{} (code.bytes) | std::hash<unsigned char>{} (code.length);
	}
};

template<>  struct formatter<typename Kelpa::Compress::Huffman::CodingSheet::code_type> {  
    constexpr auto parse(std::format_parse_context& context) 					const noexcept 
	{  	return context.begin(); 							}  
	auto format(typename Kelpa::Compress::Huffman::CodingSheet::code_type const& t, std::format_context& context) 	const 
	{  	return std::format_to(context.out(), "{}", (std::stringstream{} << t).str());  	}  
};  

}

#endif
