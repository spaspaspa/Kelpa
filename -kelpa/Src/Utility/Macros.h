/**
 * @Brief		Predefined Util Macros
 **/
 
#ifndef __KELPA_UTILITY_MACROS_H__
#define __KELPA_UTILITY_MACROS_H__

#include <memory> 
/* imports memory/{
	::std::shared_ptr,
	::std::weak_ptr,
	::std::unique_ptr
}*/
#include <functional>
/* imports memory/{
	::std::reference_wrapper
}*/
#include <concepts> /* imports concepts/type_traits/{
	::std::xxx_t,
	::std::is_xxx_v
}*/
#define __KELPA_DEFINES_STRUCT_MEMBER_TYPES__(STRUCT) \
	static_assert(!std::is_reference_v<STRUCT>, "!(bool)std::is_reference_v: "	#STRUCT		" == false");						\
	typedef std::decay_t<STRUCT>									value_type;					\
	typedef value_type const										const_value_type;			\
	typedef value_type& 											reference;					\
	typedef value_type const& 										const_reference;			\
	typedef value_type *											pointer;					\
	typedef value_type const *										const_pointer;				\
	typedef std::shared_ptr<value_type>								shared_pointer;				\
	typedef std::shared_ptr<value_type const>						const_shared_pointer;		\
	typedef std::unique_ptr<value_type>								unique_pointer;				\
	typedef std::unique_ptr<value_type const>						const_unique_pointer;		\
	typedef std::weak_ptr<value_type>								weak_pointer;				\
	typedef std::weak_ptr<value_type const>							const_weak_pointer;			\
	typedef std::reference_wrapper<value_type>						reference_wrapper;			\
	typedef std::reference_wrapper<value_type const>				const_reference_wrapper;
	
#define __KELPA_CONCAT_MACRO__(A, B) 			A ## B	
#define __KELPA_QUOTED_TO_STRING__(A)			#A	

#define SCOPE_BEGIN() 		{
#define SCOPE_END() 		}


#endif
