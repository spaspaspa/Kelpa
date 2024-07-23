/**
 * contribute internet.hpp
 **/

#ifndef __CONET_INTERNET_INCLUDED
#define __CONET_INTERNET_INCLUDED

#include <cstdint>
#include <array>
#include <bit>
#include <limits>
#include <algorithm>
#include <sstream>
#include <utility>
#include <cstring>
#include <format>
#include <variant>
#include <ranges>

namespace conet __attribute__((__visibility__("default"))) {
namespace detail __attribute__((__visibility__("hidden"))) {
	
template <typename Alloc> using string_with = 
	std::enable_if_t<
		std::same_as<typename std::allocator_traits<Alloc>::value_type, char>, 
		std::basic_string<char, std::char_traits<char>, Alloc>>;
#if defined(__GNUC__) || defined(__clang__)
    #if defined(__x86_64__) || defined(__i386__)
        #define BSWAP_SUPPORTED
    #endif
#endif
#ifdef BSWAP_SUPPORTED
    #define bswap16 __builtin_bswap16
    #define bswap32 __builtin_bswap32
    #define bswap64 __builtin_bswap64
    #undef BSWAP_SUPPORTED
#else
    template<std::integral T>
    [[nodiscard]] inline constexpr T __bswap(T value) noexcept {
        uint8_t* p = reinterpret_cast<std::uint8_t*>(std::addressof(value));
        for (std::size_t i = 0, j = sizeof(T) - 1; i < j; ++i, --j) 
            std::swap(p[i], p[j]);
        return value;
    }

    template<typename T>
    [[nodiscard]] inline constexpr T __bswap16(T value) noexcept {
        uint16_t result = bswap(static_cast<std::uint16_t>(value));
        return static_cast<T>(result);
    }

    template<typename T>
    [[nodiscard]] inline constexpr T __bswap32(T value) noexcept {
        uint32_t result = bswap(static_cast<std::uint32_t>(value));
        return static_cast<T>(result);
    }

    template<typename T>
    [[nodiscard]] inline constexpr T __bswap64(T value) noexcept {
        uint64_t result = bswap(static_cast<std::uint64_t>(value));
        return static_cast<T>(result);
    }
    #define bswap16 conet::net::detail::__bswap16
    #define bswap32 conet::net::detail::__bswap32
    #define bswap64 conet::net::detail::__bswap64
#endif

	template<std::integral T>
	constexpr T byteswap(T value) noexcept {
		static_assert(std::has_unique_object_representations_v<T>, "T may not have padding bits");
		auto value_representation = std::bit_cast<std::array<std::byte, sizeof(T)>>(value);
		std::ranges::reverse(value_representation);
		return std::bit_cast<T>(value_representation);
	}
	
	[[nodiscard]] static int inet_pton4(const char *src, unsigned char *dst) noexcept {
		static constexpr std::size_t v4_size = 4;
		bool 			saw_digit = false;
		int 			octets = 0;
		int 			character = 0;
		unsigned char 	temporary[v4_size], *pointer;
		* (pointer = temporary) = 0;
		while ((character = *src ++)) {
			if (std::isdigit(character)) {
				unsigned int _new = (* pointer) * 10 + character - '0';
				if ((saw_digit and * pointer == 0) or _new > 255)
					return (0);
				*pointer = _new;
				if (not saw_digit) {
					if (++ octets > 4)
						return (0);
					saw_digit = true;
				}
			} else if (character == '.' && saw_digit) {
				if (octets == 4)
					return (0);
				* ++pointer = 0;
				saw_digit = false;
			} else
				return (0);
		}
		if (octets < 4)
			return (0);
		std::memcpy(dst, temporary, v4_size);
		return (1);
	}
	
	[[nodiscard]] static int inet_pton6(const char *src, unsigned char *dst) noexcept {
		static constexpr std::size_t 	v6_size = 16;
		static constexpr std::size_t 	v4_size = 4;
		static constexpr std::size_t 	uint_size = 2;
	
		unsigned char 		tempory[v6_size] = {0};
		unsigned char 		*pointer = nullptr;
		unsigned char 		*endp = nullptr;
		unsigned char 		*colonp = nullptr;
		const char  		*curtok = nullptr;
		int 				character = 0;
		int 				seen_xdigits = 0;
		unsigned int 		value = 0;
	
		std::memset((pointer = tempory), '\0', v6_size);
		endp = pointer + v6_size;
	
		if (* src == ':') if (* ++src != ':') return (0);
		curtok = src;
		while ((character = *src ++)) {
			if (std::isxdigit(character)) {
				(value <<= 4) |= (std::isdigit(character) ? character - '0' : std::tolower(character) - 'a' + 10);
				if (++ seen_xdigits > 4) return (0);
				continue;
			}
			if (character == ':') {
				curtok = src;
				if (not seen_xdigits) {
					if (colonp) return (0);
					colonp = pointer;
					continue;
				} else if (not *src) return (0);
				if (pointer + uint_size > endp) return (0);
				*pointer ++ = (unsigned char) (value >> 8) & 0xff;
				*pointer ++ = (unsigned char) value & 0xff;
				seen_xdigits = 0; value = 0;
				continue;
			}
			if (character == '.' and ((pointer + v4_size) <= endp) and inet_pton4(curtok, pointer) > 0) {
				pointer += v4_size;
				seen_xdigits = 0;
				break;  
			}
			return (0);
		}
		if (seen_xdigits) {
			if (pointer + uint_size > endp) return (0);
			*pointer ++ = (unsigned char) (value >> 8) & 0xff;
			*pointer ++ = (unsigned char) (value & 0xff);
		}
		if (colonp) {
			const int distance = pointer - colonp;
			if (pointer == endp) return (0);
			for (int i = 1; i <= distance; i ++) 
				endp[0 - i] = std::exchange(colonp[distance - i], 0);
			pointer = endp;
		}
		if (pointer != endp) return (0);
		std::memcpy(dst, tempory, v6_size);
		return (1);
	}
	[[nodiscard]] static char *inet_ntop4(unsigned char const *src, char *dst, std::size_t size) noexcept {
		static constexpr char 	format[] = "%u.%u.%u.%u";
		char 					temporary[sizeof "255.255.255.255"] = {0};
		int length = std::snprintf(temporary, sizeof(temporary), format, src[0], src[1], src[2], src[3]);
		if (length <= 0 or (std::size_t) length >= size)
			return (nullptr);
		std::strncpy(dst, temporary, size);
		return (dst);
	}
	
