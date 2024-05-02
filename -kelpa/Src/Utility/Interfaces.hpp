/** 
 * 		@Path 	Kelpa/Src/Utility/Interfaces.hpp
 * 		@Brief	Some use interfaces that inherit certain properties
 * 		@Dependency	None
 * 		@Since  2024/04/25
 * 		@Version 1st
 **/
 
#ifndef __KELPA_UTILITY_INTERFACES_HPP__
#define __KELPA_UTILITY_INTERFACES_HPP__
#include <typeindex>			/* imports ./ { 
	std::typeindex::hash_code 
	std::typeinfo
}*/
#include "./Functions.hpp"		/* imports ./ { 
	TypeName() 
}*/
namespace Kelpa {
namespace Utility {
	
struct noncopyable {
	constexpr noncopyable(noncopyable &&) noexcept = default;
	constexpr noncopyable& operator=(noncopyable &&) noexcept = default;
	constexpr noncopyable() noexcept = default;
private:
	noncopyable(noncopyable const&) = delete;
	noncopyable& operator=(noncopyable const&) = delete;
};
	
struct nonmoveable {
	constexpr nonmoveable(nonmoveable const &) noexcept = default;
	constexpr nonmoveable& operator=(nonmoveable const&) noexcept = default;
	constexpr nonmoveable() noexcept = default;
private:
	nonmoveable(nonmoveable &&) = delete;
	nonmoveable& operator=(nonmoveable &&) = delete;
};		

template <typename T> struct TypeInfo {
	constexpr std::string_view 	TypeName() 	const noexcept 
	{	return Kelpa::Utility::TypeName<T>();			}
	constexpr std::size_t	HashCode() 		const noexcept 
	{	return std::type_index(typeid(T)).hash_code();	}
};

template <typename __SubWriter> struct Writer {
	template <typename... Args> constexpr decltype(auto) Write(Args&& ...args) noexcept {
		return static_cast<__SubWriter &>(*this).template Write(std::forward<Args>(args) ...);
	}  
};

template <typename __SubReader> struct Reader {
	template <typename... Args> constexpr decltype(auto) Read(Args&& ...args) noexcept {
		return static_cast<__SubReader &>(*this).template Read(std::forward<Args>(args) ...);
	}  
};

template <typename T> struct SharedFrom {
	template <typename... Args> /*static*/constexpr std::shared_ptr<T> Shared(Args&& ...args) noexcept {
		return static_cast<T &>(*this).template Read(std::forward<Args>(args) ...);
	}  
};

template <typename T> struct UniqueFrom {
	template <typename... Args> /*static*/constexpr std::unique_ptr<T> Unique(Args&& ...args) noexcept {
		return static_cast<T &>(*this).template Read(std::forward<Args>(args) ...);
	}  
};
	
}
}



#endif
