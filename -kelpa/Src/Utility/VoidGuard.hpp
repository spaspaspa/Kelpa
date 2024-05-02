/** 
 * 		@Path	Kelpa/Src/Utility/VoidGuard.hpp
 * 		@Brief	Filter the type so that the non-void type is left as it is 
 * 				and the void type is converted to the some non-void type
 * 		@Dependency	None
 * 		@Since  2024/04/23
 * 		@Version 1st
 **/
 
#ifndef __KELPA_UTILITY_VOIDGUARD_HPP__
#define __KELPA_UTILITY_VOIDGUARD_HPP__

#include <concepts>						/* imports ./ { 
	std::same_as
	./type_traits/ { std::conditional_t  }
}*/
#include <tuple>						/* imports ./ { 
	std::ignore 
}*/
namespace Kelpa {
namespace Utility {
	
template <typename T> 	struct VoidGuard 					{ typedef T 						type; 	};
template <> 			struct VoidGuard<void>	 			{ typedef struct VoidGuard<void> 	type; 	};

template <typename T>	struct GuardedType;
template <typename T> 	struct GuardedType<VoidGuard<T>> 	{ typedef T 						type;	};


template <typename T> using VoidGuardType = std::conditional_t<std::same_as<T, void>, std::decay_t<decltype(std::ignore)>, T>;

}
}


#endif