	[[nodiscard]] static char *inet_ntop6(unsigned char const *src, char *dst, std::size_t size) noexcept {
		char 	temporary[sizeof "ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255"] = {0};
		char* 	pointer = nullptr;
		struct {
			int 	base = -1;
			int 	length = 0;
		} best {}, 	current {};
		static constexpr std::size_t 	v6_size   = 16;
		static constexpr std::size_t 	short_size  = 2;
		unsigned int 					words[v6_size / short_size] = {0};
	
		std::memset(words, '\0', sizeof words);
		for (std::size_t i {}; i < v6_size; i ++)
			words[i / 2] |= (src[i] << ((1 - (i % 2)) << 3));
		
		for (std::size_t i {}; i < (v6_size / short_size); i++) {
			if (0 == words[i]) {
				if (-1 == current.base)
				{ current.base = i; current.length = 1; }
				else current.length ++;
			} else {
				if (current.base != -1) {
					if (-1 == best.base or current.length > best.length) best = current;
					current.base = -1;
				}
			}
		}
		if (current.base != -1 and (-1 == best.base or current.length > best.length))  best = current;
		if (best.base != -1 and best.length < 2) best.base = -1;
			
		pointer = temporary;
		for (std::size_t i {}; i < (v6_size / short_size); i++) {
			if (best.base != -1 and static_cast<int>(i) >= best.base and static_cast<int>(i) < (best.base + best.length)) {
				if (static_cast<int>(i) == best.base)
					*pointer ++ = ':';
				continue;
			}
			if (not (0 == i)) *pointer ++ = ':';
			if (6 == i and 0 == best.base and (6 == best.length or (7 == best.length and words[7] != 0x0001) or (5 == best.length and 0xffff == words[5]))	) {
				if (not inet_ntop4(src + 12, pointer, sizeof temporary - (pointer - temporary))) return (nullptr);
				pointer += std::strlen(pointer);
				break;
			}
			pointer += std::sprintf(pointer, "%x", words[i]);
		}
		if (best.base != -1 and (best.base + best.length) == (v6_size / short_size)) *pointer ++ = ':';
		*pointer++ = '\0';
	
		if ((std::size_t)(pointer - temporary) > size) return (nullptr);
		std::strcpy(dst, temporary);
		return (dst);
	}
} // namespace conet::detail	

namespace net __attribute__((__visibility__("default"))) {
namespace ip __attribute__((__visibility__("default"))) {
		
	typedef std::uint_least16_t port_type;
	typedef std::uint_least32_t scope_id_type;

	typedef enum class address_family: signed int {} address_family;
	typedef enum class socket_type: signed int {} socket_type;
	typedef enum class socket_protocol: signed int {} socket_protocol;
	inline constexpr address_family ipv4_address = static_cast<address_family>(0x0000);
	inline constexpr address_family ipv6_address = static_cast<address_family>(0x0001);
	inline constexpr socket_type stream_socket = static_cast<socket_type>(0x0000);
	inline constexpr socket_type datagram_socket = static_cast<socket_type>(0x0001);
	inline constexpr socket_protocol tcp_protocol = static_cast<socket_protocol>(0x0000);
	inline constexpr socket_protocol upd_protocol = static_cast<socket_protocol>(0x0001);
	inline constexpr std::size_t ipv4_address_strlen = 16;
	inline constexpr std::size_t ipv6_address_strlen = 46;
	
	[[nodiscard]] inline int inet_pton(address_family family, char const* src, void *dst) noexcept {
		if(family == ipv4_address) return detail::inet_pton4(src, reinterpret_cast<unsigned char *>(dst));
		if(family == ipv6_address) return detail::inet_pton6(src, reinterpret_cast<unsigned char *>(dst));
		return (-1);
	}
	[[nodiscard]] inline char *inet_ntop(address_family family, void const* src, char *dst, std::size_t size) noexcept {
		if(family == ipv4_address) return detail::inet_ntop4(reinterpret_cast<unsigned char const *>(src), dst, size);
		if(family == ipv6_address) return detail::inet_ntop6(reinterpret_cast<unsigned char const *>(src), dst, size);
		return (nullptr);
	}
	struct [[nodiscard("address_v4")]] address_v4 {
		typedef std::uint_least32_t uint_type;
		struct bytes_type: std::array<unsigned char, 4> {
			template <typename... Ts> explicit constexpr bytes_type(Ts... ts) noexcept: array<unsigned char, 4>{{static_cast<unsigned char>(ts) ...}} {
				static_assert(not (std::numeric_limits<unsigned char>::max() > 0x00ff));
			}	
		};
		constexpr address_v4() noexcept = default;
		constexpr address_v4(address_v4 const&) noexcept = default;
		constexpr address_v4& operator=(address_v4 const&) noexcept = default;
		constexpr address_v4(address_v4 &&) noexcept = default;
		constexpr address_v4& operator=(address_v4 &&) noexcept = default;
		
		constexpr address_v4(bytes_type const& b) noexcept: M_address(std::bit_cast<std::uint_least32_t>(b)) {}
		constexpr address_v4(std::uint_least32_t value): M_address(detail::byteswap<std::uint_least32_t>(value)) {}
		
		[[nodiscard]] constexpr bool is_unspecified() const noexcept { return (0 == M_address); }
		[[nodiscard]] constexpr bool is_loopback() const noexcept { return (to_uint() & 0xff000000) == 0X7f000000; }
		[[nodiscard]] constexpr bool is_multicast() const noexcept { return (to_uint() & 0xf0000000) == 0xe0000000; }
		
		[[nodiscard]] constexpr std::uint_least32_t to_uint() const noexcept { return detail::byteswap<std::uint_least32_t>(M_address); }
		[[nodiscard]] constexpr bytes_type to_bytes() const noexcept { return std::bit_cast<bytes_type>(M_address); }
		template <typename Alloc = std::allocator<char>>
		[[nodiscard]] constexpr detail::string_with<Alloc> to_string(Alloc const& alloc = Alloc()) const noexcept(std::is_default_constructible_v<Alloc>) {
			auto __address = to_uint();
			return {(std::ostringstream() << ((__address >> 24) & 0xFF) << "."  
			        << ((__address >> 16) & 0xFF) << "."  
			        << ((__address >> 8)  & 0xFF) << "."  
			        << (__address & 0xFF)).view().data(), 
				alloc	};
		}
		
