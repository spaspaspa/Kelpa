/** 
 * @Path		Kelpa/Src/Serde/FunctionTraits.hpp 
 * @Brief		The characteristics of callable object such as 
 * 				parameter type and return value type 
 * 				are analyzed by type extraction and template spfinea technique 
 * @Dependency	../Utility/ { Ignore.hpp }
 * @Since		2024/04/22
 * @Version     1st
 **/
 
#ifndef __KELPA_SERDE_FUNCTIONTRAITS_HPP__
#define __KELPA_SERDE_FUNCTIONTRAITS_HPP__
#include <functional>				/* imports function/ { 
	std::function, 
	std::tuple 
}*/
#include "../Utility/Ignore.hpp"	/* imports ./ { 
	struct Ignore 
}*/
namespace Kelpa {
namespace Serde {
	
/*	UnitType {};	*/
typedef Utility::Ignore	UnitType;

/*base*/
template <typename F> struct FunctionTraits;

/*reference specifier purify*/
template <typename T> struct FunctionTraits<T &> : FunctionTraits<T> {};
template <typename T> struct FunctionTraits<T const &> : FunctionTraits<T> {};
template <typename T> struct FunctionTraits<T volatile &> : FunctionTraits<T> {};
template <typename T> struct FunctionTraits<T const volatile &> : FunctionTraits<T> {};
template <typename T> struct FunctionTraits < T && > : FunctionTraits<T> {};
template <typename T> struct FunctionTraits < T const && > : FunctionTraits<T> {};
template <typename T> struct FunctionTraits < T volatile && > : FunctionTraits<T> {};
template <typename T> struct FunctionTraits < T const volatile && > : FunctionTraits<T> {};

/*normal function trait*/
template <typename R, typename... Args>
struct FunctionTraits<R(Args ...)> {
	static constexpr std::size_t arity { sizeof...(Args) };
	using result_type = 			R;
	using function_type = 			R(Args ...);
	using wrapper_type = 			std::function<R(Args ...)>;
	using function_pointer_type = 	R(*)(Args ...);
	using args_tuple_type = 		std::tuple<Args ...>;
	template <std::size_t I> struct args {
		static_assert(I < sizeof...(Args));
		using type = 				typename std::tuple_element<I, std::tuple<Args ...>>::type;
	};
	template <std::size_t I>
	using arg_type = 				typename args<I>::type;
};

/*noexcept function trait*/
template <typename R, typename... Args>
struct FunctionTraits<R(Args ...) noexcept> : FunctionTraits<R(Args ...)> {
	static constexpr UnitType is_noexcept {};
};

/*std::function poly-function wrapper purify*/
template <typename F> struct FunctionTraits<std::function<F>> : FunctionTraits<F> {};
template <typename F> struct FunctionTraits<const std::function<F>> : FunctionTraits<F> {};
template <typename F> struct FunctionTraits<volatile std::function<F>> : FunctionTraits<F> {};
template <typename F> struct FunctionTraits<const volatile std::function<F>> : FunctionTraits<F> {};

/*regular function pointer purify*/
template <typename R, typename... Args>
struct FunctionTraits<R(*)(Args ...)>: FunctionTraits<R(Args ...)> {};
template <typename R, typename... Args>
struct FunctionTraits<R(* const)(Args ...)>: FunctionTraits<R(Args ...)> {};
template <typename R, typename... Args>
struct FunctionTraits<R(* volatile)(Args ...)>: FunctionTraits<R(Args ...)> {};
template <typename R, typename... Args>
struct FunctionTraits<R(* const volatile)(Args ...)>: FunctionTraits<R(Args ...)> {};

/*reference_wrapper function reference purify*/
template <typename R, typename... Args>
struct FunctionTraits<std::reference_wrapper<R(Args ...)>>: FunctionTraits<R(Args ...)> {};
template <typename R, typename... Args>
struct FunctionTraits<std::reference_wrapper<R(Args ...)> const>: FunctionTraits<R(Args ...)> {};
template <typename R, typename... Args>
struct FunctionTraits<std::reference_wrapper<R(Args ...)> volatile>: FunctionTraits<R(Args ...)> {};
template <typename R, typename... Args>
struct FunctionTraits<std::reference_wrapper<R(Args ...)> const volatile>: FunctionTraits<R(Args ...)> {};

/*member function pointer purify*/
template <typename R, typename Class, typename... Args>
struct FunctionTraits<R(Class::*)(Args ...)> : FunctionTraits<R(Args ...)> {
	static constexpr UnitType 	is_member {};
	using class_type = 					Class;
	using function_pointer_type = 		R(Class::*)(Args ...);
};

/*undecayed member function pointer specifier purify*/
#define __KELPA_UNDECAYED_MEMBER_FUNCTION_POINTER__(SPECIFIER)\
	template <typename R, typename Class, typename... Args>													\
	struct FunctionTraits<R(Class::* SPECIFIER)(Args ...) const> : FunctionTraits<R(Class::*)(Args ...)> {	\
		static constexpr UnitType 	is_const {};													\
		using function_pointer_type = 		R(Class::*)(Args ...) const;									\
	};																										\
	template <typename R, typename Class, typename... Args>\
	struct FunctionTraits<R(Class::* SPECIFIER)(Args ...) volatile> : FunctionTraits<R(Class::*)(Args ...)> {\
		static constexpr UnitType 	is_volatile {};													\
		using function_pointer_type = 		R(Class::*)(Args ...) volatile;									\
	};																										\
	template <typename R, typename Class, typename... Args>													\
	struct FunctionTraits<R(Class::* SPECIFIER)(Args ...) noexcept> : FunctionTraits<R(Class::*)(Args ...)> {\
		static constexpr UnitType 	is_noexcept {};													\
		using function_pointer_type = 		R(Class::*)(Args ...) noexcept;									\
	};																										\
	template <typename R, typename Class, typename... Args>													\
	struct FunctionTraits<R(Class::* SPECIFIER)(Args ...) const volatile> : FunctionTraits<R(Class::*)(Args ...)> {\
		static constexpr UnitType 	is_const {};													\
		static constexpr UnitType 	is_volatile {};													\
		using function_pointer_type = 		R(Class::*)(Args ...) const volatile;							\
	};																										\
	template <typename R, typename Class, typename... Args>													\
	struct FunctionTraits<R(Class::* SPECIFIER)(Args ...) const noexcept> : FunctionTraits<R(Class::*)(Args ...)> {\
		static constexpr UnitType 	is_const {};													\
		static constexpr UnitType 	is_noexcept {};													\
		using function_pointer_type = 		R(Class::*)(Args ...) const noexcept;							\
	};																										\
	template <typename R, typename Class, typename... Args>													\
	struct FunctionTraits<R(Class::* SPECIFIER)(Args ...) volatile noexcept> : FunctionTraits<R(Class::*)(Args ...)> {\
		static constexpr UnitType 	is_volatile {};													\
		static constexpr UnitType 	is_noexcept {};													\
		using function_pointer_type = 		R(Class::*)(Args ...) volatile noexcept;						\
	};																										\
	template <typename R, typename Class, typename... Args>													\
	struct FunctionTraits<R(Class::* SPECIFIER)(Args ...) const volatile noexcept> : FunctionTraits<R(Class::*)(Args ...)> {\
		static constexpr UnitType 	is_const {};													\
		static constexpr UnitType 	is_volatile {};													\
		static constexpr UnitType 	is_noexcept {};													\
		using function_pointer_type = 		R(Class::*)(Args ...) const volatile noexcept;					\
	};

__KELPA_UNDECAYED_MEMBER_FUNCTION_POINTER__()
__KELPA_UNDECAYED_MEMBER_FUNCTION_POINTER__(const)
__KELPA_UNDECAYED_MEMBER_FUNCTION_POINTER__(volatile)
__KELPA_UNDECAYED_MEMBER_FUNCTION_POINTER__(const volatile)
#undef __KELPA_UNDECAYED_MEMBER_FUNCTION_POINTER__

/*functor with operator() overload purify*/
template <typename Functor>
struct FunctionTraits:  FunctionTraits<decltype(&Functor::operator())> {
	static constexpr UnitType is_functor {};
};

/*to-function helper method*/
template <typename F> auto to_function(F && lambda) noexcept 
{	return static_cast<typename FunctionTraits<F>::wrapper_type>(std::forward<F>(lambda));	}	
	
}
}

#endif
