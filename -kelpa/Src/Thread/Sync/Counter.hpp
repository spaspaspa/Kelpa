/** 
 * 		@Path 	Kelpa/Src/Thread/SyncQueue.hpp
 * 		@Brief	Thread-safe counter holding a lock	
 * 		@Dependency	None
 * 		@Since  2024/04/25
 * 		@Version 1st
 **/

#ifndef __KELPA_THREAD_SYNCCOUNTER_HPP__
#define __KELPA_THREAD_SYNCCOUNTER_HPP__
#include <shared_mutex>				/* imports ./ { 
	std::shared_mutex, 
	std::shared_lock 
	./mutex/ { std::unique_lock } 
}*/
#include <utility>					/* imports ./ { 
	std::exchange 
}*/
namespace Kelpa {
namespace Thread {
namespace Sync {
	
struct  SyncCounter{
	SyncCounter(SyncCounter const&) 				= delete;
	SyncCounter& operator=(SyncCounter const&) 		= delete;
	
    SyncCounter() 						noexcept 	= default;
 	SyncCounter(unsigned int __value) 	noexcept: value(__value) {}
    unsigned int get() 		const noexcept {
        std::shared_lock lock(mutex);
        return value;
    }
    void increment() 		noexcept {
        std::unique_lock lock(mutex);
        value ++;
    }
    unsigned int reset()	noexcept {
        std::unique_lock lock(mutex);
        return std::exchange(value, 0);
    }
private:
    mutable std::shared_mutex 	mutex;
    unsigned int 				value	{};
};	
	
}		//namespace Sync
}		//namespace Thread
}		//namespace Kelpa

#endif
