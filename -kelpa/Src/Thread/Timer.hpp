/** 
 * 		@Path 	Kelpa/Src/Thread/Timer.hpp
 * 		@Brief	Single-threaded timer and high-precision 
 * 				event scheduler based on timeline	
 * 		@Dependency	../Utility/ { Functions.hpp }
 * 		@Since  2024/04/25
 * 		@Version 1st
 **/
 
#ifndef __KELPA_THREAD_TIMER_HPP__
#define __KELPA_THREAD_TIMER_HPP__

#include <functional>				/* imports ./ { 	
	std::functionstd, 
	std::reference_wrapper 
}*/
#include <chrono>					/* imports ./ { 
	std::chrono::duration, 
	std::chrono::time_point
}*/
#include <map>						/* imports ./ { 
	std::multimap 
}*/
#include <thread>					/* imports ./ { 
	std::this_thread::sleep_xxx
}*/
#include <utility>					/* imports ./ {
	std::exchange
}*/
#include <mutex>					/* imports ./ { 
	std::mutex, 
	std::lock_guard 
}*/
#include "../Utility/Functions.hpp"	/* imports ./ { 
	MilliNow() 
}*/
#include "../Utility/Interfaces.hpp"/* imports ./ { 
	struct nonXXXable 
}*/
#include "../Utility/Macros.h"		/* imports ./ {
	#define __KELPA_DEFINES_STRUCT_MEMBER_TYPES__(STRUCT)
}*/
namespace Kelpa {
namespace Thread {

	
struct 		Timer;
struct 		TimerFormulateHandle;
struct 		Intervalometer;

struct TimerFormulateHandle {
	std::reference_wrapper<Timer> 		proxy;

	template<typename Rep, typename Period>	
	TimerFormulateHandle const& SetDuration(const std::chrono::duration<Rep,Period>&) const noexcept;
	TimerFormulateHandle const& SetRepeat(long long signed) 	const noexcept;
	template <typename F, typename... Args> requires std::invocable<F, Args ...>
	TimerFormulateHandle const& SetCallback(F&&, Args&&...) 	const noexcept;
	Timer& Continue() const noexcept;
};	
struct Timer {
__KELPA_DEFINES_STRUCT_MEMBER_TYPES__(Timer)
	typedef std::chrono::milliseconds 				duration_type;
	typedef std::chrono::system_clock 				clock_type;
	typedef std::chrono::system_clock::time_point 	time_point_type;
	
	static constexpr long long signed 				infinite { -1 };
	friend 			struct Intervalometer;
	friend 			struct TimerFormulateHandle;

	constexpr TimerFormulateHandle Build() noexcept 
	{ 	return { std::ref(*this) }; 	}
	
protected:
	std::chrono::milliseconds UntilWhen() const noexcept 
	{ 	return timeout; 				}
	
	Timer& Advance() noexcept 
	{	return ((void)std::exchange(timeout, timeout + duration), *this);		}
	
	Timer& Callback() 	noexcept {
		if(~repeat && ! repeat) 	return *this;
		(void) std::invoke(func);
		if(! ~repeat) 				return *this;
		repeat --;
		return *this;
	}
	
	bool operator<(Timer const& other) const noexcept 
	{	return timeout < other.timeout;		}
	
	bool StillAlive() 	const noexcept 
	{	return ! ~repeat || repeat > 0;		}
	
	std::chrono::milliseconds 	duration 	{ std::chrono::milliseconds(1000) };
	long long signed 			repeat 		{ infinite };
	std::function<void()> 		func 		{ 	[] { std::puts("default callback");	}	};
	std::chrono::milliseconds 	timeout		{};
};

template<typename Rep, typename Period>	
TimerFormulateHandle const& TimerFormulateHandle::SetDuration(const std::chrono::duration<Rep,Period>& __duration) const noexcept 
{	static_cast<Timer&>(proxy).duration = __duration;	return *this;	}

TimerFormulateHandle const& TimerFormulateHandle::SetRepeat(long long signed __repeat) const noexcept 
{	static_cast<Timer&>(proxy).repeat = __repeat;	return *this;	}

template <typename F, typename... Args> requires std::invocable<F, Args ...>
TimerFormulateHandle const& TimerFormulateHandle::SetCallback(F&& f, Args&&... args) const noexcept {
	static_assert(std::same_as<void, std::invoke_result_t<F, Args ...>>, "Can carry back a non-void return");
	static_cast<Timer&>(proxy).func = std::bind(std::forward<F>(f), std::forward<Args>(args) ...);
	return *this;
}

Timer& TimerFormulateHandle::Continue() const noexcept 
{	return static_cast<Timer&>(proxy);		}


struct Intervalometer : Utility::noncopyable {
__KELPA_DEFINES_STRUCT_MEMBER_TYPES__(Intervalometer)

	typedef typename Timer::duration_type 				duration_type;
	typedef typename Timer::clock_type 					clock_type;
	typedef typename Timer::time_point_type 			time_point_type;
			
	template <typename U> requires std::convertible_to<U, Timer>
	constexpr void Schedule(U&& timer) noexcept {	
		std::lock_guard lock { Mutex }; 
		Cues.emplace((timer.timeout = Utility::Now<>() + (std::forward<U>(timer)).duration), std::forward<U>(timer));	
	}
	void Update() noexcept {
		auto next { Cues.begin() };
		while(next != Cues.end()) {
			auto [timeout, timer] = std::move(* next);
			
			auto now = Utility::Now<>();
			std::this_thread::sleep_for(timeout > now ? timeout - now: std::chrono::system_clock::duration::zero());
			
		SCOPE_BEGIN() std::lock_guard lock { Mutex };
			next = Cues.erase(next);
			
			if(timer.Advance().StillAlive()) 	{
				auto current = Cues.emplace(timer.UntilWhen(), timer);
				if(next == Cues.end() || (* current).first < (* next).first) {
					(void) std::exchange(next, current);
				}
			}
		SCOPE_END()	
			WhenNextComes.store(next == Cues.end() ? std::chrono::milliseconds::max() : (* next).first, std::memory_order_seq_cst);
			timer.Callback();
		} 
	}	
	bool Finish() const noexcept { 		
		std::lock_guard lock {const_cast<Intervalometer &>(*this).Mutex}; 
		return Cues.empty(); 											
	}

	std::atomic<std::chrono::milliseconds>			WhenNextComes;
private:
	std::multimap<std::chrono::milliseconds, Timer> Cues;
	mutable std::mutex 								Mutex;
};	
	
	
}
}

#endif
