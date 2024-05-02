/** 
 * 		@Path 	Kelpa/Src/CppJson/Load.hpp
 * 		@Brief	Parse the JSON format file to get the head node of 
 * 				the corresponding structure of the json data tree
 * 		@Dependency	../Utility/ { Functions.hpp, Error.hpp, ScopeGuard.hpp }
 * 					./Node.hpp
 * 		@since	2024/04/24
 * 		@Version 1st
 **/
 
#ifndef __KELPA_CPPJSON_LOAD_HPP__
#define __KELPA_CPPJSON_LOAD_HPP__

#include "./Node.hpp"					/* imports ./ { 
	struct Node 
	./Error.h { #define Assert__() }
}*/
#include "../Utility/Functions.hpp"		/* imports ./ { 
	FromChars() 
}*/
#include "../Utility/ScopeGuard.hpp"	/* imports ./ { 
	struct ScopeGuard
}*/
#include <stack>						/* imports ./ { 
	std::stack 
}*/
#include <sstream>						/* imports ./ { 
	std::stringstream 
}*/
#include <fstream>						/* imports ./ { 
	std::fstream 
	./ios_base/ { std::ios_base::flags }
}*/
#include <ranges>						/* imports ./ { 
	std::views::{ 
		std::filter, 
		std::transform }
}*/
#include <regex>						/* imports ./ { 
	std::regex_search, 
	std::cmatch
}*/
#include <filesystem>					/* imports ./ { 
	std::filesystem::path 
}*/
namespace Kelpa {
namespace CppJson {
namespace Detail {
constexpr char ToEscaped(char alpha) noexcept {
	switch (alpha) {
		case 'n':	return '\n';
		case 'r':	return '\r';
		case '0':	return '\0';
		case 't':	return '\t';
		case 'v':	return '\v';
		case 'f':	return '\f';
		case 'b':	return '\b';
		case 'a':	return '\a';
		default:	return alpha;
	}
}

void NestVerify(std::string_view context) noexcept {
	std::stack<std::string_view::size_type> stack;
	std::string_view 						braces { "[]{}()\"" };
	static constexpr auto 					N { std::string_view::npos };
	bool 									quotes { true };
	for(auto index: context
	| std::views::filter(	[&](char c) { return braces.find_first_of(c) != N;  })
	| std::views::transform([&](char c) { return braces.find_first_of(c) + (std::equal_to<>{} (c, '"') ? (quotes = !quotes) : 0U); })
	) {
		if(!(index & 1)) {
			stack.emplace(index);
			continue;
		}
		Assert__(!stack.empty() && index == stack.top() + 1, "JSON BRACES OR QUOTES NOT MATCH 1")
		stack.pop();
	}
	Assert__(stack.empty(), "JSON BRACES OR QUOTES NOT MATCH 2")	
}
std::stringstream& SkipWS(std::stringstream& torrent) noexcept {
	typedef typename std::stringstream::traits_type 		traits;
	while(~torrent.peek() && std::isspace(torrent.peek())) 
		(void) 	torrent.ignore(1, traits::eof());
	return 		torrent;
}

Node FromStream(std::stringstream& torrent) noexcept {
	typedef typename std::stringstream::traits_type 		traits;
	
	if(!~SkipWS(torrent).peek()) 			return { None{} };
	
	std::string_view lookUp { torrent.view().substr(torrent.tellg()) };
	
	if (lookUp.starts_with("true") || lookUp.starts_with("TRUE") || lookUp.starts_with("false") || lookUp.starts_with("FALSE")) {
		Boolean boolean;
		(void) (torrent >> std::boolalpha >> boolean);
		return { boolean };
	}
	
	if (lookUp.starts_with("null") || lookUp.starts_with("NULL")) {
		torrent.ignore(std::strlen("null"), traits::eof());
		return { nullptr };
	}
	
	if(std::isdigit(torrent.peek()) || torrent.peek() == '+' || torrent.peek() == '-') {
		std::regex 	regex 	{"[+-]?[0-9]+(\\.[0-9]*)?([eE][+-]?[0-9]+)?"};
		std::cmatch match 	{};
		
		Assert__(std::regex_search(lookUp.data(), lookUp.data() + lookUp.size(), match, regex), "REGEX SEARCH ERROR")
		
		std::string haystack { match.str() };
		torrent.ignore(haystack.size(), traits::eof());
		
		if (auto digit { Utility::FromChars<Integer>(haystack) 	})		return { * digit };
		if (auto digit { Utility::FromChars<Double>(haystack) 	})		return { * digit };			
	}
	
	char front 	{ static_cast<traits::char_type>(torrent.get()) };
	
	static constexpr char 		DoubleQuote   	{ '"' };
	static constexpr char 		OpenSquare 		{ '[' };
	static constexpr char 		OpenCurly 		{ '{' };
	static constexpr char 		CloseSquare 	{ ']' };
	static constexpr char 		CloseCurly 		{ '}' };
	static constexpr char 		Colon			{ ':' };
	static constexpr char 		Comma			{ ',' };	
	
	if (front == DoubleQuote) {
		String 	content;
		enum class 		mode : unsigned char { unescaped, escaped } phase { mode::unescaped };
		char 			alpha {};
		while (torrent.get(alpha)) {
			if (phase == mode::unescaped) {
				if (alpha == '\\') 				phase = mode::escaped;
				else if (alpha == DoubleQuote) 	return { std::move(content) };
				else 							content.push_back(alpha);
			} else {
				content.push_back(ToEscaped(alpha));
				phase = mode::unescaped;
			}
		}
	}	
	if (front == OpenSquare) {
		Array array;
		
		while (~torrent.peek()) {
			if (torrent.peek() == CloseSquare && torrent.ignore(1, traits::eof()))
				return { std::move(array) };
			array.emplace_back(FromStream(torrent).Expect("<Array value error>"));

			if (SkipWS(torrent).peek() == Comma) torrent.ignore(1, traits::eof());
		}
	}
	if (front == OpenCurly) {
		Object object;
		
		while (~torrent.peek()) {
			if (torrent.peek() == CloseCurly && torrent.ignore(1, traits::eof()))
				return { std::move(object) };
			
			Node 	keyNode = FromStream(torrent).Expect("<Object key error>");
			
			Assert__(keyNode.Holds<String>(), "JSON MAP KEY NOT STRING")
				
			if (SkipWS(torrent).peek() == Colon) 	torrent.ignore(1, traits::eof());
			
			Node 	valueNode = FromStream(torrent).Expect("<Object value error>"); 
			
			object.insert_or_assign(keyNode.As<String>(), std::move(valueNode));
			
			if (SkipWS(torrent).peek() == Comma) 	torrent.ignore(1, traits::eof());
		}
	}
	return { Node::Panic{} };								
}	
}

Node FromFile(std::filesystem::path path) noexcept {
	std::fstream fs { path.filename(), std::ios_base::binary | std::ios_base::in };
	Utility::ScopeGuard guard ([&] { if(fs.is_open()) fs.close(); });
	Assert__(fs.is_open(), "can not open file")
	auto torrent = std::move(std::stringstream {} << fs.rdbuf());
	Detail::NestVerify(torrent.view());
	return Detail::FromStream(torrent);
}
Node FromString(std::string_view text) noexcept {
	Detail::NestVerify(text);
	std::stringstream torrent {text.data()};
	return Detail::FromStream(torrent);
}

	
}
}

#endif
