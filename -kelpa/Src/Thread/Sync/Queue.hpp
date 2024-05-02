/** 
 * 		@Path 	Kelpa/Src/Thread/Sync/Queue.hpp
 * 		@Brief	Thread-safe queue holding a lock	
 * 		@Dependency	None
 * 		@Since  2024/04/25
 * 		@Version 1st
 **/
 
#ifndef __KELPA_THREAD_SYNCQUEUE_HPP__
#define __KELPA_THREAD_SYNCQUEUE_HPP__

#include <mutex>					/* imports ./ { 
	std::mutex,
	std::lock_guard,
	std::unique_lock,
	./concepts { 
		std::moveable, 
		std::swappable, 
		std::constructible_from, ... 
	}
}*/
#include "../SpinLock.hpp"
#include <condition_variable>		/* imports ./ { 
	std::condition_variable_any 
}*/
#include <queue>					/* imports ./ { 
	std::queue, 
	./deque { std::deque }
	std::priority_queue,
	./iterator { std::iterator_traits }
}*/
#include <list>						/* imports ./ { 
	std::list 
}*/
#include <algorithm>				/* imports ./ { 
	std::swap 
}*/
#include <utility>					/* imports ./ { 
	std::as_const 
}*/
namespace Kelpa {
namespace Thread {
	
template <typename T, typename Mutex = Thread::SpinLock> struct Queue {
	using value_type 		= T;
	using container_type 	= std::queue<T>;
	using size_type 		= typename std::queue<T>::size_type;
	using reference 		= typename std::queue<T>::reference;
	using const_reference 	= typename std::queue<T>::const_reference;

	Queue(Queue&&)  				= delete;
	Queue& operator=(Queue&&)  		= delete;
	Queue(Queue const&)  			= delete;
	Queue& operator=(Queue const&)  = delete;

	Queue() 	noexcept 				= default;
	~Queue() 	noexcept				= default;
	template <typename Container> 	requires std::constructible_from<container_type, Container> 
	explicit Queue(Container&& cont) noexcept: Q(std::forward<Container>(cont)) {}
/*
	Since C++	23
	template <typename InputIt> 	requires std::input_iterator<InputIt>
	Queue(InputIt first, InputIt last) noexcept: Q(first, last) {}	
*/
	void swap(Queue& other) noexcept requires std::swappable<value_type> {
		std::lock_guard self_guard 	{ mutex };
		std::lock_guard other_guard { other.mutex };
		std::swap(Q, other.Q);
	}
	template <typename U> requires std::convertible_to<U, T>
	void push(U&& value) noexcept {
	{	
		std::lock_guard 	guard { mutex }; 
		Q.push(std::forward<U>(value));
	}
		condition.notify_one();
	}	
	void pop() noexcept {
		std::unique_lock 	lock { mutex };
		condition.wait(lock, [this]{ return !Q.empty(); });
		Q.pop();
	}
	value_type get() noexcept {
		value_type 			value {};
		std::unique_lock 	lock { mutex };
		condition.wait(lock, [this]{ return !Q.empty(); });
		if constexpr (requires { requires std::movable<value_type>; }) 
				value = 	std::move(Q.front());
		else  	value = 	Q.front();
		Q.pop();
		return value;
	}
	const_reference front() const noexcept {
		std::unique_lock 	lock { mutex };
		condition.wait(lock, [this]{ return !Q.empty(); });
		return Q.front();
	}
	decltype(auto) front() noexcept 
	{		return const_cast<reference>(std::as_const(Q).front());		}
	const_reference back() const {
		std::unique_lock 	lock { mutex };
		condition.wait(lock, [this]{ return !Q.empty(); });
		return Q.back();
	}
	decltype(auto) back() noexcept 
	{		return const_cast<reference>(std::as_const(Q).back());		}
	
	template <typename... Args> requires std::constructible_from<value_type, Args ...>
	decltype(auto) emplace(Args&&... args) noexcept {
		std::lock_guard 	guard { mutex };
		return Q.emplace(std::forward<Args>(args) ...);
	}
	size_type size() const noexcept {
		std::lock_guard 	guard { const_cast<Queue<T, Mutex> &>(*this).mutex };
		return Q.size();
	}
	bool empty() const noexcept {
		std::lock_guard 	guard { const_cast<Queue<T, Mutex> &>(*this).mutex };
		return Q.empty();
	}
private:	
	mutable Mutex 								mutex {};
	mutable std::condition_variable_any 		condition {};
	std::queue<T> 								Q {};
};
template <typename Container> Queue(Container&&) 
	-> Queue<typename std::decay_t<Container>::value_type, Thread::SpinLock>;
/*
Since C++ 23
template <typename InputIt> Queue(InputIt, InputIt)
	-> Queue<typename std::iterator_traits<InputIt>::value_type, Thread::SpinLock>;	
*/
template <typename T, typename Mutex> void swap(Queue<T, Mutex> & one, Queue<T, Mutex> & two) noexcept 
{		one.swap(two);		}
	
}
}


#endif
