/** 
 * 		@Path 	Kelpa/IOSchedule/Reactor.hpp
 * 		@Brief	The asynchronous scheduler listening file descriptors for associated events
 * 		@Dependency ../Coroutine 	{ Coroutine.hpp,  }
 * 					../Thread 		{ Executor.hpp, Timer.hpp 	}
 * 					../Utility      { Macros.h, Interfaces.hpp, UniqueAny.hpp }
 *		@Since  2024/04/25
 		@Version 1st
 **/
 
#ifndef __KELPA_IOSCHEDULE_REACTOR_HPP__
#define __KELPA_IOSCHEDULE_REACTOR_HPP__
#include <windows.h>					/* imports ./ { 
	fd_set, 
	#define 
	FD_XXX, 
	select 
}*/
#include <coroutine>					/* imports ./ { 
	std::coroutine_handle<> 
}*/	
#include <cstring>						/* imports ./ { 
	std::memset 
}*/		
#include "../Thread/Executor.hpp"		/* imports ./ { 
	struct Executor, 
	struct SubmitHandle 
}*/
#include "../Thread/Timer.hpp"			/* imports ./ { 
	struct Timer, 
	struct Intervalometer 
}*/
#include "../Utility/Macros.h"			/* imports ./ {
	#define __KELPA_DEFINES_STRUCT_MEMBER_TYPES__(STRUCT)
}}*/
#include "../Utility/Interfaces.hpp"	/* imports ./ { 
	struct nonXXXable
} */
#include "../Coroutine/Coroutine.hpp"	/* imports ./ { 
	struct MainPromise 
}*/
#include "../Utility/UniqueAny.hpp"		/* imports ./ { 
	struct UniqueAny 
}*/

namespace Kelpa {
namespace IOSchedule {
	
//struct CoUnique {
//	constexpr CoUnique() 	noexcept = default;
//	constexpr CoUnique(std::coroutine_handle<> coroutine) noexcept:
//		coroutine_(coroutine) {}
//	CoUnique(CoUnique const&) 			= delete;
//	CoUnique& operator=(CoUnique const&) 	= delete;
//	~CoUnique() noexcept {
//		if(not coroutine_.operator bool()) 	
//			return;
//		coroutine_.destroy();
//	}
//	std::coroutine_handle<> coroutine_;
//};	
typedef Utility::UniqueAny<std::coroutine_handle<>, decltype([](auto&& coroutine) {
	if(coroutine.operator bool()) coroutine.destroy();
})> CoUnique;	
struct Reactor : Utility::noncopyable {
__KELPA_DEFINES_STRUCT_MEMBER_TYPES__(Reactor)	

	typedef signed int 					file_discriptor_type;
	typedef std::coroutine_handle<> 	coroutine_type;

	enum class Event: unsigned char { 		NOEVENT = 0X00, READ = 0x01, WRITE = 0x02, EXCEPT = 0x04 		};

	Reactor& 	Listen(Event event, signed fd, Coroutine::WeakFuture<> const& future) 	noexcept;

	template <typename F, typename... Args> requires std::invocable<F, Args ...>
	Reactor& 	Listen(Event event, signed fd, F&&f, Args&&... args) 					noexcept;

	Reactor& 	Cancel(signed fd, Event event = static_cast<Event>((unsigned char) 0x07)) noexcept;

	Event 		LookUp(signed fd) 			const noexcept;

	Reactor&    Clear()						noexcept;

	Reactor() 	noexcept;
	~Reactor() 	noexcept { 	exit.store(true, std::memory_order_seq_cst);	}
	
	Thread::Intervalometer				lometer;
private:
	std::unordered_multimap<signed, std::function<void()>> 		ReadCallbacks;
	std::unordered_multimap<signed, CoUnique>	ReadFibers;
	
	std::unordered_multimap<signed, std::function<void()>>  	WriteCallbacks;
	std::unordered_multimap<signed, CoUnique>	WriteFibers;
	
	std::unordered_multimap<signed, std::function<void()>>  	ExceptCallbacks;
	std::unordered_multimap<signed, CoUnique>	ExceptFibers;
	
	std::atomic<bool>					exit 		{ false };
	std::atomic<signed>					throttle 	= 0;
	
	std::unique_ptr<Thread::Executor> 	executor 	= std::make_unique<Thread::Executor>();
	Thread::SubmitHandle 				handle 		= (* executor).Spawn().SetJoinable(false).Continue().Activate();
	
	fd_set 								ReadSet;
	fd_set 								WriteSet;
	fd_set								ExceptSet;
	
