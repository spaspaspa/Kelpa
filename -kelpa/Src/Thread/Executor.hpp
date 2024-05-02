/** 
 * 		@Path 	Kelpa/Src/Thread/Executor.hpp
 * 		@Brief	High availability multi-configuration thread pool
 * 		@Dependency	../Utility/ { Interfaces.hpp, ScopeGuard.hpp }
 * 		@Since  2024/04/25
 * 		@Version 1st
 **/
 
#ifndef __KELPA_THREAD_EXECUTOR_HPP__
#define __KELPA_THREAD_EXECUTOR_HPP__

#include <exception>				/* imports ./ { 
	std::exception 
}*/
#include <string>					/* imports ./ { 
	string_view/ { std::string_view } 
}*/
#include <future>					/* imports ./ { 
	std::future, 
	std::packaged_task 
}*/
#include <list>						/* imports ./ { 
	std::list 
}*/
#include <queue>					/* imports ./ { 
	std::priority_queue 
}*/
#include <cmath>					/* imports ./ { 
	std::log 
}*/
#include <algorithm>				/* imports ./ { 
	std::ranges::for_each 
}*/
#include <shared_mutex>				/* imports ./ { 
	std::shared_mutex, 
	std::shared_lock 
	./mutex { 
		std::unique_lock, 
		std::scope_lock 
	} 
}*/
#include <utility>					/* imports ./ { 
	std::exchange 
} */
#include <atomic>					/* imports ./ { 
	std::memory_order, 
	std::atomic 
}*/
#include <functional>				/* imports ./ { 
	std::function, 
	std::bind 
}*/
#include "../Utility/Interfaces.hpp"/* imports ./ { 
	struct nonXXXable 
}*/
#include "../Utility/ScopeGuard.hpp"/* imports ./ { 
	struct ScopeGuard 
}*/

#include <iostream>
#include <syncstream>
namespace Kelpa {
namespace Thread {
	
struct 		RejectedExecutionError: std::exception {
	RejectedExecutionError(std::string_view __message) noexcept: message(__message) {}
	
	char const* what() const noexcept override 
	{ 	return message.data();	}
private:	
	std::string_view message;
};	

struct 		Executable {
	typedef 					void(* 	Assignment);

	enum class Priority: unsigned char {
		DISCARD, UNHURRIED, STANDARD, URGENT, HARSH,		
	}									priority;
	std::packaged_task<void()>  		assignment;

	template <typename Callable> requires std::is_invocable_r_v<void, Callable>
	constexpr explicit Executable(Priority __priority, Callable&& __assignment) 
		noexcept: priority(__priority), assignment(std::forward<Callable>(__assignment)) {}

	constexpr bool operator<(Executable const& other) const 
	{ 	return priority < other.priority;	}	

	constexpr bool operator>(Executable const& other) const
	{ 	return priority > other.priority;	}	
};
typedef typename Executable::Priority 	Priority;
typedef enum class RejectionPolicy: unsigned char {
	ABORT,		CALLER_RUNS,	DISCARD,	DISCARD_OLDEST,
} 										RejectedPolicy;

struct 							Executor;
template <typename T> struct 	IRefer { 
	std::reference_wrapper<T> 		refer; 

	constexpr T& deref() const noexcept 
	{	return static_cast<T &>(refer);		}
};
struct SubmitHandle: IRefer<Executor> {
	template <typename F, typename... Args> requires std::invocable<F, Args ...>
	auto Submit(Priority priority, F&& f, Args&& ...args) const
						-> std::future<std::invoke_result_t<F, Args ...>>;
};
struct ActiveHandle: IRefer<Executor> {
	SubmitHandle Activate() const noexcept;
};
struct FormulateHandle: IRefer<Executor> {
	ActiveHandle 	Continue() 												const noexcept;
	FormulateHandle const& 	SetJoinable(bool joinable) 						const noexcept;
	FormulateHandle const& 	SetRejectionPolicy(RejectionPolicy policy) 		const noexcept;
	FormulateHandle const& 	SetAssignmentThrottle(signed int throttle) 		const noexcept;
	FormulateHandle const& 	SetInitialThread(signed int initial) 					const noexcept;
	template<typename Rep = std::uint_least64_t, typename Period = std::milli>
	FormulateHandle const& 	SetDynamicUpdateCycle(std::chrono::duration<Rep,Period> const& duration) 	const noexcept;
	template<typename F, typename... Args, typename Rep = std::int64_t, typename Period = std::milli> requires std::invocable<F, Args ...>
	FormulateHandle const& 	SetWaitingInterval(std::chrono::duration<Rep,Period> const& duration, F&&f, Args&&... args)  const noexcept;
	FormulateHandle const& 	SetDynamicUpdateRange(signed int min, signed int max) 	const noexcept;	
};

struct Executor: Utility::noncopyable, Utility::nonmoveable {
	friend 			struct FormulateHandle;
	friend 			struct ActiveHandle;
	friend 			struct SubmitHandle;
	