		[[nodiscard]] static constexpr address_v4 any() noexcept { return address_v4{}; }
	    [[nodiscard]] static constexpr address_v4 loopback() noexcept { return address_v4{0x7F000001}; }
	    [[nodiscard]] static constexpr address_v4 broadcast() noexcept { return address_v4{0xFFFFFFFF}; }
	    
    	[[nodiscard]] friend constexpr std::weak_ordering operator<=>(address_v4 const& x, address_v4 const& y) noexcept {
			return std::compare_three_way{}(x.to_uint(), y.to_uint());
		}
	    
	private:
		template <typename Protocol> friend struct basic_endpoint;
		friend address_v4 make_address_v4(char const*, std::error_code&) noexcept;

		[[nodiscard]] static constexpr std::uint16_t hton_16(std::uint16_t __host) noexcept { 
			if constexpr (std::endian::native == std::endian::big) 
				return __host; 
			return bswap16(__host); 
		}
		[[nodiscard]] static constexpr std::uint16_t ntoh_16(std::uint16_t __net) { 
			if constexpr (std::endian::native == std::endian::big) 
				return __net;
			return bswap16(__net); 
		}
		[[nodiscard]] static constexpr std::uint32_t hton_32(std::uint32_t __host) noexcept { 
			if constexpr (std::endian::native == std::endian::big) 
				return __host;
			return bswap32(__host); 
		}
		[[nodiscard]] static constexpr std::uint32_t ntoh_32(std::uint32_t __net) noexcept { 
			if constexpr (std::endian::native == std::endian::big) 
				return __net;
			return bswap32(__net); 
		}		
		std::uint32_t M_address = 0;
	};
	
	[[nodiscard]] constexpr bool operator==(address_v4 const& __a, address_v4 const& __b) noexcept
	{ 	return __a.to_uint() == __b.to_uint(); }
	
	[[nodiscard]] constexpr bool operator!=(address_v4 const& __a, address_v4 const& __b) noexcept
	{ 	return not (__a == __b); }
	
	[[nodiscard]] constexpr bool operator< (address_v4 const& __a, address_v4 const& __b) noexcept
	{ 	return __a.to_uint() < __b.to_uint(); }
	
	[[nodiscard]] constexpr bool operator> (address_v4 const& __a, address_v4 const& __b) noexcept
	{ 	return __b < __a; }
	
	[[nodiscard]] constexpr bool operator<=(address_v4 const& __a, address_v4 const& __b) noexcept
	{ 	return not (__b < __a); }
	
	[[nodiscard]] constexpr bool operator>=(address_v4 const& __a, address_v4 const& __b) noexcept
	{ 	return not (__a < __b); }	
	
	[[nodiscard]] constexpr address_v4 make_address_v4(address_v4::bytes_type const& bytes) noexcept { return address_v4{bytes}; }
	[[nodiscard]] constexpr address_v4 make_address_v4(address_v4::uint_type value) noexcept { return address_v4{value}; }
	[[nodiscard]] inline address_v4 make_address_v4(char const* s, std::error_code& ec) noexcept {
		address_v4 address;
		const int result = inet_pton(ipv4_address, s, reinterpret_cast<void *>(std::addressof(address.M_address)));
		if(result > 0) {
			ec.clear();
			return address;
		}
		if(result == 0) 
			ec = std::make_error_code(std::errc::invalid_argument);
		else 
			ec.assign(errno, std::generic_category());
		return {};
	}
	[[nodiscard]] inline address_v4 make_address_v4(std::string const& s, std::error_code& e) noexcept {
		return make_address_v4(s.c_str(), e);
	}
	[[nodiscard]] inline address_v4 make_address_v4(std::string_view s, std::error_code& e) noexcept {
		char buffer[16] = {0};
		auto length = s.copy(buffer, std::size(buffer));
		if(length == std::size(buffer)) {
			e = std::make_error_code(std::errc::invalid_argument);
			return {};
		}
		e.clear();
		buffer[length] = '\0';
		return make_address_v4(buffer, e);
	}
	template<typename CharT, typename Traits> 
	inline std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& __os, address_v4 const& __a) noexcept
    { 	return (__os << __a.to_string()); }
 
	struct [[nodiscard("address_v6")]] address_v6 {
		struct bytes_type: std::array<unsigned char, 16> {
			template <typename... Ts> explicit constexpr bytes_type(Ts... ts): std::array<unsigned char, 16>{{static_cast<unsigned char>(ts) ...}} {
				static_assert(not (std::numeric_limits<unsigned char>::max() > 0x00ff));
			}
		};
		constexpr address_v6() noexcept = default;
		constexpr address_v6(address_v6 const&) noexcept = default;
		constexpr address_v6& operator=(address_v6 const&) noexcept = default;
		constexpr address_v6(address_v6 &&) noexcept = default;
		constexpr address_v6& operator=(address_v6 &&) noexcept = default;
		
		constexpr address_v6(bytes_type const& __bytes, scope_id_type __scope_id = 0) noexcept
		: M_bytes(__bytes), M_scope_id(__scope_id) {}
		
