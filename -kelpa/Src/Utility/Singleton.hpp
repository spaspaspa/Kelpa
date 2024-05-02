/** 
 *				@Path			Kelpa/Src/Utility/Singleton.hpp
 *   			@Brief			A template-type singleton interface 
 * 								that creates globally unique instances 
 * 								of a particular type in a thread-safe manner
 * 				@Dependency		./ { Concepts.hpp }
 * 				@Since 			2024/04/23
 * 				@Version        1st
 **/
 
#ifndef __KELPA_UTILITY_SINGLETON_HPP__
#define __KELPA_UTILITY_SINGLETON_HPP__
#include "./Concepts.hpp"		/* imports ./ { 
	concept non_reference 
}*/
#include <mutex>				/* imports mutex/ { 
	std::mutex 
	std::memory_order
}*/
namespace Kelpa {
namespace Utility {
	
template <typename T, std::size_t = 0> 	requires non_reference<T> struct Singleton {
template <typename... Args>				requires std::constructible_from<T, Args ...>
	static T& Get(Args&& ...args) {
		T * temporary = pointer.load(std::memory_order_relaxed);
		std::atomic_thread_fence(std::memory_order_acquire);
		if(! temporary) {
			std::lock_guard guard {  mutex };
			temporary = pointer.load(std::memory_order_relaxed);
			if(! temporary) {
				temporary = new T { std::forward<Args>(args) ... };
				static auxiliary_delete deletor {};
				std::atomic_thread_fence(std::memory_order_relaxed);
			}
		}
		if(temporary) pointer.store(temporary, std::memory_order_seq_cst);
		return 		* pointer;
	}
private:    
	static std::mutex 					mutex;
	static std::atomic<T *> 			pointer;
struct auxiliary_delete {
	~auxiliary_delete() 		noexcept {	auxiliary();	}
private:	
	constexpr void auxiliary() 	const volatile noexcept {
		if(! Singleton::pointer.load()) return;
 		delete pointer;
 		pointer.store(nullptr);
	};
};
};
template <typename T, std::size_t I> requires non_reference<T>
std::atomic<T *> 		Singleton<T, I>::pointer { nullptr };
template <typename T, std::size_t I> requires non_reference<T>
std::mutex 				Singleton<T, I>::mutex {};
	

}
}




#endif
