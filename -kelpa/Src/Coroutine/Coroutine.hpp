/** 
 * 		@Path 	Kelpa/Src/Coroutine/Coroutine.hpp
 * 		@Brief	A variety of encapsulated Awaitable/Promise/Future objects with certain properties
 * 		@Dependency	../Utility/ { Ignore.hpp, VoidGuard.hpp, Deferrable.hpp }
 * 		@Since  2024/04/25
 * 		@Version 1st
 **/
 
#ifndef __KELPA_COROUTINE_COROUTINE_HPP__
#define __KELPA_COROUTINE_COROUTINE_HPP__

#include <coroutine>				/* imports ./ { 
	std::coroutine_handle, 
	std::noop_coroutine_handle,
	std::suspend_always, std::suspend_never, ...
}*/
#include <chrono>					/* imports ./ { 
	std::duration, 
	std::time_point, 
	std::now, ... 
}*/
#include <map>						/* imports ./ { 
	std::multimap 
}*/
#include <thread>					/* imports ./ { 
	std::this_thread::sleep_xxx 
}*/
#include "../Utility/Ignore.hpp" 	/* imports ./ { 
	struct Ignore 
	./tuple/ { std::tuple }
}*/
#include "../Utility/Deferrable.hpp"/* imports ./ { 
	struct Utility::Deferrable 
	./variant/ { std::variant }
}*/
#include "../Utility/VoidGuard.hpp" /* imports ./ { 
	struct Utility::VoidGuard 
}*/
#include "../Utility/Concepts.hpp"  /* imports ./ { 
	concept Awaitable, 
	concept Utility::Awaiter 
}*/
#include "./Generator.hpp"			/* imports ./ { 
	struct Generator 
}*/
namespace Kelpa {
namespace Coroutine {
	

/*
	##Awaitables
*/
struct FinalSwapinAwaitable {
	constexpr bool await_ready() const noexcept { return false; }
	
	constexpr std::coroutine_handle<> await_suspend(Utility::Ignore) const noexcept 
	{	return not coroutine ? std::noop_coroutine() : coroutine;	}
	
	constexpr void await_resume() const noexcept {}
	
	std::coroutine_handle<> coroutine {};
};

struct RepeatItselfAwaitable {
	constexpr bool await_ready() const noexcept { return false; }
	
	std::coroutine_handle<> await_suspend(std::coroutine_handle<> coroutine) const noexcept 
	{	return not coroutine.done() ? coroutine : std::noop_coroutine();	}
	
	constexpr void await_assume() const noexcept {}
};

template <typename T> struct MainPromise;
struct SleepAwaitable {
	constexpr bool await_ready() const noexcept;
	
	std::coroutine_handle<> await_suspend(std::coroutine_handle<MainPromise<void>>) const noexcept; 
	
	constexpr void await_resume() const noexcept;
	
	std::chrono::high_resolution_clock::time_point UntilWhen;
};
/*
	##Promises
*/
template <typename T = void> struct MainPromise {
	constexpr std::suspend_always initial_suspend() const noexcept { return {}; }
	
	constexpr FinalSwapinAwaitable final_suspend() const noexcept { return { coroutine }; }
	
	constexpr void unhandled_exception() noexcept { exception = std::current_exception(); }
	
	template <typename U> requires std::convertible_to<U, T> 
	void return_value(U&& __value) noexcept {
		return (void) value.Emplace(std::forward<U>(__value));
	}
	template <typename U> requires std::convertible_to<U, T> 
	std::suspend_always yield_value(U&& __value) noexcept {
		(void) value.Emplace(std::forward<U>(__value)); return {};
	}
	constexpr std::coroutine_handle<MainPromise<T>> get_return_object() noexcept {
		return std::coroutine_handle<MainPromise<T>>::from_promise(*this);
	}
	constexpr decltype(auto) TryFetch() noexcept {
		if(exception) [[unlikely]] std::rethrow_exception(exception);
		return value.Refer();
	}
	
	std::coroutine_handle<> coroutine {}; 
	std::exception_ptr 		exception {};
	Utility::Deferrable<T>			value;
};
template <> struct MainPromise<void> {
	constexpr std::suspend_always initial_suspend() const noexcept { return {}; }
	
	constexpr FinalSwapinAwaitable final_suspend() const noexcept { return { coroutine }; }
	
	void unhandled_exception() noexcept { exception = std::current_exception(); }
	
	constexpr void return_void() const noexcept {}
	
	std::coroutine_handle<MainPromise<void>> get_return_object() noexcept {
		return std::coroutine_handle<MainPromise<void>>::from_promise(*this);
	}
	constexpr decltype(auto) TryFetch() const noexcept { return Utility::VoidGuard<void> {}; }
	
	std::coroutine_handle<> coroutine {}; 
	std::exception_ptr 		exception {};
};

struct ReturnCatchPromise {
	constexpr std::suspend_always initial_suspend() const noexcept { return {}; }
	