	std::shared_mutex					mutex;
};	
typedef typename Reactor::Event 		Event;
 
constexpr Event operator|(Event a, Event b) noexcept {  
    typedef std::underlying_type_t<Event> T;  
    return static_cast<Event>(static_cast<T>(a) | static_cast<T>(b));  
}  
constexpr Event operator&(Event a, Event b) noexcept {  
    using T = std::underlying_type_t<Event>;  
    return static_cast<Event>(static_cast<T>(a) & static_cast<T>(b));  
}
constexpr Event operator|=(Event a, Event b) noexcept {  
    typedef std::underlying_type_t<Event> T;  
    return (a = static_cast<Event>(static_cast<T>(a) | static_cast<T>(b)));  
}  
constexpr Event operator&=(Event a, Event b) noexcept {  
    using T = std::underlying_type_t<Event>;  
    return (a = static_cast<Event>(static_cast<T>(a) & static_cast<T>(b)));  
} 
 
constexpr bool HasEvent(Event event) noexcept {
	return 	event == Event::NOEVENT 
	|| (	static_cast<std::underlying_type_t<Event>>((event & Event::READ))	)
	|| (	static_cast<std::underlying_type_t<Event>>((event & Event::WRITE))	)
	|| (	static_cast<std::underlying_type_t<Event>>((event & Event::EXCEPT))	);
} 
 
Reactor::Reactor() noexcept {
	FD_ZERO(std::addressof(ReadSet));
	FD_ZERO(std::addressof(WriteSet));
	FD_ZERO(std::addressof(ExceptSet));
	
	lometer.Schedule(Thread::Timer()
	.Build()
	.SetDuration(std::chrono::milliseconds(500))
	.SetRepeat(Thread::Timer::infinite)
	.SetCallback([this] {
		std::shared_lock 	lock 			{mutex};	//locking op might as well be removed
	
		fd_set 				tempReadSet 	= ReadSet;
		fd_set 				tempWriteSet 	= WriteSet;
		fd_set				tempExceptSet   = ExceptSet;
	
		struct timeval tVal;
		std::memset(std::addressof(tVal), 0, sizeof tVal);
		tVal.tv_usec = (lometer.WhenNextComes.load(std::memory_order_seq_cst) - Utility::Now<>()).count();
		

		if(!~select(throttle + 1, std::addressof(tempReadSet), std::addressof(tempWriteSet), nullptr, std::addressof(tVal))) {
			std::perror("select");
			std::quick_exit(EXIT_FAILURE);		
		}
		for(signed int fd {}; fd < throttle + 1; fd ++) {
			if(FD_ISSET(fd, std::addressof(tempReadSet))) 
				for(auto it { ReadCallbacks.equal_range(fd).first }; it != ReadCallbacks.equal_range(fd).second; it ++)
					handle.Submit(Thread::Priority::STANDARD, ( *it).second);
			if(FD_ISSET(fd, std::addressof(tempWriteSet))) 
				for(auto it { WriteCallbacks.equal_range(fd).first }; it != WriteCallbacks.equal_range(fd).second; it ++)
					handle.Submit(Thread::Priority::STANDARD, (* it).second);
			if(FD_ISSET(fd, std::addressof(tempExceptSet))) 
				for(auto it { ExceptCallbacks.equal_range(fd).first }; it != ExceptCallbacks.equal_range(fd).second; it ++)
					handle.Submit(Thread::Priority::URGENT, (* it).second);
		}
	}).Continue());
	handle.Submit(Thread::Priority::HARSH, [this] {	
		while(!exit.load(std::memory_order_seq_cst)) 
			if(not lometer.Finish())	
				lometer.Update();	
	});
}

constexpr std::string NameOfEvent(Event event) noexcept {
	if(not HasEvent(event)) 
		return "NOEVENT";
	
	std::string name;
	if(static_cast<std::underlying_type_t<Event>>((event & Event::READ))) 	
		name += "READ |";
	if(static_cast<std::underlying_type_t<Event>>((event & Event::WRITE))) 		
		name += "WRITE |";
	if(static_cast<std::underlying_type_t<Event>>((event & Event::EXCEPT))) 
		name += "EXCEPT |";
	if(name.empty()) 
		return "UNKNOWN";
	return name.erase(name.length() - 1);
}

std::ostream& operator<<(std::ostream& Os, Event event) noexcept {
	return (Os << NameOfEvent(event));
}


Reactor& Reactor::Listen(Event event, signed fd, Coroutine::WeakFuture<> const& future) 	noexcept {	
	if(not HasEvent(event)) 
		return *this;
	
	throttle.store(std::max(throttle.load(std::memory_order_seq_cst), fd), std::memory_order_seq_cst);
	
	std::unique_lock lock {mutex};
	
	if(static_cast<std::underlying_type_t<Event>>((event & Event::READ))) 	{
		if(!FD_ISSET(fd, std::addressof(ReadSet)))
			FD_SET(fd, std::addressof(ReadSet));
		ReadFibers.emplace(fd, future.operator std::coroutine_handle<>());
		ReadCallbacks.emplace(fd, [coroutine = future.operator std::coroutine_handle<>()]{
			std::puts("resume");
			coroutine.resume();
		});
	}
	if(static_cast<std::underlying_type_t<Event>>((event & Event::WRITE))) 	{
		if(!FD_ISSET(fd, std::addressof(WriteSet)))
			FD_SET(fd, std::addressof(WriteSet));
		WriteFibers.emplace(fd, future.operator std::coroutine_handle<>());
		WriteCallbacks.emplace(fd, [coroutine = future.operator std::coroutine_handle<>()]{
			std::puts("resume");
			coroutine.resume();
		});
	}	
	if(static_cast<std::underlying_type_t<Event>>((event & Event::EXCEPT))) 	{
		if(!FD_ISSET(fd, std::addressof(ExceptSet)))
			FD_SET(fd, std::addressof(ExceptSet));
		ExceptFibers.emplace(fd, future.operator std::coroutine_handle<>());
		ExceptCallbacks.emplace(fd, [coroutine = future.operator std::coroutine_handle<>()]{
			std::puts("resume");
			coroutine.resume();
		});
	}	
	
	return *this;	
}

template <typename F, typename... Args> requires std::invocable<F, Args ...>
Reactor& Reactor::Listen(Event event, signed fd, F&&f, Args&&... args) noexcept {
	if(not HasEvent(event)) 
		return *this;
	
	throttle.store(std::max(throttle.load(std::memory_order_seq_cst), fd), std::memory_order_seq_cst);
	
	std::unique_lock lock {mutex};
	
	if(static_cast<std::underlying_type_t<Event>>((event & Event::READ))) 	{
		if(!FD_ISSET(fd, std::addressof(ReadSet)))
			FD_SET(fd, std::addressof(ReadSet));
		ReadCallbacks.emplace(fd, std::bind(std::forward<F>(f), std::forward<Args>(args) ...));
	}
	if(static_cast<std::underlying_type_t<Event>>((event & Event::WRITE))) 	{
		if(!FD_ISSET(fd, std::addressof(WriteSet)))
			FD_SET(fd, std::addressof(WriteSet));
		WriteCallbacks.emplace(fd, std::bind(std::forward<F>(f), std::forward<Args>(args) ...));		
	}	
	if(static_cast<std::underlying_type_t<Event>>((event & Event::EXCEPT))) 	{
		if(!FD_ISSET(fd, std::addressof(ExceptSet)))
			FD_SET(fd, std::addressof(ExceptSet));
		ExceptCallbacks.emplace(fd, std::bind(std::forward<F>(f), std::forward<Args>(args) ...));		
	}	
	
	return *this;
}
Reactor& Reactor::Cancel(signed fd, Event event) 			noexcept {
	std::unique_lock lock {mutex};
	
	if(static_cast<std::underlying_type_t<Event>>((event & Event::READ))) 	{
		FD_CLR(fd, std::addressof(ReadSet));
		ReadCallbacks.erase(fd);
		ReadFibers.erase(fd);
	}
	if(static_cast<std::underlying_type_t<Event>>((event & Event::WRITE))) 	{
		FD_CLR(fd, std::addressof(WriteSet));
		WriteCallbacks.erase(fd);
		WriteFibers.erase(fd);		
	}	
	if(static_cast<std::underlying_type_t<Event>>((event & Event::EXCEPT))) 	{
		FD_CLR(fd, std::addressof(ExceptSet));
		ExceptCallbacks.erase(fd);
		ExceptFibers.erase(fd);		
	}
	return *this;
}
Event 		Reactor::LookUp(signed fd) 		const noexcept {
	Event 		result = Event::NOEVENT;
	if(FD_ISSET(fd, std::addressof(ReadSet))) 
		result |= Event::READ;
	if(FD_ISSET(fd, std::addressof(WriteSet))) 
		result |= Event::WRITE;
	if(FD_ISSET(fd, std::addressof(ExceptSet))) 
		result |= Event::EXCEPT;
	return 		result;
}
Reactor&    Reactor::Clear()				noexcept {
	FD_ZERO(std::addressof(ReadSet));
	FD_ZERO(std::addressof(WriteSet));	
	FD_ZERO(std::addressof(ExceptSet));

	std::decay_t<decltype(ReadCallbacks)>().swap(ReadCallbacks);
	std::decay_t<decltype(ReadCallbacks)>().swap(WriteCallbacks);
	std::decay_t<decltype(ReadCallbacks)>().swap(ExceptCallbacks);

	std::decay_t<decltype(ReadFibers)>().swap(ReadFibers);
	std::decay_t<decltype(WriteFibers)>().swap(WriteFibers);
	std::decay_t<decltype(ExceptFibers)>().swap(ExceptFibers);


	return *this;	
}


}
}

namespace std {

	
template<>  struct formatter<Kelpa::IOSchedule::Event> {  
    constexpr auto parse(std::format_parse_context& context) 					const noexcept 
	{  	return context.begin(); 							}  
	auto format(Kelpa::IOSchedule::Event const& t, std::format_context& context) 	const 
	{  	return std::format_to(context.out(), "{}", Kelpa::IOSchedule::NameOfEvent(t));  	}  
}; 


	
}

#endif