		void scope_id(scope_id_type __scope_id) noexcept { M_scope_id = __scope_id; }
		[[nodiscard]] constexpr scope_id_type scope_id() const noexcept { return M_scope_id; }
		[[nodiscard]] constexpr bool is_unspecified() const noexcept {
			if (not std::ranges::all_of(M_bytes, [](auto&& value) { return (0 == value); }))
				return false;
			return (0x0000 == M_scope_id);
		}
		[[nodiscard]] constexpr bool is_loopback() const noexcept {
			if(not std::all_of(M_bytes.cbegin(), std::prev(M_bytes.cend()), [](auto&& value) { return (0 == value); }))
				return false;
			return (0x0001 == M_bytes.back()) and (0x0000 == M_scope_id);	
		}
		[[nodiscard]] constexpr bool is_multicast() const noexcept { return (0x00ff == M_bytes.front()); }
		[[nodiscard]] constexpr bool is_link_local() const noexcept { return (0x00fe == M_bytes.front()) and (0x80 == (0xc0 & M_bytes[1])); }
		[[nodiscard]] constexpr bool is_site_local() const noexcept { return (0x00fe == M_bytes.front()) and (0xc0 == (0xc0 & M_bytes[1])); }
		[[nodiscard]] constexpr bool is_unique_local() const noexcept { return (0xfc == (M_bytes.front() & 0xfe)) ; }
		[[nodiscard]] constexpr bool is_v4_mapped() const noexcept {
			return (std::all_of(M_bytes.cbegin(), M_bytes.cbegin() + 9, [](auto&& value) { return (0x0000 == value); }) and (0x00ff == M_bytes[10]) && (0x00ff == M_bytes[11])); 
		}
		[[nodiscard]] constexpr bool is_multicast_node_local() const noexcept { return (is_multicast() and (0x01 == (M_bytes[1] & 0x0f))); }	
		[[nodiscard]] constexpr bool is_multicast_link_local() const noexcept { return (is_multicast() and (0x02 == (M_bytes[1] & 0x0f))); }
		[[nodiscard]] constexpr bool is_multicast_site_local() const noexcept { return (is_multicast() and (0x05 == (M_bytes[1] & 0x0f))); }
		[[nodiscard]] constexpr bool is_multicast_org_local() const noexcept { return (is_multicast() and (0x08  == (M_bytes[1] & 0x0f))); }
		[[nodiscard]] constexpr bool is_multicast_unique_local() const noexcept { return (is_multicast() and (0x0e  == (M_bytes[1] & 0x0f))); }
		[[nodiscard]] constexpr bool is_multicast_global() const noexcept { return (is_multicast() and (0x0b  == (M_bytes[1] & 0x0f))); }

		constexpr bytes_type to_bytes() const noexcept { return M_bytes; }
		template <typename Alloc = std::allocator<char>>
		[[nodiscard]] constexpr detail::string_with<Alloc> to_string(Alloc const& alloc = Alloc()) const noexcept(std::is_default_constructible_v<Alloc>) {
			detail::string_with<Alloc> s(alloc);
			s.resize(ipv6_address_strlen + (not (0 == M_scope_id) ? 11 : 0));
			char *const pointer = std::addressof(s.front());
			if(inet_ntop(ipv6_address, std::addressof(M_bytes), pointer, s.size())) {
				auto sentinel = s.find('\0');
				if(not (0 == M_scope_id)) 
					s.erase(sentinel + std::sprintf(pointer + sentinel, "%%%lu", (long unsigned int)M_scope_id));
				return s;	
			} else return {};
		}
		
		[[nodiscard]] static constexpr address_v6 any() noexcept { return {}; }
		[[nodiscard]] static constexpr address_v6 loopback() noexcept { return {bytes_type{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}}; }
		
    	[[nodiscard]] friend constexpr std::weak_ordering operator<=>(address_v6 const& x, address_v6 const& y) noexcept {
			if(auto result = std::compare_three_way{}(x.M_bytes, y.M_bytes); result != std::weak_ordering::equivalent) 
				return result;
			return std::compare_three_way{}(x.M_scope_id, y.M_scope_id);	
		}
	private:
		template <typename Protocol> friend struct basic_endpoint;
		template <typename> friend struct basic_address_iterator;
	    friend constexpr bool operator==(address_v6 const&, address_v6 const&) noexcept;
	    friend constexpr bool operator< (address_v6 const&, address_v6 const&) noexcept;
		template<typename> friend struct std::hash;
		
		bytes_type 		M_bytes 	{};
		scope_id_type 	M_scope_id 	{};
	};    

	[[nodiscard]] constexpr bool operator==(address_v6 const& x, address_v6 const& y) noexcept {
		return ((x.M_bytes <=> y.M_bytes) == std::weak_ordering::equivalent) and (x.M_scope_id == y.M_scope_id);
	}
	[[nodiscard]] constexpr bool operator!=(address_v6 const& __a, address_v6 const& __b) noexcept
	{ return not (__a == __b); }
	[[nodiscard]] constexpr bool operator<(address_v6 const& x, address_v6 const& y) noexcept {
		if((x.M_bytes <=> y.M_bytes) != std::weak_ordering::equivalent)
			return (x.M_bytes <=> y.M_bytes) == std::weak_ordering::less;					
		return x.M_scope_id < y.M_scope_id;	
	}
  	[[nodiscard]] constexpr bool operator> (address_v6 const& __a, address_v6 const& __b) noexcept
    { 	return __b < __a; }
  	[[nodiscard]] constexpr bool operator<=(address_v6 const& __a, address_v6 const& __b) noexcept
  	{ 	return not (__b < __a); }
  	[[nodiscard]] constexpr bool operator>=(address_v6 const& __a, address_v6 const& __b) noexcept
  	{ 	return not (__a < __b); }

	[[nodiscard]] constexpr address_v6 make_address_v6(address_v6::bytes_type const& bytes, scope_id_type scope_id) noexcept 
	{ 	return address_v6{bytes, scope_id}; }
	[[nodiscard]] inline address_v6 make_address_v6(char const* address, char const *scope, std::error_code& ec) noexcept {
		address_v6::bytes_type bytes;
		const int result = inet_pton(ipv6_address, address, reinterpret_cast<void *>(std::addressof(bytes)));
		if(result > 0) {
			ec.clear();
			if(nullptr == scope) 
				return { bytes };
			char *sentinel = nullptr;	
			unsigned long value = std::strtoul(scope, std::addressof(sentinel), 10);	
			if(sentinel != scope and ! *sentinel and value < std::numeric_limits<scope_id_type>::max())
				return { bytes, static_cast<scope_id_type>(value) };
			ec = std::make_error_code(std::errc::invalid_argument);	
		} else if(0 == result) ec = std::make_error_code(std::errc::invalid_argument);
		else ec.assign(errno, std::generic_category());	
		return {};
	}
	[[nodiscard]] inline address_v6 make_address_v6(char const* s, std::error_code& ec) noexcept {
		auto 	scope_position = std::strchr(s, '%');
		if(nullptr == scope_position) return make_address_v6(s, nullptr, ec);
		char 	buffer[64] = {0};
		char 	*output = buffer;
		bool 	skip_leading_zero = true;
		while(s < scope_position && output < std::end(buffer)) {
			if(not skip_leading_zero or '0' != *s) {
				if(':' == * s or '.' == * s)  skip_leading_zero = true;
				else { skip_leading_zero = false; * output ++ = * s; }
				s ++;
			}
		}
		if(output == std::end(buffer)) {
			ec = std::make_error_code(std::errc::invalid_argument);
			return {};
		} else {
			* output = '\0';
			return make_address_v6(buffer, scope_position, ec);
		}	
	}
	[[nodiscard]] inline address_v6 make_address_v6(std::string const& s, std::error_code& ec) noexcept {
		return make_address_v6(s.data(), ec);
	}
	[[nodiscard]] inline address_v6 make_address_v6(std::string_view s, std::error_code& ec) noexcept {
		return make_address_v6(s.data(), ec);
	}	
	
