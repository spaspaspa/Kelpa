/** 
 * @Path		Kelpa/Src/Serde/SerdeStream.hpp 
 * @Brief		c++ style character stream manager that overloads various types of 
 * 				extract and disextract operators
 * @Dependency	./ { Util.hpp }
 * 				../Utility/ { Concepts.hpp, Functions.hpp, ScopeGuard.hpp, SelfWrap.hpp }
 * @Since		2024/04/22
 * @Version     1st
 **/
 
 
#ifndef __KELPA_SERDE_SERDESTREAM_HPP__
#define __KELPA_SERDE_SERDESTREAM_HPP__

#include <sstream>						/* imports ./ { 
	std::stringstream, 
	./std::ios_base/{ * } 
}*/
#include <fstream>						/* imports ./ { 
	std::fstream 
}*/
#include "../Utility/SelfWrap.hpp"		/* imports ./ { 
	struct SelfWrap 
}*/
#include "../Utility/Functions.hpp"		/* imports ./ { 
	TypeName() 
}*/
#include "./Util.hpp"					/* imports ./ { 
	Detail::Concepts, Detail::is_xxx_field_v
	./Information.hpp { struct Information},
	./Field.hpp { struct Field }
	./XXX_Traits.hpp { struct XXX_Traits }
 }*/
#include "../Utility/ScopeGuard.hpp"	/* imports ./ { 
	struct ScopeGuard
}*/
#include "../Compress/Varint.hpp"		/* imports ./ { 
	to/fromVariint 
}*/
#include "../Compress/ZigZag.hpp"		/* imports. / { 
	to/fromZigZag 
}*/

namespace Kelpa {
namespace Serde {
	
struct SerdeStream: public std::stringstream {
	typedef Utility::SelfWrap<SerdeStream> self_type;

	explicit SerdeStream(std::ios_base::openmode mode) noexcept: std::stringstream(mode) {}

	SerdeStream() noexcept: basic_stringstream(std::ios_base::in | std::ios_base::out) {}
	
	explicit SerdeStream(
		std::basic_string<char_type, traits_type, allocator_type> const& s,
	    std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out
	) noexcept: std::stringstream(s, mode) {};

	explicit SerdeStream(
		std::basic_string<char_type, traits_type, allocator_type>&& s,
	    std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out 
	) noexcept: std::stringstream(s, mode) {};
	
	SerdeStream(std::ios_base::openmode mode, allocator_type const& a) 
		noexcept: std::stringstream(mode, a) {};
	
	template<typename SAlloc> SerdeStream(
		std::basic_string<char_type, traits_type, SAlloc> const& s,
	    std::ios_base::openmode mode, 
		allocator_type const& a
	) noexcept: std::stringstream(s, mode, a) {};

	template<typename SAlloc> SerdeStream(
		std::basic_string<char_type, traits_type, SAlloc> const& s,
	    allocator_type const& a
	) noexcept: SerdeStream(s, std::ios_base::in | std::ios_base::out, a) {}

	template<typename SAlloc> explicit SerdeStream(
		std::basic_string<char_type, traits_type, SAlloc> const& s,
		std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out 
	) noexcept: std::stringstream(s, mode) {};
	
	SerdeStream(SerdeStream&&) noexcept = 				default;
	SerdeStream& operator=(SerdeStream&&) noexcept = 	default;
	SerdeStream(SerdeStream const&) = 					delete;
	SerdeStream& operator=(SerdeStream const&) = 		delete;
	
