/** 
 * 		@Path 	Kelpa/Src/Thread/Sync/HashMap.hpp
 * 		@Brief	Thread-safe unordered_map holding a lock	
 * 		@Dependency	None
 * 		@Since  2024/04/25
 * 		@Version 1st
 **/
 
#ifndef __KELPA_THREAD_HASHMAP_HPP__
#define __KELPA_THREAD_HASHMAP_HPP__

#include <unordered_map>				/* imports ./ { 
	std::unordered_map, 
	std;:hash, 
	./ functional/ { 
		std::equal_to, 
		std::allocator 
	}
}*/
#include <condition_variable>			/* imports ./ { 
	std::condition_variable_any 
}*/
#include "../CASLock.hpp"				/* imports ./ { 
	struct CASLock 
}*/

namespace Kelpa {
namespace Thread {
namespace Sync {
	
template<
    typename Key,
    typename T,
    typename Hash = std::hash<Key>,
    typename KeyEqual = std::equal_to<Key>,
    typename Allocator = std::allocator<std::pair<const Key, T>>,
    typename Mutex = CASLock 
> struct HashMap {
	typedef typename std::unordered_map<Key, T, Hash, KeyEqual, Allocator>							container_type;
	typedef typename std::unordered_map<Key, T, Hash, KeyEqual, Allocator>::key_type 				key_type;
	typedef typename std::unordered_map<Key, T, Hash, KeyEqual, Allocator>::mapped_type 			mapped_type;
	typedef typename std::unordered_map<Key, T, Hash, KeyEqual, Allocator>::value_type 				value_type;
	typedef typename std::unordered_map<Key, T, Hash, KeyEqual, Allocator>::size_type 				size_type;
	typedef typename std::unordered_map<Key, T, Hash, KeyEqual, Allocator>::key_equal 				key_equal;
	typedef typename std::unordered_map<Key, T, Hash, KeyEqual, Allocator>::allocator_type 			allocator_type;
	typedef typename std::unordered_map<Key, T, Hash, KeyEqual, Allocator>::reference 				reference;
	typedef typename std::unordered_map<Key, T, Hash, KeyEqual, Allocator>::const_reference 		const_reference;
	typedef typename std::unordered_map<Key, T, Hash, KeyEqual, Allocator>::const_pointer 			const_pointer;
	typedef typename std::unordered_map<Key, T, Hash, KeyEqual, Allocator>::iterator 				iterator;
	typedef typename std::unordered_map<Key, T, Hash, KeyEqual, Allocator>::const_iterator 			const_iterator;
	typedef typename std::unordered_map<Key, T, Hash, KeyEqual, Allocator>::local_iterator 			local_iterator;
	typedef typename std::unordered_map<Key, T, Hash, KeyEqual, Allocator>::const_local_iterator 	const_local_iterator;
	typedef typename std::unordered_map<Key, T, Hash, KeyEqual, Allocator>::node_type 				node_type;
	/* Since c++ 17*/
	typedef typename std::unordered_map<Key, T, Hash, KeyEqual, Allocator>::insert_return_type 		insert_return_type;

	HashMap() noexcept = default;

	explicit HashMap( size_type bucket_count) noexcept
		:container_type(count) {} 
		
	HashMap( size_type bucket_count,
			 Allocator const& alloc ) noexcept
		: container_type(bucket_count, Hash(), key_equal(), alloc) {}

	HashMap( size_type bucket_count,
				   const Hash& hash,
				   const Allocator& alloc ) noexcept
		: container_type(bucket_count, hash, key_equal(), alloc) {}
	
	explicit HashMap( const Allocator& alloc ) noexcept
		: container_type(alloc) {};
	
	template<typename InputIt>
	HashMap( InputIt first, InputIt last) noexcept
		: container_type(first, last) {};
		
	template<typename InputIt>
	HashMap( InputIt first, InputIt last,
				   size_type bucket_count,
				   Allocator const& alloc) noexcept
		: container_type(first, last,
						bucket_count, Hash(), key_equal(), alloc) {}

	template<typename InputIt>
	HashMap( InputIt first, InputIt last,
				   size_type bucket_count,
				   Hash const& hash,
				   Allocator const& alloc ) noexcept
		: container_type(first, last,
						bucket_count, hash, key_equal(), alloc) {}

	HashMap( const unordered_map& other ) noexcept = default;
		
	HashMap( unordered_map const& other, Allocator const& alloc ) noexcept
		: container_type(other, alloc) {};
		
	HashMap( unordered_map&& other ) noexcept = default;
		
	HashMap( unordered_map&& other, Allocator const& alloc ) noexcept
		: container_type(other, alloc) {};
	
	unordered_map( std::initializer_list<value_type> init) noexcept;
		(13) 	(since C++11)
	unordered_map( std::initializer_list<value_type> init,

				   size_type bucket_count,
				   const Allocator& alloc )
		: unordered_map(init, bucket_count,
						Hash(), key_equal(), alloc) {}
		(14) 	(since C++14)
	unordered_map( std::initializer_list<value_type> init,

				   size_type bucket_count,
				   const Hash& hash,
				   const Allocator& alloc )
		: unordered_map(init, bucket_count,
						hash, key_equal(), alloc) {}

	
private:
	std::unordered_map<Key, T, Hash, KeyEqual, Allocator>		M;
	mutable Mutex 												mutex;
	mutable std::condition_variable_any 						condition;
}; 
	
}		//namespace Sync
}		//namespace Thread
}		//namespace Kelpa

#endif
