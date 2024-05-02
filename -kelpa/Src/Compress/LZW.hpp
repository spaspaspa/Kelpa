/** 
 * 		@Path 	Kelpa/Src/Compress/LZW.hpp
 * 		@Brief	Using LZW algorithm to compress text data
 * 		@Dependency		../Utility/Interfaces.hpp
 * 						
 *		@Since 	2024/04/25
 		@Version 1st
 **/
 
 
#ifndef __KELPA_COMPRESS_LZW_HPP__
#define __KELPA_COMPRESS_LZW_HPP__

#include <string>					/* imports ./ { 
	std::string, 
	std::char_traits 
}*/
#include <vector>					/* imports ./ { 
	std::vector 
}*/
#include <unordered_map>			/* imports ./ { 	
	std::unordered_map 
}*/
#include <ranges>					/* imports ./ { 
	std::ranges::iota_view, 
	std::views::filter 
}*/
#include <limits>					/* imports ./ { 
	std::numeric_limits 
}*/
#include <concepts>					/* imports ./ { 
	std::convertible_to, 
	./iterator/ { 
		std::input_iterator, 
		std::output_iterator 
	}
}*/
#include "../Utility/Interfaces.hpp"/* imports ./ { 
	struct Reader, 
	struct Writer
}*/
namespace Kelpa 	{
namespace Compress 	{
namespace Detail {	
struct Dictionary {
	typedef unsigned short 						CodeT;
	typedef std::string							SeqT;
	typedef typename std::string::value_type 	CharT;
	
	struct Encapsulates {
		CodeT			code;
		SeqT			seq;
	};
	
	bool Insert(SeqT const& seq) noexcept {
		if(Container.size() >= std::numeric_limits<CodeT>::max()) {
			std::fprintf(stderr, "Overflow");
			std::quick_exit(EXIT_FAILURE);
		}
		if(WhereSeq.contains(seq)) return false;

		auto 	code { static_cast<CharT>(Container.size()) };

		WhereCode	.emplace(code, Container.size());
		WhereSeq	.emplace(seq, Container.size());
		Container	.emplace_back(code, seq);

		return true;
	}
	
	Dictionary() noexcept {
		for(char Char: std::ranges::iota_view {0, 256} 
			| std::views::filter([] (char Char) {
				return std::isprint(Char) || std::isspace(Char) || std::iscntrl(Char) || std::isblank(Char);
			})) Insert(std::string(1ull, Char));
	}
	
	std::vector<Encapsulates>				Container;
	std::unordered_map<SeqT, std::size_t>	WhereSeq;
	std::unordered_map<CodeT, std::size_t>	WhereCode;
};

}		//namespace Detail
	
namespace LZW {
	
template <
	std::input_iterator InputIt,	
	std::output_iterator<typename std::char_traits<Detail::Dictionary::CodeT>::char_type> OutputIt>
requires std::convertible_to<
	typename std::iterator_traits<InputIt>::value_type, 
	typename Detail::Dictionary::CharT
>	struct Writer : Utility::Writer<Writer<InputIt, OutputIt>> {

	typedef typename Detail::Dictionary::CodeT						code_type;
	typedef typename Detail::Dictionary::SeqT						sequence_type;
	typedef typename Detail::Dictionary::CharT						char_type;
	typedef 		InputIt											input_iterator;
	typedef 		OutputIt										output_iterator;
	
	template <typename InputIt_, typename OutputIt_>
	explicit constexpr Writer(InputIt_ __first, InputIt_ __last, OutputIt_ __out) noexcept
		: first(__first)
		, last(__last)
		, out(__out) {}	
	auto Write() noexcept -> typename std::iterator_traits<OutputIt>::difference_type;
	
	InputIt 		first;
	InputIt 		last;
	OutputIt 		out;	
	
};
template <typename InputIt_, typename OutputIt_> Writer(InputIt_, InputIt_, OutputIt_) -> Writer<InputIt_, OutputIt_>;

template <
	std::input_iterator InputIt,	
	std::output_iterator<typename std::char_traits<Detail::Dictionary::CodeT>::char_type> OutputIt>
requires std::convertible_to<
	typename std::iterator_traits<InputIt>::value_type, 
	typename Detail::Dictionary::CharT
> 
auto Writer<InputIt, OutputIt>::Write() noexcept 
	 -> typename std::iterator_traits<OutputIt>::difference_type {
	auto 					out_ 	= out;
	std::vector<char_type> 	Buffer;

	while(first != last) 
		Buffer.emplace_back(* first ++);

	std::size_t 		index 	{};
	std::string 		Pattern {};
	char_type 			Char;
	Detail::Dictionary 	Dict;

	while(index < Buffer.size()) {
		Pattern = Buffer[index];
		Char 	= Buffer[index + 1];
		while(!Dict.Insert(Pattern)) {
			Pattern += Char;
			index 	++;
			Char 	= Buffer[index + 1];
		}
		Pattern.erase(std::prev(Pattern.end()));
		* out ++ = Dict.Container[Dict.WhereSeq[Pattern]].code;
	}	 
	return std::distance(out_, out);
}

template <
	std::input_iterator InputIt,	
	std::output_iterator<typename Detail::Dictionary::CharT> OutputIt>
requires std::convertible_to<
	typename std::iterator_traits<InputIt>::value_type, 
	typename Detail::Dictionary::CodeT
>	struct Reader : Utility::Reader<Reader<InputIt, OutputIt>>{

	typedef typename Detail::Dictionary::CodeT						code_type;
	typedef typename Detail::Dictionary::SeqT						sequence_type;
	typedef typename Detail::Dictionary::CharT						char_type;
	typedef 		InputIt											input_iterator;
	typedef 		OutputIt										output_iterator;
	
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
	std::output_iterator<typename Detail::Dictionary::CharT> OutputIt>
requires std::convertible_to<
	typename std::iterator_traits<InputIt>::value_type, 
		typename Detail::Dictionary::CodeT
> 
auto Reader<InputIt, OutputIt>::Read() noexcept 
	 -> typename std::iterator_traits<OutputIt>::difference_type {
	auto 					out_ 	= out;
	std::vector<code_type> 	Buffer;

	while(first != last) 
		Buffer.emplace_back(* first ++);

	sequence_type 		Current {};
	sequence_type 		Previous {};
	Detail::Dictionary 	Dict;

	for(auto Code: Buffer) {
		Previous 	= std::move(Current);
		Current 	= Dict.Container[Dict.WhereCode[Code]].seq;
		Previous 	+= Current.front();
		Dict.Insert(Previous);
		for(auto Char: Current) 
			* out ++ = Char;
	}
	return std::distance(out_, out);
}


}		//namespace	LZW
}		//namespace Compress	
}		//namespace Kelpa

#endif
