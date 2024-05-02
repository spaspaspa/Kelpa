/** 
 * 		@Path 	Kelpa/Src/Thread/SpinLock.hpp
 * 		@Brief	Spin lock impl
 * 		@Dependency	None
 * 		@Since  2024/04/25
 * 		@Version 1st
 **/
 
#ifndef __KELPA_THREAD_SPINLOCK_HPP__
#define __KELPA_THREAD_SPINLOCK_HPP__

#include <atomic>					/* imports ./ { 
	std::atomic_flag 
	./system_error {
		std::make_error_code, 
		std::system_error, std::errc()
	}
} */
#include <thread>					/* imports ./ { 
	std::this_thread::sleep_xxx 
}*/
#include <chrono>					/* imports ./ { 
	std::duration, 
	std::time_point 
	./ algorithm { std::swap }
}*/
#include <memory>					/* imports ./ { 
	std::unique_ptr 
}*/

namespace Kelpa {
namespace Thread {
	
struct SpinLock {
	SpinLock(SpinLock const&)						= delete;
	SpinLock& operator=(SpinLock const&)			= delete;
	
	constexpr SpinLock() 	noexcept 				= default;
	SpinLock(SpinLock &&) 	noexcept				= default;
	SpinLock& operator=(SpinLock &&) noexcept	 	= default;

	void swap(SpinLock& other)  noexcept
	{ 	std::swap(value, other.value);	}

	void lock() 	{
		if(!value) 
			throw std::system_error { std::make_error_code(std::errc::operation_not_permitted) }; 
		while (std::atomic_flag_test_and_set_explicit(std::addressof((* value)), std::memory_order_acquire))
			std::this_thread::yield();
	}

	void unlock() 	{
		if(!value) 
			throw std::system_error { std::make_error_code(std::errc::operation_not_permitted) };			
		(* value).clear(std::memory_order_release);		
	}

	bool try_lock() {	
		if(!value) 
			throw std::system_error { std::make_error_code(std::errc::operation_not_permitted) };		
		return !(* value).test_and_set(std::memory_order_acquire);	 	
	}

	template< class Rep, class Period >
	bool try_lock_for(std::chrono::duration<Rep, Period> const& duration)  {
		if(!value) 
			throw std::system_error { std::make_error_code(std::errc::operation_not_permitted) };
		if(owns_lock()) 
			throw std::system_error { std::make_error_code(std::errc::resource_deadlock_would_occur) };

		auto const theEnd 	{ std::chrono::high_resolution_clock::now() + duration };

		while(std::chrono::high_resolution_clock::now() < theEnd) 	
			if(try_lock()) 	
				return true;
		return 	false;
	} 	

	template< class Clock, class Duration >
	bool try_lock_until(std::chrono::time_point<Clock, Duration> const& timepoint)  
	{	return try_lock_for(timepoint - std::chrono::high_resolution_clock::now());		}

	operator bool() const {
		if(!value) 
			throw std::system_error { std::make_error_code(std::errc::operation_not_permitted) };		
		return (* value).test();	
	}

	bool owns_lock() const 
	{	return static_cast<bool>( *this);	}
private:
	std::unique_ptr<std::atomic_flag> value { new std::atomic_flag {ATOMIC_FLAG_INIT} };
};	
	
	
	
}
}


namespace std {
template<> void swap<Kelpa::Thread::SpinLock>(Kelpa::Thread::SpinLock& one, Kelpa::Thread::SpinLock& two) noexcept
{			one.swap(two);			}
}

#endif
