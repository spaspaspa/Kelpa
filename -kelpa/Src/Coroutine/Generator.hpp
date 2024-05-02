/** 
 * 		@Path 	Kelpa/Src/Coroutine/Generator.hpp
 * 		@Brief	presents a view of the elements yielded by the evaluation of a coroutine.
 * 		@Dependency	./ {Coroutine.hpp}
 * 		@Since  2024/04/25
 * 		@Version 1st
 **/

#ifndef __KELPA_COROUTINE_GENERATOR_HPP__
#define __KELPA_COROUTINE_GENERATOR_HPP__
#include <ranges>						/* imports ./ { 
	std::view_interface 
	./concept/type_tratis/ { 
		std::is_void_v 
	}
	std::default_sentinel_t 
}*/
#include <coroutine>					/* imports ./ { 
	std::coroutine_handle, 
	std::noop_coroutine_handle, 
	std::suspend_xxx 
}*/
#include <utility>						/* imports ./ { 
	std::exchange 
}*/
namespace Kelpa {
namespace Coroutine {
	
template <typename Ref> requires std::negation_v<std::is_void<Ref>>
struct Generator :public std::ranges::view_interface<Generator<Ref>> {
struct promise_type {
	typedef std::remove_cvref_t<Ref> 			value_type;
	typedef std::add_lvalue_reference_t<Ref> 	reference;
	typedef std::add_pointer_t<Ref>       		pointer;
	
	constexpr decltype(auto) get_return_object() noexcept { 
		return std::coroutine_handle<promise_type>::from_promise(*this); 
	}
    constexpr std::suspend_always initial_suspend() const noexcept { 
		return {}; 
	}
    constexpr decltype(auto) final_suspend() const noexcept {
      	struct FinalAwaitable 		{
        	constexpr bool await_ready() const noexcept { 
				return false; 
			}
	        constexpr std::coroutine_handle<> await_suspend(std::coroutine_handle<promise_type> handle) noexcept {
	          	auto 		parent 	= handle.promise().parent_;
	          	if (parent) [[unlikely]] {
	            	(* handle.promise().root_).leaf_ = parent;
	            	return std::coroutine_handle<promise_type>::from_promise(*parent);
	          	} 
	          	return 		std::noop_coroutine();
	        }
	        constexpr void await_resume() const noexcept {}
      	};
      	
		return FinalAwaitable{};
    }
    constexpr void unhandled_exception() const noexcept { 
		std::rethrow_exception(std::current_exception()); 
	}
	template <typename U> requires std::convertible_to<U, value_type>
    constexpr std::suspend_always yield_value(U&& value) noexcept {
      	(* root_).pointer_ = std::addressof(std::forward<U>(value));
      	return {};
    }
    template <typename Gen> constexpr decltype(auto) yield_value(Gen&& g) noexcept {
      	struct YieldAwaitable 	{
        	constexpr YieldAwaitable(Gen&& g) noexcept : gen_(std::forward<Gen>(g)) {}

        	constexpr bool await_ready() const noexcept { 
				return !gen_.coroutine_.operator bool(); 
			}
        	std::coroutine_handle<> await_suspend(std::coroutine_handle<promise_type> handle) noexcept {
	          	auto& current 	= handle.promise();
	          	auto& root 		= current.root_;
	          	auto& nested 	= gen_.coroutine_.promise();
	          // merge g to h
	          	nested.parent_ 	= std::addressof(current);
	          	nested.root_ 	= root;
	          	root->leaf_ 	= std::addressof(nested);

          		return gen_.coroutine_;
        	}
        	constexpr void await_resume() const noexcept {}

        	Generator gen_;
    	};
      	
		return YieldAwaitable(std::forward<Gen>(g));
    }
    constexpr void return_void() const noexcept {}

    constexpr reference value() noexcept { 
		return *root_->pointer_; 
	}

    constexpr void resume() noexcept { 
		std::coroutine_handle<promise_type>::from_promise(*leaf_).resume(); 
	}

	private:
	    promise_type* 		parent_ { nullptr };
	    promise_type* 		root_ 	{this};
	    promise_type* 		leaf_ 	{this};
	
	    pointer pointer_;
};
	typedef std::conditional_t<std::is_reference_v<Ref>, Ref, typename promise_type::reference> 	yielded;
	
	constexpr Generator(std::coroutine_handle<promise_type> handle) noexcept : coroutine_(handle) {}
	~Generator() noexcept {
		if(coroutine_)
			coroutine_.destroy();
	}
	constexpr Generator& operator=(Generator other) noexcept {
		std::swap(coroutine_, other.coroutine_);
		std::swap(active_, other.active_);
		return *this;
	}

  	struct iterator {
	    typedef std::input_iterator_tag 							iterator_category;
	    typedef std::ptrdiff_t 										difference_type;
		typedef std::remove_cvref_t<Ref> 							value_type;
		typedef std::add_lvalue_reference_t<Ref> 					reference;
		typedef std::add_const_t<std::add_lvalue_reference_t<Ref>> 	const_reference;
		typedef std::add_pointer_t<Ref>       						pointer;
		typedef std::add_const_t<std::add_pointer_t<Ref>>       	const_pointer;
	
	    constexpr bool operator!=( std::default_sentinel_t) const noexcept { return !coroutine_.done(); }
	    constexpr bool operator==( std::default_sentinel_t) const noexcept { return coroutine_.done(); }
	    constexpr iterator(std::coroutine_handle<promise_type> h) noexcept: coroutine_(h) {}
		
		constexpr iterator(iterator&& other) noexcept: coroutine_(std::exchange(other.coroutine_, {})) {}
	    constexpr iterator(iterator const& other) noexcept: coroutine_(other.coroutine_) {}
		
		constexpr iterator& operator=(iterator const& other) noexcept {
			coroutine_ = other.coroutine_;
			return *this;
		}
	    constexpr iterator& operator=(iterator&& other) noexcept {
			coroutine_ = std::exchange(other.coroutine_, {});
			return *this;
		}
		
	    
		constexpr void operator++(int) noexcept {
			return *this ++;
		};
		constexpr iterator& operator++() noexcept { 
			coroutine_.promise().resume(); 
			return *this;
		}
	    reference operator*()  noexcept( std::is_nothrow_copy_constructible_v<reference> ) { 
			return coroutine_.promise().value(); 
		}
	
	    std::coroutine_handle<promise_type> coroutine_;
  };

  	decltype(auto) end() const noexcept { 
	  	return  std::default_sentinel_t(); 
	}
  	decltype(auto) begin() noexcept 	{
	    auto 	it = iterator{coroutine_};
	    if (not active_ ) [[unlikely]]{
	      ++ it;
	      active_  = true;
	    }
	    return 	it;
 	}

private:
 	std::coroutine_handle<promise_type> 	coroutine_ {};
  	bool 									active_  	= false;
};
	
}
}


#endif