	template <typename CharT, typename Traits> 
	[[nodiscard]] inline std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& __os, address_v6 const& __a) noexcept 
	{	return (__os << __a.to_string());		}
	
	struct bad_address_cast : public std::bad_cast {
		bad_address_cast() noexcept = default;
		constexpr const char* what() const noexcept { return "bad address cast"; }
	};
	
	struct v4_mapped_t {};
	inline constexpr v4_mapped_t v4_mapped;
	
  	[[nodiscard]] constexpr address_v4 make_address_v4(v4_mapped_t, address_v6 const& __a) {
    	if (not __a.is_v4_mapped()) throw bad_address_cast();
    	const auto __v6b = __a.to_bytes();
    	return address_v4::bytes_type(__v6b[12], __v6b[13], __v6b[14], __v6b[15]);
  	}
	[[nodiscard]] constexpr address_v6 make_address_v6(v4_mapped_t, const address_v4& __a) noexcept {
		const address_v4::bytes_type __v4b = __a.to_bytes();
		address_v6::bytes_type __v6b(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, __v4b[0], __v4b[1], __v4b[2], __v4b[3]);
		return address_v6(__v6b);
	}	
	
	struct [[nodiscard("address")]] address {
		constexpr address() noexcept = default;
		constexpr address(address const&) noexcept = default;
		constexpr address& operator=(address const&) = default;
		constexpr address(address &&) noexcept = default;
		constexpr address& operator=(address &&) noexcept = default;
		constexpr address(address_v4 const& __v4) noexcept: M_address(__v4) {}
		constexpr address(address_v6 const& __v6) noexcept: M_address(__v6) {}
		constexpr address& operator=(address_v4 const& __v4) noexcept 
		{	return (* this = address{__v4});	}
		constexpr address& operator=(address_v6 const& __v6) noexcept 
		{	return (* this = address{__v6});	}
		
	    [[nodiscard]] constexpr bool is_v4() const noexcept { return std::holds_alternative<address_v4>(M_address); }
	    [[nodiscard]] constexpr bool is_v6() const noexcept { return std::holds_alternative<address_v6>(M_address); }
	    [[nodiscard]] constexpr address_v4 to_v4() const {
			if(not is_v4()) throw bad_address_cast();
			return std::get<address_v4>(M_address);
		}
	    [[nodiscard]] constexpr address_v6 to_v6() const {
			if(not is_v6()) throw bad_address_cast();
			return std::get<address_v6>(M_address);
		}
	    [[nodiscard]] constexpr bool is_unspecified() const noexcept
	    { 	return is_v4() ? std::get<address_v4>(M_address).is_unspecified() : std::get<address_v6>(M_address).is_unspecified(); }
	    [[nodiscard]] constexpr bool is_loopback() const noexcept
	    { 	return is_v4() ? std::get<address_v4>(M_address).is_loopback() : std::get<address_v6>(M_address).is_loopback(); }
	    [[nodiscard]] constexpr bool is_multicast() const noexcept
	    { 	return is_v4() ? std::get<address_v4>(M_address).is_multicast() : std::get<address_v6>(M_address).is_multicast(); }
		template<typename Alloc = std::allocator<char>>
		[[nodiscard]] constexpr detail::string_with<Alloc>
		to_string(const Alloc& __a = Alloc()) const noexcept {
			return is_v4() ? 
				std::get<address_v4>(M_address).to_string(__a) : 
				std::get<address_v4>(M_address).to_string(__a);
		}	
		
    	[[nodiscard]] friend constexpr std::weak_ordering operator<=>(address const& __a, address const& __b) noexcept {
			if (__a.is_v4())
			  	return __b.is_v4() ? std::compare_three_way{}(std::get<address_v4>(__a.M_address), std::get<address_v4>(__a.M_address)) : std::weak_ordering::less;
			return __b.is_v4() ? std::weak_ordering::greater : std::compare_three_way{}(std::get<address_v4>(__a.M_address), std::get<address_v4>(__a.M_address));			
		}
		
	private:
		template <typename Protocol> friend struct basic_endpoint;
		template <typename> friend struct std::hash;
		friend constexpr bool operator==(address const&, address const&) noexcept;
		friend constexpr bool operator< (address const&, address const&) noexcept;
		std::variant<address_v4, address_v6> M_address = {address_v4{}};
	};
	[[nodiscard]] constexpr bool operator==(address const& __a, address const& __b) noexcept {
		if (__a.is_v4())
		  	return __b.is_v4() ? std::get<address_v4>(__a.M_address) == std::get<address_v4>(__a.M_address) : false;
		return __b.is_v4() ? false : std::get<address_v6>(__a.M_address) == std::get<address_v6>(__a.M_address);
	}	
	[[nodiscard]] constexpr bool operator!=(address const& __a, address const& __b) noexcept
	{ 	return not (__a == __b); }	
	[[nodiscard]] constexpr bool operator< (address const& __a, address const& __b) noexcept {
		if (__a.is_v4())
		  	return __b.is_v4() ? std::get<address_v4>(__a.M_address) < std::get<address_v4>(__a.M_address) : true;
		return __b.is_v4() ? false : std::get<address_v6>(__a.M_address) < std::get<address_v6>(__a.M_address);
	}	
	[[nodiscard]] constexpr bool operator> (address const& __a, address const& __b) noexcept
	{ 	return __b < __a; }
	
	[[nodiscard]] constexpr bool operator<=(address const& __a, address const& __b) noexcept
	{ 	return not (__b < __a); }
	