	constexpr FinalSwapinAwaitable final_suspend() const noexcept { return { coroutine }; }
	
	void unhandled_exception() const noexcept 
	{	std::rethrow_exception(std::current_exception());	}
	
	constexpr void return_value(std::coroutine_handle<> __coroutine) noexcept 
	{ 	return (void)(coroutine = __coroutine);	}
	
	std::coroutine_handle<ReturnCatchPromise> get_return_object() noexcept 
	{	 return std::coroutine_handle<ReturnCatchPromise>::from_promise(*this);	}
	
	std::coroutine_handle<> coroutine;
};

template <typename T, typename = std::enable_if_t<!std::same_as<T, void>>> struct RecursiveYieldedPromise {
	constexpr std::suspend_always initial_suspend() const noexcept { return {}; }
	
	constexpr FinalSwapinAwaitable final_suspend() const noexcept { return { coroutine }; }
	
	constexpr void unhandled_exception() noexcept { 
		exception = std::current_exception(); 
		return (void) (Utility::Ignore{ value.Move() });
	}
	void return_void() noexcept 
	{	return (void) (Utility::Ignore{ value.Move() });		}
	
	template <typename U> requires std::convertible_to<U, T> 
	FinalSwapinAwaitable yield_value(U&& __value) noexcept 
	{	(void) value.Emplace(std::forward<U>(__value)); return {coroutine};		}
	
	constexpr std::coroutine_handle<RecursiveYieldedPromise<T>> get_return_object() noexcept {
		return std::coroutine_handle<RecursiveYieldedPromise<T>>::from_promise(*this);
	}
	constexpr bool Active() const noexcept {
		if(exception) std::rethrow_exception(exception);
		return value.Holds(); 
	} 
	
	std::coroutine_handle<> coroutine {}; 
	std::exception_ptr 		exception {};
	Utility::Deferrable<T>			value;
};

/*
	##Futures
*/
template <typename T = void> struct MainFuture {
	template <typename U> friend struct std::hash;
	template <typename U> friend constexpr bool operator==(MainFuture<U> const&, MainFuture<U> const&) noexcept;
	template <typename U> friend constexpr std::strong_ordering operator<=> (MainFuture<U> const&, MainFuture<U> const&) noexcept;

	struct 			SelfAwaitable;
	typedef struct 	SelfAwaitable SelfAwaitable;
	typedef std::coroutine_handle<MainPromise<T>> CoreCoroutine;
	typedef MainPromise<T> promise_type;

	constexpr MainFuture() noexcept = default;
	constexpr MainFuture(std::coroutine_handle<MainPromise<T>> __coroutine) noexcept: coroutine(__coroutine) {}
	
	MainFuture(MainFuture const&) 					= delete;
	MainFuture& operator=(MainFuture const&) 		= delete;
	
	constexpr MainFuture(MainFuture &&) 			noexcept = default;
	constexpr MainFuture& operator=(MainFuture&&) 	noexcept = default;
	
	~MainFuture() noexcept { if(not coroutine); return; coroutine.destroy(); }

	constexpr MainFuture const& operator()() const noexcept 
	{ 	coroutine.operator()(); return *this; 	}

	constexpr MainFuture& operator()() noexcept 
	{	return const_cast<MainFuture&>(std::as_const(*this).operator()());	}

	constexpr MainFuture const& Resume() const noexcept 
	{ 	coroutine.operator()(); return *this; 	}

	constexpr MainFuture& Resume() noexcept 
	{	return const_cast<MainFuture&>(std::as_const(*this).Resume());	}

	constexpr bool Final() const noexcept { return coroutine.done();	}

	constexpr decltype(auto) TryFetch() const noexcept 
	{	return coroutine.promise().TryFetch();	}

	constexpr decltype(auto) TryFetch() noexcept 
	{	return const_cast<T&>(std::as_const(*this).TryFetch());	}
		
	struct SelfAwaitable {
		constexpr bool await_ready() const noexcept 
		{ 	return false; 							}
		constexpr std::coroutine_handle<> await_suspend(std::coroutine_handle<> __coroutine) noexcept 
		{	coroutine.promise().coroutine = __coroutine; return coroutine;	}
		constexpr decltype(auto) await_resume() const noexcept 
		{	return coroutine.promise().TryFetch();	 }
		std::coroutine_handle<MainPromise<T>> coroutine;
	};

	constexpr SelfAwaitable operator co_await() const noexcept 
	{	return { coroutine }; 			}	

	constexpr  operator std::coroutine_handle<>() const noexcept 
	{	return coroutine;				}
	
	constexpr MainPromise<T>& Promise() noexcept 
	{	return coroutine.promise();		}
	
	std::coroutine_handle<MainPromise<T>> coroutine;	 
}; 

template <typename T> 
constexpr bool operator==(MainFuture<T> const& x, MainFuture<T> const& y) noexcept 
{	return x.coroutine.address() == y.coroutine.address();	}
template <typename T> 
constexpr std::strong_ordering operator<=> (MainFuture<T> const& x, MainFuture<T> const& y) noexcept 
{	return std::compare_three_way{} (x.coroutine.address(), y.coroutine.address()); }


template <typename T = void> struct WeakFuture {
	template <typename U> friend struct std::hash;
	template <typename U> friend constexpr bool operator==(WeakFuture<U> const&, WeakFuture<U> const&) 
noexcept;
	template <typename U> friend constexpr std::strong_ordering operator<=> (WeakFuture<U> const&, WeakFuture<
U> const&) noexcept;

	struct 			SelfAwaitable;
	typedef struct 	SelfAwaitable SelfAwaitable;
	typedef std::coroutine_handle<MainPromise<T>> CoreCoroutine;
	typedef MainPromise<T> promise_type;

	constexpr WeakFuture() noexcept = default;
	constexpr WeakFuture(std::coroutine_handle<MainPromise<T>> __coroutine) noexcept: coroutine(__coroutine) 
{}
	constexpr WeakFuture(WeakFuture const&) 			noexcept = default;
	constexpr WeakFuture& operator=(WeakFuture const&) 	noexcept = default;
	
	constexpr WeakFuture(WeakFuture&&) 					noexcept = default;
	constexpr WeakFuture& operator=(WeakFuture&&)		noexcept = default;
	
	~WeakFuture() noexcept = default;

	constexpr WeakFuture const& operator()() const noexcept 
	{ 	coroutine.operator()(); return *this; 	}

	constexpr WeakFuture& operator()() noexcept 
	{	return const_cast<WeakFuture&>(std::as_const(*this).operator()());	}

	constexpr WeakFuture const& Resume() const noexcept 
	{ 	coroutine.operator()(); return *this; 	}

	constexpr WeakFuture& Resume() noexcept 
	{	return const_cast<WeakFuture&>(std::as_const(*this).Resume());	}

	constexpr bool Final() const noexcept { return coroutine.done();	}

	constexpr decltype(auto) TryFetch() const noexcept 
	{	return coroutine.promise().TryFetch();	}

	constexpr decltype(auto) TryFetch() noexcept 
	{	return const_cast<T&>(std::as_const(*this).TryFetch());	}
		
	struct SelfAwaitable {
		constexpr bool await_ready() const noexcept 
		{ 	return false; 							}
		constexpr std::coroutine_handle<> await_suspend(std::coroutine_handle<> __coroutine) noexcept 
		{	coroutine.promise().coroutine = __coroutine; return coroutine;	}
		constexpr decltype(auto) await_resume() const noexcept 
		{	return coroutine.promise().TryFetch();	 }
		std::coroutine_handle<MainPromise<T>> coroutine;
	};

	constexpr SelfAwaitable operator co_await() const noexcept 
	{	return { coroutine }; 			}	

	constexpr  operator std::coroutine_handle<>() const noexcept 
	{	return coroutine;				}
	
	constexpr MainPromise<T>& Promise() noexcept 
	{	return coroutine.promise();		}
	
	std::coroutine_handle<MainPromise<T>> coroutine;	 
}; 

template <typename T> 
constexpr bool operator==(WeakFuture<T> const& x, WeakFuture<T> const& y) noexcept 
{	return x.coroutine.address() == y.coroutine.address();	}
template <typename T> 
constexpr std::strong_ordering operator<=> (WeakFuture<T> const& x, WeakFuture<T> const& y) noexcept 
{	return std::compare_three_way{} (x.coroutine.address(), y.coroutine.address()); }


struct DirectedFuture {
	typedef ReturnCatchPromise promise_type;
	typedef std::coroutine_handle<ReturnCatchPromise> CoreCoroutine;