	SerdeStream& Read(char_type* pointer, std::streamsize count) noexcept {
		return dynamic_cast<SerdeStream&>(std::stringstream::read(pointer, count));
	};
	SerdeStream& Write(char_type const *const_pointer, std::streamsize count) noexcept {
		return dynamic_cast<SerdeStream&>(std::stringstream::write(const_pointer, count));
	}	
/*
	template <Detail::Primitive T> std::basic_ostream<char_type>& operator<<(T value) noexcept {
		return 	Write(reinterpret_cast<char_type const *>(std::addressof(Information<T>::code)), sizeof(CodecTraits::code_type))
				.Write(reinterpret_cast<char_type const *>(std::addressof(value)), sizeof(T));
	}
*/	
	template <Detail::Primitive T> std::basic_ostream<char_type>& operator<<(T value) noexcept {
		(void) Write(reinterpret_cast<char_type const *>(std::addressof(Information<T>::code)), sizeof(CodecTraits::code_type));
	
		if constexpr(!std::integral<T>) 	
			return Write(reinterpret_cast<char_type const *>(std::addressof(value)), sizeof(T));

		auto v = Compress::ToVarint(Compress::ToZigZag(value));
		return Write(reinterpret_cast<char_type const*>(v.data()), v.size());	
	}
	template <Detail::Structure T> std::basic_ostream<char_type>& operator<<(T value) noexcept {
		(void) Write(reinterpret_cast<char_type const *>(std::addressof(Information<T>::code)), sizeof(CodecTraits::code_type));
	
		return std::apply([&]<typename...Pointers>(Field<Pointers> const&... fs) {
			return std::ref((operator<<(value.*(fs.pointer)), ...));
		}, Information<T>::variables);	
	}
	template <Detail::Array T> std::basic_ostream<char_type>& operator<<(T value) noexcept {
		auto extent { static_cast<CodecTraits::extent_type>(value.size()) };

		(void) Write(reinterpret_cast<char_type const *>(std::addressof(Information<T>::code)), sizeof(CodecTraits::code_type))
		.Write(reinterpret_cast<char_type const *>(std::addressof(Information<typename T::value_type>::code)), sizeof(CodecTraits::code_type))
		.Write(reinterpret_cast<char_type const *>(std::addressof(extent)), sizeof(CodecTraits::extent_type));		
	
		std::ranges::for_each(value, [&] (auto& v) { (void) operator<<(v); });

		return dynamic_cast<std::basic_ostream<char_type>&>(*this);				
	}
	template <Detail::Dictionary T> std::basic_ostream<char_type>& operator<<(T value) noexcept {
		auto extent { static_cast<CodecTraits::extent_type>(value.size()) };
	
		(void) Write(reinterpret_cast<char_type const *>(std::addressof(Information<T>::code)), sizeof(CodecTraits::code_type))
		.Write(reinterpret_cast<char_type const *>(std::addressof(Information<typename T::key_type>::code)), sizeof(CodecTraits::code_type))
		.Write(reinterpret_cast<char_type const *>(std::addressof(Information<typename T::mapped_type>::code)), sizeof(CodecTraits::code_type))
		.Write(reinterpret_cast<char_type const *>(std::addressof(extent)), sizeof(CodecTraits::extent_type));

		std::ranges::for_each(value, [&] (auto& p) { (void) operator<<(p.first).operator<<(p.second); });

		return dynamic_cast<std::basic_ostream<char_type>&>(*this);				
	}
/*	
	template <Detail::Primitive T> std::basic_istream<char_type>& operator>>(T& value) noexcept {
		CodecTraits::code_type code;
		return Read(reinterpret_cast<char_type *>(std::addressof(code)), sizeof(CodecTraits::code_type))
		.template Verify<T>(code)
		.Read(reinterpret_cast<char_type *>(std::addressof(value)), sizeof(T));
	}
*/	
	template <Detail::Primitive T> std::basic_istream<char_type>& operator>>(T& value) noexcept {
		CodecTraits::code_type 	code;

		Read(reinterpret_cast<char_type *>(std::addressof(code)), sizeof(CodecTraits::code_type))
		.template Verify<T>(code);

		if constexpr(! std::integral<T>) 
			return Read(reinterpret_cast<char_type *>(std::addressof(value)), sizeof(T));

		std::vector<unsigned char> v;
		while(~peek() && static_cast<unsigned char>(peek()) & 0x00FF) v.emplace_back(static_cast<unsigned char>(get()));

		if constexpr(! std::unsigned_integral<T>)
			(void) std::exchange(value, Compress::FromVarint<T>(v));
		else (void) 
			std::exchange(value, Compress::FromZigZag(Compress::FromVarint<T>(v)));

		return static_cast<std::basic_istream<char_type>& >(*this);
	}
	template <Detail::Structure T> std::basic_istream<char_type>& operator>>(T& value) noexcept {
		CodecTraits::code_type code;

		(void) Read(reinterpret_cast<char_type *>(std::addressof(code)), sizeof(CodecTraits::code_type)).Verify<T>(code);

		return std::apply([&]<typename... Pointers>(Field<Pointers> const& ...fs) {
			return std::ref((operator>>(value.*(fs.pointer)), ...));
		}, Information<T>::variables);			
	}
	template <Detail::Array T> std::basic_istream<char_type>& operator>>(T& value) noexcept {
		CodecTraits::code_type 		arraycode;
		CodecTraits::code_type 		valuecode;
		CodecTraits::extent_type 	extent;

		(void) Read(reinterpret_cast<char_type *>(std::addressof(arraycode)), sizeof(CodecTraits::code_type))
		.template Verify<T>(arraycode)
		.Read(reinterpret_cast<char_type *>(std::addressof(valuecode)), sizeof(CodecTraits::code_type))
		.template Verify<typename T::value_type>(valuecode)
		.Read(reinterpret_cast<char_type *>(std::addressof(extent)), sizeof(CodecTraits::extent_type));

		value.resize(extent, typename T::value_type {});

		std::ranges::for_each(value, [&] (auto& v) { (void) operator>>(v); });

		return dynamic_cast<std::basic_istream<char_type>&>(*this);		
	}
	template <Detail::Dictionary T> std::basic_istream<char_type>& operator>>(T& value) noexcept {
		CodecTraits::code_type 		dictcode;
		CodecTraits::code_type 		keycode;
		CodecTraits::code_type 		valuecode;
		CodecTraits::extent_type 	extent;	

		(void) Read(reinterpret_cast<char_type *>(std::addressof(dictcode)), sizeof(CodecTraits::code_type))
		.template Verify<T>(dictcode)
		.Read(reinterpret_cast<char_type *>(std::addressof(keycode)), sizeof(CodecTraits::code_type))
		.template Verify<typename T::key_type>(keycode)	
		.Read(reinterpret_cast<char_type *>(std::addressof(valuecode)), sizeof(CodecTraits::code_type))
		.template Verify<typename T::mapped_type>(valuecode)	
		.Read(reinterpret_cast<char_type *>(std::addressof(extent)), sizeof(CodecTraits::extent_type));

		T {}.swap(value);

		typename T::key_type 		first 	{};
		typename T::mapped_type 	second  {};

		for(std::decay_t<decltype(extent)> index {}; index < extent && operator>>(first).operator>>(second); index ++) 
			value.emplace(std::move(first), std::move(second));

		return dynamic_cast<std::basic_istream<char_type>&>(*this);			
	}
	std::basic_istream<char_type>& operator<<(auto value) noexcept {
		std::fprintf(stderr, std::format("{} not registered", Utility::TypeName<std::decay_t<decltype(value)>>()).data());
		std::quick_exit(EXIT_FAILURE);

		return dynamic_cast<std::basic_istream<char_type>&>(*this);		
	}
	std::basic_istream<char_type>& operator>>(auto& value) noexcept {
		std::fprintf(stderr, std::format("{} not registered", Utility::TypeName<std::decay_t<decltype(value)>>()).data());
		std::quick_exit(EXIT_FAILURE);

		return dynamic_cast<std::basic_istream<char_type>&>(*this);		
	}
	std::optional<CodecTraits::code_type> nextc() noexcept {
		CodecTraits::code_type 		code;
		std::streamsize 			size;
	
		Utility::ScopeGuard guard ( [&] { for(std::streamsize count {}; count < size; count ++) (void) unget(); } );
		size = readsome(reinterpret_cast<char_type *>(std::addressof(code)), sizeof(CodecTraits::code_type));

		return size == sizeof(CodecTraits::code_type) ? std::make_optional(code) : std::nullopt;
	}
	self_type fWrite(std::string_view filename) noexcept {
		std::fstream fs { filename.data(), std::ios_base::binary | std::ios_base::out | std::ios_base::trunc };
		Utility::ScopeGuard guard ( [&] { 
			if(fs.is_open()) 
				fs.close(); 
		} );
		if(! fs.is_open()) 
			return self_type::Arouse("can not open file");
	
		std::copy(	std::istreambuf_iterator<char_type>(seekg(0, std::ios_base::beg).rdbuf()),  
           			std::istreambuf_iterator<char_type>(),  
           			std::ostreambuf_iterator<char_type>(fs)); 
        return fs.fail() ? self_type::Arouse("Write file error"): self_type::Enwrap(*this);		
	}	
	self_type fRead(std::string_view filename) noexcept {
		std::fstream fs { filename.data(), std::ios_base::binary | std::ios_base::in };
		Utility::ScopeGuard guard( [&] { 
			if(fs.is_open()) 
				fs.close(); 
		} );

		if(!fs.is_open()) 
			return self_type::Arouse("can not open file");

		fs.seekg(0, std::ios_base::end);
		std::streamsize size { fs.tellg() };
		fs.seekg(0, std::ios_base::beg);

		std::vector<char> torrent(size, char {});
		if(!fs.read(reinterpret_cast<char *>(torrent.data()), torrent.size())) 
		/* ##basic_ostream& operator<<( std::basic_streambuf<CharT, Traits>* sb ); */
			return self_type::Arouse("Read file error");

		return !seekp(0, std::ios_base::beg).write(reinterpret_cast<char_type const *>(torrent.data()), torrent.size()) ?
			self_type::Arouse("Writeback buffer error"): self_type::Enwrap(*this);
	}
private: 
	template <typename T> constexpr SerdeStream& Verify(CodecTraits::code_type code) noexcept {
		if(code == Information<T>::code) 
			return *this;
		std::fprintf(stderr, std::format("require {}: {} |next {}: {}\n",
			Information<T>::name, 			Information<T>::code,
			CodecFactory::nameOf[code], 	code
		).data());
		std::quick_exit(EXIT_FAILURE);
	} 		
};
	
}
}

#endif