	[[nodiscard]] constexpr bool operator>=(address const& __a, address const& __b) noexcept
	{ 	return not (__a < __b); }		
	
	[[nodiscard]] inline address make_address(const char* __str, std::error_code& __ec) noexcept {
		if (address_v4 __v4a = make_address_v4(__str, __ec); not __ec) return {__v4a};
		if (address_v6 __v6a = make_address_v6(__str, __ec); not __ec) return {__v6a};
		return {};
	}

	[[nodiscard]] inline address make_address(const std::string& __str, std::error_code& __ec) noexcept
	{ 	return make_address(__str.c_str(), __ec); }
	
	[[nodiscard]] inline address make_address(std::string_view __str, std::error_code& __ec) noexcept {
		if (__str.rfind('\0') != std::string_view::npos)
		  	return make_address(__str.data(), __ec);
		return make_address(std::string{__str}, __ec); 
	}
	
	template<typename CharT, typename Traits> 
	inline std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& __os, address const& __a) noexcept
    { 	return (__os << __a.to_string()); }	

 	template <typename> struct basic_address_iterator { static_assert(std::bool_constant<false>::value, "not defined"); };
 	
 	template <> struct basic_address_iterator<address_v4> {
		typedef address_v4 				value_type;
		typedef std::ptrdiff_t 			difference_type;
		typedef address_v4* 			pointer;
		typedef address_v4 const* 		const_pointer;
		typedef address_v4& 			reference;
		typedef address_v4 const& 		const_reference;
		typedef std::input_iterator_tag iterator_category;
		
		constexpr basic_address_iterator() noexcept = default;
		constexpr basic_address_iterator(basic_address_iterator const&) noexcept = default;
		constexpr basic_address_iterator& operator=(basic_address_iterator const&) noexcept = default;
		constexpr basic_address_iterator(basic_address_iterator &&) noexcept = default;
		constexpr basic_address_iterator& operator=(basic_address_iterator &&) noexcept = default;
		constexpr basic_address_iterator(address_v4 const& __a) noexcept: M_address(__a) {}
		
	    constexpr reference operator*() noexcept { return M_address; }
	    constexpr pointer operator->() noexcept { return std::addressof(M_address); }
	    constexpr const_reference operator*() const noexcept { return M_address; }
	    constexpr const_pointer operator->() const noexcept { return std::addressof(M_address); }
		
		constexpr basic_address_iterator& operator++() noexcept {
		  	M_address = value_type(M_address.to_uint() + 1);
		  	return *this;
		}
		constexpr basic_address_iterator operator++(int) noexcept {
		  	auto __tmp = *this;
		  	(void) operator++();
		  	return __tmp;
		}
		constexpr basic_address_iterator& operator--() noexcept {
			M_address = value_type(M_address.to_uint() - 1);
			return *this;
		}
		constexpr basic_address_iterator operator--(int) noexcept {
		  	auto __tmp = *this;
		  	(void) operator--();
		  	return __tmp;
		}		
		constexpr bool operator==(const basic_address_iterator& __rhs) const noexcept
		{ 	return M_address == __rhs.M_address; 	}
		constexpr bool operator!=(const basic_address_iterator& __rhs) const noexcept
		{ 	return M_address != __rhs.M_address; 	}
		
	private:
		address_v4 M_address;
	};
 	template <> struct basic_address_iterator<address_v6> {
		typedef address_v6 				value_type;
		typedef std::ptrdiff_t 			difference_type;
		typedef address_v6* 			pointer;
		typedef address_v6 const* 		const_pointer;
		typedef address_v6& 			reference;
		typedef address_v6 const& 		const_reference;
		typedef std::input_iterator_tag iterator_category;
		
		constexpr basic_address_iterator() noexcept = default;
		constexpr basic_address_iterator(basic_address_iterator const&) noexcept = default;
		constexpr basic_address_iterator& operator=(basic_address_iterator const&) noexcept = default;
		constexpr basic_address_iterator(basic_address_iterator &&) noexcept = default;
		constexpr basic_address_iterator& operator=(basic_address_iterator &&) noexcept = default;
		constexpr basic_address_iterator(address_v6 const& __a) noexcept: M_address(__a) {}
		
	    constexpr reference operator*() noexcept { return M_address; }
	    constexpr pointer operator->() noexcept { return std::addressof(M_address); }
	    constexpr const_reference operator*() const noexcept { return M_address; }
	    constexpr const_pointer operator->() const noexcept { return std::addressof(M_address); }
		
		constexpr basic_address_iterator& operator++() noexcept {
			for(auto& reference: M_address.M_bytes | std::views::reverse) {
				if(reference < std::numeric_limits<unsigned char>::max()) 
				{ reference ++; break; }
				reference = std::numeric_limits<unsigned char>::min();
			}
		  	return *this;
		}
		constexpr basic_address_iterator operator++(int) noexcept {
		  	auto __tmp = *this;
		  	(void) operator++();
		  	return __tmp;
		}
		constexpr basic_address_iterator& operator--() noexcept {
			for(auto& reference: M_address.M_bytes | std::views::reverse) {
				if(reference > std::numeric_limits<unsigned char>::min()) 
				{ reference --; break; }
				reference = std::numeric_limits<unsigned char>::max();
			}			
			return *this;
		}
		constexpr basic_address_iterator operator--(int) noexcept {
		  	auto __tmp = *this;
		  	(void) operator--();
		  	return __tmp;
		}		
		constexpr bool operator==(const basic_address_iterator& __rhs) const noexcept
		{ 	return M_address == __rhs.M_address; 	}
		constexpr bool operator!=(const basic_address_iterator& __rhs) const noexcept
		{ 	return M_address != __rhs.M_address; 	}
		
	private:
		address_v6 M_address;
	};
	
	typedef basic_address_iterator<address_v4> address_v4_iterator;
	typedef basic_address_iterator<address_v6> address_v6_iterator;
	
	template <typename> struct basic_address_range { static_assert(std::bool_constant<false>::value, "not defined"); };
	
	template <> struct basic_address_range<address_v4> : public std::ranges::view_interface<basic_address_range<address_v4>> {
		typedef basic_address_iterator<address_v4> 						iterator;
		typedef basic_address_iterator<address_v4>::value_type 			value_type;  			
		typedef basic_address_iterator<address_v4>::difference_type 	difference_type;
		typedef basic_address_iterator<address_v4>::pointer 			pointer;
		typedef basic_address_iterator<address_v4>::const_pointer 		const_pointer;
		typedef basic_address_iterator<address_v4>::reference 			reference;
		typedef basic_address_iterator<address_v4>::const_reference 	const_reference;
	
		constexpr basic_address_range() noexcept = default;
		constexpr basic_address_range(basic_address_range const&) noexcept = default;
		constexpr basic_address_range& operator=(basic_address_range const&) noexcept = default;
		constexpr basic_address_range(basic_address_range &&) noexcept = default;
		constexpr basic_address_range& operator=(basic_address_range &&) noexcept = default;
		constexpr basic_address_range(address_v4 const& first, address_v4 const& last) noexcept: M_begin(first), M_end(last) {}
		constexpr basic_address_iterator<address_v4> begin() const noexcept { return M_begin; }
		constexpr basic_address_iterator<address_v4> end() const noexcept { return M_end; }
				
	private:
		basic_address_iterator<address_v4> 	M_begin = {};
		basic_address_iterator<address_v4> 	M_end = {};
	}; 
	template <> struct basic_address_range<address_v6> : public std::ranges::view_interface<basic_address_range<address_v6>> {
		typedef basic_address_iterator<address_v6> 						iterator;
		typedef basic_address_iterator<address_v6>::value_type 			value_type;  			
		typedef basic_address_iterator<address_v6>::difference_type 	difference_type;
		typedef basic_address_iterator<address_v6>::pointer 			pointer;
		typedef basic_address_iterator<address_v6>::const_pointer 		const_pointer;
		typedef basic_address_iterator<address_v6>::reference 			reference;
		typedef basic_address_iterator<address_v6>::const_reference 	const_reference;
	
		constexpr basic_address_range() noexcept = default;
		constexpr basic_address_range(basic_address_range const&) noexcept = default;
		constexpr basic_address_range& operator=(basic_address_range const&) noexcept = default;
		constexpr basic_address_range(basic_address_range &&) noexcept = default;
		constexpr basic_address_range& operator=(basic_address_range &&) noexcept = default;
		constexpr basic_address_range(address_v6 const& first, address_v6 const& last) noexcept: M_begin(first), M_end(last) {}
		constexpr basic_address_iterator<address_v6> begin() const noexcept { return M_begin; }
		constexpr basic_address_iterator<address_v6> end() const noexcept { return M_end; }
				
	private:
		basic_address_iterator<address_v6> 	M_begin = {};
		basic_address_iterator<address_v6> 	M_end = {};
	}; 	
	
	typedef basic_address_range<address_v4> address_v4_range;
	typedef basic_address_range<address_v6> address_v6_range;
	
	/* 	todos:
	struct network_v4
	struct network_v6
	*/
	
	template <typename Protocol> struct [[nodiscard("basic_endpoint")]] basic_endpoint {
		typedef Protocol protocal_type;
		constexpr basic_endpoint() noexcept = default;
		constexpr basic_endpoint(basic_endpoint const&) noexcept = default;
		constexpr basic_endpoint& operator=(basic_endpoint const&) noexcept = default;
		constexpr basic_endpoint(basic_endpoint &&) noexcept = default;
		constexpr basic_endpoint& operator=(basic_endpoint &&) noexcept = default;
		constexpr basic_endpoint(address const& __address, port_type __port = port_type()) noexcept: M_address(__address), M_port(__port) {}
		template <typename OProtocol> constexpr basic_endpoint(OProtocol const& __proto, port_type __port = port_type()) noexcept: M_port(address_v4::hton_16(__port)) {
			if(__proto.family() == ipv6_address) 
				M_address = address_v4();
		} 		
		
		[[nodiscard]] constexpr protocal_type protocol() const noexcept { return M_address.is_v4() ? protocal_type::v4() : protocal_type::v6() ;}
		[[nodiscard]] constexpr conet::net::ip::address address() const noexcept { return M_address; }
		[[nodiscard]] constexpr port_type port() const noexcept { return M_port; }
		constexpr void address(conet::net::ip::address const& __address) noexcept { M_address = __address; }
		constexpr void port(port_type __port) noexcept { M_port = __port; }
		
		[[nodiscard]] friend constexpr std::weak_ordering operator<=>(basic_endpoint const& x, basic_endpoint const& y) noexcept {
			if(x.M_address != y.M_address) return std::compare_three_way{} (x.M_address, y.M_address);
			return std::compare_three_way{} (x.M_port, y.M_port);
		}
		
	private:
		port_type 				M_port = {};
		conet::net::ip::address M_address = {};
	};
	template <typename OProtocol> basic_endpoint(OProtocol const&, port_type) -> basic_endpoint<OProtocol>;
		
	template<typename Protocol>
	[[nodiscard]] constexpr bool operator==(basic_endpoint<Protocol> const& __a, basic_endpoint<Protocol> const& __b) noexcept 
	{ 	return __a.address() == __b.address() and __a.port() == __b.port(); }
	
	template<typename Protocol>
	[[nodiscard]] constexpr bool operator!=(basic_endpoint<Protocol> const& __a, basic_endpoint<Protocol> const& __b) noexcept 
	{ 	return not (__a == __b); }
	
	template<typename Protocol>
	[[nodiscard]] constexpr bool operator< (basic_endpoint<Protocol> const& __a, basic_endpoint<Protocol> const& __b) noexcept {
	  	return __a.address() < __b.address()
		or (not (__b.address() < __a.address()) and __a.port() < __b.port());
	}
	
	template<typename Protocol>
	[[nodiscard]] constexpr bool operator> (basic_endpoint<Protocol> const& __a, basic_endpoint<Protocol> const& __b) noexcept 
	{ 	return __b < __a;  }
	
	template<typename Protocol>
	[[nodiscard]] constexpr bool operator<=(basic_endpoint<Protocol> const& __a, basic_endpoint<Protocol> const& __b) noexcept 
	{ 	return not (__b < __a); }
	
	template<typename Protocol>
	[[nodiscard]] constexpr bool operator>=(basic_endpoint<Protocol> const& __a, basic_endpoint<Protocol> const& __b) noexcept 
	{ 	return not (__a < __b); }
	
	template<typename Protocol, typename CharT, typename Traits> 
	inline std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& __os, basic_endpoint<Protocol> const& __p) noexcept	{
		if(__p.protocol().family() == ipv6_address) 
			return (__os << "[" << __p.address() << "]");
		return (__os << __p.address() << ":" << __p.port());	
	}
	
	struct [[nodiscard("tcp")]] tcp {
		typedef basic_endpoint<tcp> endpoint;
		/*
		typedef basic_socket_iostream<tcp> iostream;
		typedef basic_socket_acceptor<tcp> acceptor;
		typedef basic_stream_socket<tcp> socket;
		*/
		constexpr tcp() = delete;
		[[nodiscard]] static constexpr tcp v4() noexcept { return tcp(ipv4_address); }
		[[nodiscard]] static constexpr tcp v6() noexcept { return tcp(ipv6_address); }
		[[nodiscard]] constexpr address_family family() const noexcept { return M_family; }
		[[nodiscard]] constexpr socket_type type() const noexcept { return stream_socket; }
		[[nodiscard]] constexpr socket_protocol protocol() const noexcept { return tcp_protocol; }
		
	private:
		constexpr explicit tcp(address_family __family) noexcept: M_family(__family) {}
		address_family M_family = ipv4_address;
	};

	[[nodiscard]] constexpr bool operator==(const tcp& __a, const tcp& __b) noexcept
	{ 	return __a.family() == __b.family(); }
	
	[[nodiscard]] constexpr bool operator!=(const tcp& __a, const tcp& __b) noexcept
	{ 	return !(__a == __b); }
	
	struct [[nodiscard("upd")]] upd {
		typedef basic_endpoint<upd> endpoint;
		/*
		typedef basic_socket_iostream<upd> iostream;
		typedef basic_socket_acceptor<upd> acceptor;
		typedef basic_datagram_socket<upd> socket;
		*/
		constexpr upd() = delete;
		[[nodiscard]] static constexpr upd v4() noexcept { return upd(ipv4_address); }
		[[nodiscard]] static constexpr upd v6() noexcept { return upd(ipv6_address); }
		[[nodiscard]] constexpr address_family family() const noexcept { return M_family; }
		[[nodiscard]] constexpr socket_type type() const noexcept { return datagram_socket; }
		[[nodiscard]] constexpr socket_protocol protocol() const noexcept { return upd_protocol; }
		
	private:
		constexpr explicit upd(address_family __family) noexcept: M_family(__family) {}
		address_family M_family = ipv4_address;
	};

	[[nodiscard]] constexpr bool operator==(const upd& __a, const upd& __b) noexcept
	{ 	return __a.family() == __b.family(); }
	
	[[nodiscard]] constexpr bool operator!=(const upd& __a, const upd& __b) noexcept
	{ 	return !(__a == __b); }	
	
} // namespace conet::net::ip
} // namespace conet::net
} // namespace conet