	typedef bool 										JOINABLE;
	typedef std::chrono::milliseconds 					duration_type;
	typedef std::chrono::system_clock::time_point 		time_point_type;
	Executor() 						noexcept;
   ~Executor()						noexcept;
	constexpr FormulateHandle 	Spawn()& 	noexcept;
private:
bool 	ShrinkSome(signed int) 				noexcept;
bool 	ExtendSome(signed int) 				noexcept; 	

	typedef typename std::list<Executable>::iterator 	Iterator;
	struct Compare {
		bool operator()(Iterator one, Iterator two) const noexcept
		{	return std::greater<>{} (* one, * two);		} 
	};
	std::list<Executable>								Container;
	std::priority_queue<
		Iterator, 	std::vector<Iterator>,	Compare
		>												Indexer;
	RejectedPolicy										policy 		{ RejectedPolicy::ABORT };
	signed int 											throttle 	{ (signed int) (std::thread::hardware_concurrency() << 1 | 1) };
	
	mutable std::shared_mutex							mutex;		
	std::condition_variable_any							condition;
	std::vector<std::thread>							threads;
	
	std::atomic<bool>									active 		{ false };
	JOINABLE											joinable	{ true };
	
	std::function<void(std::size_t)>					busyloop	{ nullptr };
	std::function<void()>								watchloop   { nullptr };

	std::atomic<signed int>								engage 		{};
	std::atomic<signed int>								survive 	{};
	std::atomic<signed int>								rest 		{};
	
	signed int 											initial 	{ (signed int) std::thread::hardware_concurrency() };
	signed int 											max 		{ (signed int) (std::thread::hardware_concurrency() << 1 | 1) };
	signed int 											min 		{ (signed int) (std::thread::hardware_concurrency() >> 1 & (~1)) };
	
	std::chrono::milliseconds							duration	{ std::chrono::milliseconds(256) };
	
	std::chrono::milliseconds							waitfor		{ std::chrono::milliseconds(512) };
	std::function<void()>								atexit      { nullptr };
	
