/** 
 * 		@Path	Kelpa/Src/Utility/Ignore.hpp
 * 		@Brief	Using RAII technology to trigger a certain end operation at the point 
 * 				in time of automatic destruction, which 
 * 				can be to close the file, free the heap memory, etc
 * 		@Dependency	None
 * 		@Since  2024/04/23
 * 		@Version 1st
 **/
 
#ifndef __KELPA_UTILITY_SCOPEGUARD_HPP__ 
#define __KELPA_UTILITY_SCOPEGUARD_HPP__
#include <functional>						/* imports ./ { 
	std::function,
	std::bind() 
	./utility/std::invoke()
}*/
#include <concepts>							/* imports ./ { 
	concept std::invocable 
}*/
namespace Kelpa {
namespace Utility {
	
struct ScopeGuard {
template <typename F, typename... Args>	requires std::invocable<F, Args ...> 
	explicit ScopeGuard(F&& f, Args&&... args) noexcept
		: func(std::bind(std::forward<F>(f), std::forward<Args>(args) ...)) {}
	
	~ScopeGuard() noexcept 
	{ if(func) (void) std::invoke(func); }
	
	ScopeGuard(ScopeGuard const&) = 				delete;
	ScopeGuard& operator=(ScopeGuard const&) = 		delete;
private:
	std::function<void()> func;
};

}
}

#endif
