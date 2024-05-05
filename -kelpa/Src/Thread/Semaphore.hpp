/** 
 * 		@Path 	Kelpa/Src/Thread/Semaphore.hpp
 * 		@Brief	Semaphore impl
 * 		@Dependency	None
 * 		@Since  2024/04/25
 * 		@Version 1st
 **/
 
#ifndef __KELPA_THREAD_SEMAPHORE_HPP__
#define __KELPA_THREAD_SEMAPHORE_HPP__

#include <mutex>

namespace Kelpa {
namespace Thread {
	
template <typename Mutex = std::mutex> struct Semaphore {
	Semaphore(Semaphore const&) = delete;
	Semaphore& operator=(Semaphore const&) = delete;
	
	constexpr Semaphore(signed long long __value = {}) noexcept: value(__value) {}
	
	constexpr decltype(auto) GetValue() const volatile noexcept {
		return value.load(std::memory_order_seq_cst); 
	}
	
	constexpr Semaphore& Wait() noexcept {
		std::unique_lock lock {mutex};
		condition.wait(lock, [this] { return value > 0; });
		value --;
		return *this;
	}
	
	constexpr Semaphore& Notify() noexcept {
		std::unique_lock lock {mutex};
		value ++;
		condition.notify_one();
		return *this;
	}
	
	constexpr bool TryWait() noexcept {
		if(value.fetch_sub(1, std::memory_order_relaxed) > 0) return true;
		value.fetch_add(1, std::memory_order_relaxed);
		return false;
	}
	
private:
	Mutex 								mutex;
	std::condition_variable_any 		condition;
	std::atomic<signed long long> 		value {};
}; 	
	
}	
}



#endif