namespace std {
	
  	template<> struct hash<conet::net::ip::address_v4> {
      	std::size_t operator()(conet::net::ip::address_v4 const& address) const noexcept
      	{ return std::hash<std::uint_least32_t>{}(address.to_uint()); }
    };	
	template<>  struct formatter<conet::net::ip::address_v4> {  
	    constexpr auto parse(std::format_parse_context& context) const noexcept 
		{  	return context.begin(); 								}  
	
		auto format(conet::net::ip::address_v4 const& t, std::format_context& context) const noexcept
		{  	return std::format_to(context.out(), "{}", t.to_string());  	}  
	};
	
  	template<> struct hash<conet::net::ip::address_v6> {
      	std::size_t operator()(conet::net::ip::address_v6 const& address) const noexcept {
	  		std::size_t result = {};
			std::ranges::for_each(address.M_bytes, [&result](auto&& value) { result |= std::hash<unsigned char>{}(value); });
			result |= std::hash<conet::net::ip::scope_id_type>{}(address.M_scope_id);
			return result;     
		}
    };	
	template<>  struct formatter<conet::net::ip::address_v6> {  
	    constexpr auto parse(std::format_parse_context& context) const noexcept 
		{  	return context.begin(); 								}  
	
		auto format(conet::net::ip::address_v6 const& t, std::format_context& context) const noexcept
		{  	return std::format_to(context.out(), "{}", t.to_string());  	}  
	};	
  
