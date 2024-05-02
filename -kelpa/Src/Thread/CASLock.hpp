/** 
 * 		@Path 	Kelpa/Src/Thread/CASLock.hpp
 * 		@Brief	Compare and exchange lock impl
 * 		@Dependency	None
 * 		@Since  2024/04/25
 * 		@Version 1st
 **/
 
#ifndef __KELPA_THREAD_CASLOCK_HPP__
#define __KELPA_THREAD_CASLOCK_HPP__

#include <atomic>					/* imports ./ { 
	std::atomic_bool 
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
	
struct CASLock {  
    CASLock(CASLock const&) 								= delete;
    CASLock& operator=(CASLock const&) 						= delete;
    CASLock(CASLock &&) 				noexcept 			= default;
    CASLock& operator=(CASLock &&) 		noexcept 			= default;
	constexpr CASLock() 				noexcept 			= default;
 
 	void swap(CASLock& other)  noexcept
 	{ 	std::swap(value, other.value);	}  

	void lock() {  	
		if(!value) throw std::system_error { std::make_error_code(std::errc::operation_not_permitted) }; 	
		bool expected { false }; 
		while (!(* value).compare_exchange_weak(expected, true, std::memory_order_release, std::memory_order_relaxed)) 
			expected = false;   		
	}  
  
    void unlock() {  		
   		if(!value) throw std::system_error { std::make_error_code(std::errc::operation_not_permitted) };			 
		(* value).store(false, std::memory_order_seq_cst);  				
	}  
  
    bool try_lock()  {  		
		if(!value) throw std::system_error { std::make_error_code(std::errc::operation_not_permitted) };					
		return !(* value).exchange(true, std::memory_order_acquire);  		
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
		return (* value).load(std::memory_order_seq_cst);	
	}

	bool owns_lock() const 
	{	return static_cast<bool>( *this);	}
private:  
    std::unique_ptr<std::atomic<bool>> 		value { new std::atomic<bool>(false) };  
};  	
	
	
}
}

#endif