	std::atomic<signed int>								shrink		{};
	std::queue<std::size_t>								expire		{};
};
Executor::Executor() noexcept: busyloop([this] (std::size_t index) mutable {
	survive ++;
	Utility::ScopeGuard elapse( [this] { 
		survive --;
	} ); 
	while(active.load(std::memory_order_seq_cst)) {
		std::packaged_task<void()> 	assignment;
		std::unique_lock unique { mutex };
/* 
	this is an optional part 
		---|
			\
			|
		--- | ---
*/
//		if(	survive > initial && ! rest.load(std::memory_order_seq_cst)
//		&& 	!condition.wait_for(unique, keepalive, [this] { return ! active.load(std::memory_order_seq_cst); } )
//		) 	return (void) expire.emplace(index); 


		condition.wait(unique, [this] { 
			return ! active.load(std::memory_order_seq_cst) 
			|| 		 shrink.load(std::memory_order_seq_cst)
			||     	 rest.load(std::memory_order_seq_cst);
		});
		
		if(!active.load(std::memory_order_seq_cst) ) 
			return;
		if(shrink.load(std::memory_order_seq_cst))  {
			if(!atexit.operator bool() || !condition.wait_for(unique, waitfor, [this] { 
				return ! 	active.load(std::memory_order_seq_cst) 
				|| 			rest.load(std::memory_order_seq_cst);
			})) {
				if(atexit.operator bool()) 
					std::invoke(atexit);
				return (void) (expire.emplace(index) && shrink --);
			}
			if(! active.load(std::memory_order_seq_cst) ) return;	
		}
		if((* Indexer.top()).priority == Priority::DISCARD) {
			(* Indexer.top()).assignment.make_ready_at_thread_exit();
			Container.erase(Indexer.top());	Indexer.pop();
			continue;
		}
		assignment = std::move((* Indexer.top()).assignment);
		Container.erase(Indexer.top());		Indexer.pop();
		
		rest --;			unique.unlock();
		engage ++;	 		assignment();			
		engage --;	 				
	}	
}), watchloop([this] {
	static auto dyn_dura = duration;
	while(active.load(std::memory_order_seq_cst)) {
			
		std::osyncstream(std::cout) << "remaining tasks:\t" << rest.load(std::memory_order_seq_cst) << "\n";
		if(ShrinkSome(survive - 	engage) && ExtendSome(rest - 	(survive - 	engage)))
			std::this_thread::sleep_for(std::exchange(dyn_dura, duration));
		else 
			std::this_thread::sleep_for(std::exchange(dyn_dura, std::chrono::milliseconds((std::uint_least64_t) std::log2(dyn_dura.count() + 60)))); 
	} 
}) {}
bool Executor::ShrinkSome(signed int many) noexcept {
	if(many <= 0 || survive <= min) 		
		return false;
	shrink.store(std::min(survive - min, many), std::memory_order_seq_cst);
	condition.notify_all();		
	return true;
}
bool Executor::ExtendSome(signed int many) noexcept {
	if(many <= 0 || survive >= max) 		
		return false;
	many = std::min(max - survive, many);
	std::lock_guard guard { mutex };

	while(many -- && ! expire.empty()) {
		threads[expire.front()] = std::thread(std::bind(busyloop, expire.front()));
		expire.pop();
	}
	while(many --) threads.emplace_back(std::bind(busyloop, threads.size()));		
	return true;
}

Executor::~Executor() noexcept {
	active.store(false, std::memory_order_seq_cst);
	condition.notify_all();

	if(!joinable)		
		return;
	for(auto& t: threads) 		
		if(t.joinable()) 	
			t.join();
}


constexpr FormulateHandle Executor::Spawn()& noexcept 
{	return FormulateHandle 	{ std::ref(*this) 	};	}

ActiveHandle 	FormulateHandle::Continue() const noexcept 
{	return ActiveHandle 	{  std::move(refer) };	}

FormulateHandle const& 			FormulateHandle::SetJoinable(bool joinable) 				const noexcept 
{	deref().joinable = joinable; return *this;					}

FormulateHandle const& 			FormulateHandle::SetRejectionPolicy(RejectionPolicy policy) const noexcept 
{	deref().policy = policy;	 return *this;					}

FormulateHandle const& 			FormulateHandle::SetAssignmentThrottle(signed int throttle) const noexcept 
{	deref().throttle = throttle; return *this;					}	

FormulateHandle const& 			FormulateHandle::SetInitialThread(signed int initial) 			const noexcept
{	deref().initial = initial;		 return *this;					}

template<typename Rep, typename Period>
FormulateHandle const& 			FormulateHandle::SetDynamicUpdateCycle(std::chrono::duration<Rep,Period> const& duration) const noexcept {
	deref().duration = std::chrono::duration_cast<std::chrono::milliseconds>(duration); 	
	return *this;
}

template<typename F, typename... Args, typename Rep, typename Period> requires std::invocable<F, Args ...>
FormulateHandle const& 	FormulateHandle::SetWaitingInterval(std::chrono::duration<Rep,Period> const& duration, F&&f, Args&&... args)  const noexcept {	
	deref().waitfor = std::chrono::duration_cast<std::chrono::milliseconds>(duration);	
	deref().atexit = std::bind(std::forward<F>(f), std::forward<Args>(args) ...);
	return *this;
}

FormulateHandle const& 			FormulateHandle::SetDynamicUpdateRange(signed int min, signed int max) 	const noexcept {
	deref().min = min; 	
	deref().max = max;			
	return *this;
}	

SubmitHandle ActiveHandle::Activate() const noexcept {
	deref().active.store(true, std::memory_order_seq_cst);
	
	signed int counter { static_cast<signed int>(deref().initial) };

	deref().threads.emplace_back(deref().watchloop);	
	while(counter --) 	
		deref().threads.emplace_back(std::bind(deref().busyloop, deref().threads.size()));
	
	if(!deref().joinable) 
		std::ranges::for_each(deref().threads, [] (auto&& t) { 
			t.detach();	
	});

	return SubmitHandle { std::move(refer) };
}

template <typename F, typename... Args> requires std::invocable<F, Args ...>
auto SubmitHandle::Submit(Priority priority, F&& f, Args&& ...args) const
	-> std::future<std::invoke_result_t<F, Args ...>>{
	using ReturnType = std::invoke_result_t<F, Args ...>;
	
	std::shared_lock share { deref().mutex };
	if(deref().rest.load(std::memory_order_seq_cst) == deref().throttle) {
		if(deref().policy == RejectionPolicy::ABORT) 
			throw RejectedExecutionError { "too many tasks" };
		if(deref().policy == RejectionPolicy::CALLER_RUNS) 
			return std::async(std::launch::deferred, std::forward<F>(f), std::forward<Args>(args) ...);
		if(deref().policy == RejectionPolicy::DISCARD) 
			return std::future<ReturnType> {};
		if(deref().policy == RejectionPolicy::DISCARD_OLDEST) {
			(void) std::exchange(deref().Container.front().priority, Priority::DISCARD);
			deref().rest --;
		}
	}
	share.unlock();
	
	auto temporary = std::make_shared<std::packaged_task<ReturnType()>>(
		std::bind(std::forward<F>(f), std::forward<Args>(args) ...)
	);
	auto future = (* temporary).get_future();
{
	std::lock_guard guard { deref().mutex };
	deref().Container.emplace_back(priority, [temporary] { (void) std::invoke(* temporary); });
	deref().Indexer.emplace(std::prev(deref().Container.end()));
}	
	deref().rest ++;	deref().condition.notify_one();
	return future;	
}
	
	
}
}


#endif