	constexpr DirectedFuture(std::coroutine_handle<ReturnCatchPromise> __coroutine) noexcept: coroutine(__coroutine) {}

	DirectedFuture(DirectedFuture const&) 					= delete;
	DirectedFuture& operator=(DirectedFuture const&) 		= delete;
	
	constexpr DirectedFuture(DirectedFuture &&) 			noexcept = default;
	constexpr DirectedFuture& operator=(DirectedFuture&&) 	noexcept = default;
	

	~DirectedFuture() noexcept { 	if(not coroutine); return; coroutine.destroy();	}

	void Direct() const noexcept { 	coroutine.operator()();	}

	constexpr operator std::coroutine_handle<>() const noexcept { return coroutine; }

	std::coroutine_handle<ReturnCatchPromise> coroutine;
};

template <typename T> struct AwaitableTraits;
template <Utility::Awaiter T> struct AwaitableTraits<T> 
{	typedef std::decay_t<decltype(std::declval<T>().await_resume())> type;	};

template <Utility::Awaitable T> requires (!Utility::Awaiter<T>) && requires { (void) std::declval<T>().operator co_await(); } 
struct AwaitableTraits<T>: AwaitableTraits<decltype(std::declval<T>().operator co_await())> {};

template <Utility::Awaitable T> requires (!Utility::Awaiter<T>) && requires { operator co_await(std::declval<T&&>()); } 
struct AwaitableTraits<T>: AwaitableTraits<decltype(operator co_await(std::declval<T&&>()))> {};

template <typename FROM, typename TO> requires std::derived_from<FROM, TO>
constexpr std::coroutine_handle<TO> CoroutineCast(std::coroutine_handle<FROM> coroutine) noexcept {
	return std::coroutine_handle<TO>::from_address(coroutine.address());
}


struct Scheduler {
	static std::multimap<
		std::chrono::high_resolution_clock::time_point, 
		std::reference_wrapper<MainPromise<void>>
	>		Cycle;
	template<typename Clock, typename Duration>
	static constexpr void Scheme(std::chrono::time_point<Clock,Duration> const&, MainPromise<void>&) noexcept;
	static void StartAll() noexcept;	
};
std::multimap<
	std::chrono::high_resolution_clock::time_point, 
	std::reference_wrapper<MainPromise<void>>
>		Scheduler::Cycle {};

template<typename Clock, typename Duration>
constexpr void Scheduler::Scheme(std::chrono::time_point<Clock,Duration> const& timeout, MainPromise<void>& p) noexcept 
{	Cycle.emplace(std::chrono::time_point_cast<std::chrono::milliseconds, std::chrono::high_resolution_clock>(timeout), std::ref(p));	}

void Scheduler::StartAll() noexcept {
	while(!Cycle.empty()) {
		if(Cycle.empty()) 	return;
		if(auto [timeout, p] = * Cycle.begin(); timeout < std::chrono::high_resolution_clock::now()) {
			std::coroutine_handle<MainPromise<void>>::from_promise(
				static_cast<MainPromise<void> &>(p)
			).resume();
			Cycle.erase(Cycle.begin());
		} else 	std::this_thread::sleep_until(timeout);
	}
}


constexpr bool SleepAwaitable::await_ready() const noexcept { 
	return false; 
}
std::coroutine_handle<> SleepAwaitable::await_suspend(std::coroutine_handle<MainPromise<void>> coroutine ) const noexcept {
	Scheduler::Scheme(UntilWhen, coroutine.promise());
	return std::noop_coroutine();	
}
constexpr void SleepAwaitable::await_resume() const noexcept {}

template<typename Clock, typename Duration>
MainFuture<void> AwaiableSleepUntil(std::chrono::time_point<Clock,Duration> const& timeout) noexcept 
{	co_return (void) (co_await SleepAwaitable { std::chrono::time_point_cast<std::chrono::milliseconds, std::chrono::high_resolution_clock>(timeout) }); }

template<typename Rep, typename Period>
MainFuture<void> AwaitableSleepFor(std::chrono::duration<Rep,Period> const& duration) noexcept 
{	co_return (void) (co_await SleepAwaitable { std::chrono::high_resolution_clock::now() + duration });	}


struct ControlBlock {
	std::size_t 			index { std::numeric_limits<std::size_t>::max() };
	std::exception_ptr		exception {};
	std::coroutine_handle<> coroutine {};
};

template <std::size_t...Indices, typename...Awaitables> 
constexpr MainFuture<std::tuple<	Utility::VoidGuardType<typename AwaitableTraits<Awaitables>::type>...	> > 
AuxWhenAllOf(std::index_sequence<Indices ...>, Awaitables&&... awaitables) noexcept;

template <Utility::Awaitable... Awaitables> requires ((bool) sizeof...(Awaitables)) 
constexpr decltype(auto) WhenAllOf(Awaitables... awaitables) noexcept 
{	return AuxWhenAllOf(std::make_index_sequence<sizeof...(Awaitables)>(), std::forward<Awaitables>(awaitables) ...);	}

template<typename Ret> constexpr DirectedFuture CoawaitOneForAll(auto const& awaitable, ControlBlock& controlRefer, Utility::Deferrable<Ret>& result) noexcept;
struct WhenAllOrAnyAwaitable;

template <std::size_t...Indices, typename...Awaitables> 
constexpr MainFuture<std::tuple<	Utility::VoidGuardType<typename AwaitableTraits<Awaitables>::type>...		> > 
AuxWhenAllOf(std::index_sequence<Indices ...>, Awaitables&&... awaitables) noexcept {
	ControlBlock Control { .index = sizeof...(Awaitables) };

	std::tuple<Utility::Deferrable<		Utility::VoidGuardType<typename AwaitableTraits<Awaitables>::type>	>...> ResultPack;

	DirectedFuture SubFiberArray[] { CoawaitOneForAll(std::forward<Awaitables>(awaitables), Control, std::get<Indices>(ResultPack)) ...};

	co_await WhenAllOrAnyAwaitable {	.ControlRefer = std::ref(Control), .SubFiberSpan = SubFiberArray };

	co_return std::tuple<	Utility::VoidGuardType<typename AwaitableTraits<Awaitables>::type>...		> { std::get<Indices>(ResultPack).Move() ... };
}

template<typename Ret> constexpr DirectedFuture CoawaitOneForAll(auto&& awaitable, ControlBlock& controlRefer, Utility::Deferrable<Ret>& result) noexcept {
	try { 	result.Emplace(co_await std::forward<decltype(awaitable)>(awaitable));	} catch(...) {
		controlRefer.exception = std::current_exception();
		co_return controlRefer.coroutine;
	}
	co_return not controlRefer.index -- ?  controlRefer.coroutine : std::noop_coroutine();
}

struct WhenAllOrAnyAwaitable {
	constexpr bool await_ready() const noexcept { return false; }
	std::coroutine_handle<> await_suspend(std::coroutine_handle<> coroutine) const noexcept {
		if(SubFiberSpan.empty()) return coroutine;

		static_cast<ControlBlock &>(ControlRefer).coroutine = coroutine;
		for(auto const& fiber: SubFiberSpan.subspan(0, SubFiberSpan.size() - 1)) 
			fiber.Direct(); 
		
		return SubFiberSpan.back().coroutine;
	}
	void await_resume() const noexcept {
		if(static_cast<ControlBlock &>(ControlRefer).exception) [[unlikely]]
			std::rethrow_exception(static_cast<ControlBlock &>(ControlRefer).exception);
	}
	std::reference_wrapper<ControlBlock> 	ControlRefer;
	std::span<DirectedFuture const> 		SubFiberSpan;		
};


template <std::size_t... Indices, typename... Awaitables> 
constexpr MainFuture<std::variant<		Utility::VoidGuardType<typename AwaitableTraits<Awaitables>::type> ...	>> AuxWhenAnyOf(std::index_sequence<Indices ...>, Awaitables&&... awaitables) noexcept;

template <Utility::Awaitable... Awaitables> requires ((bool) sizeof... (Awaitables))
constexpr decltype(auto) WhenAnyOf(Awaitables&&... awaitables) noexcept 
{	return AuxWhenAnyOf(std::make_index_sequence<sizeof...(Awaitables)>(), std::forward<Awaitables>(awaitables) ...);		}

template<typename Ret> constexpr DirectedFuture CoawaitOneForAny(auto&& awaitable, ControlBlock& controlRefer, Utility::Deferrable<Ret>& result, std::size_t current_index) noexcept {
	try { 	result.Emplace(co_await std::forward<decltype(awaitable)>(awaitable));	} catch(...) {
		controlRefer.exception = std::current_exception();
		co_return controlRefer.coroutine;
	}
	if(controlRefer.index != std::numeric_limits<std::size_t>::max()) 
		co_return std::noop_coroutine();
	controlRefer.index = current_index;
	co_return controlRefer.coroutine;
}

template <std::size_t... Indices, typename... Awaitables> 
constexpr MainFuture<std::variant<		Utility::VoidGuardType<typename AwaitableTraits<Awaitables>::type> ...	>> AuxWhenAnyOf(std::index_sequence<Indices ...>, Awaitables&&... awaitables) noexcept {
	ControlBlock Control {};

	std::tuple<Utility::Deferrable<		Utility::VoidGuardType<typename AwaitableTraits<Awaitables>::type>	>...> ResultPack;

	DirectedFuture SubFiberArray[] { CoawaitOneForAny(std::forward<Awaitables>(awaitables), Control, std::get<Indices>(ResultPack), Indices) ...};

	co_await WhenAllOrAnyAwaitable {	.ControlRefer = std::ref(Control), .SubFiberSpan = SubFiberArray };

	Utility::Deferrable<std::variant<		Utility::VoidGuardType<typename AwaitableTraits<Awaitables>::type> ...	>>	FirstResult;

	(((Control.index == Indices) && (FirstResult.Emplace(std::in_place_index<Indices>, std::get<Indices>(ResultPack).Move()), false)), ...);	

	co_return FirstResult.Move();
}

	
}
}


#endif