  	template<> struct hash<conet::net::ip::address> {
      	std::size_t operator()(conet::net::ip::address const& __address) const noexcept {
   			if(__address.is_v4()) return std::hash<conet::net::ip::address_v4>{}(std::get<conet::net::ip::address_v4>(__address.M_address));
   			return std::hash<conet::net::ip::address_v6>{}(std::get<conet::net::ip::address_v6>(__address.M_address));
		}
    };	
	template<>  struct formatter<conet::net::ip::address> {  
	    constexpr auto parse(std::format_parse_context& context) const noexcept 
		{  	return context.begin(); 								}  
	
		auto format(conet::net::ip::address const& t, std::format_context& context) const noexcept
		{  	return std::format_to(context.out(), "{}", t.to_string());  	}  
	};	
	
  	template<typename Protocol> struct hash<conet::net::ip::basic_endpoint<Protocol>> {
      	std::size_t operator()(conet::net::ip::basic_endpoint<Protocol> const& __endpoint) const noexcept {
      		return std::hash<conet::net::ip::address>{}(__endpoint.address()) | std::hash<conet::net::ip::port_type>{}(__endpoint.port());
		}
    };	
	template<typename Protocol>  struct formatter<conet::net::ip::basic_endpoint<Protocol>> {  
	    constexpr auto parse(std::format_parse_context& context) const noexcept 
		{  	return context.begin(); 								}  
	
		auto format(conet::net::ip::basic_endpoint<Protocol> const& t, std::format_context& context) const noexcept
		{  	return std::format_to(context.out(), "{}", (std::stringstream{} << t).str());  	}  
	};		      
} // namespace std

#ifdef bswap16
	#undef bswap16
#endif // #ifdef bswap16
#ifdef bswap32
	#undef bswap32
#endif // #ifdef bswap16
#ifdef bswap64
	#undef bswap64
#endif // #ifdef bswap16


#endif // #ifndef __CONET_INTERNET_INCLUDED
