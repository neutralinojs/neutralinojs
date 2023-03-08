#ifndef EFSW_ATOMIC_BOOL_HPP
#define EFSW_ATOMIC_BOOL_HPP

#include <efsw/base.hpp>

#ifndef EFSW_LEGACY_CPP
#include <atomic>
#endif

namespace efsw {

template <typename T> class Atomic {
  public:
	explicit Atomic( T set = false ) : set_( set ) {}

	Atomic& operator=( T set ) {
#ifndef EFSW_LEGACY_CPP
		set_.store( set, std::memory_order_release );
#else
		set_ = set;
#endif
		return *this;
	}

	explicit operator T() const {
#ifndef EFSW_LEGACY_CPP
		return set_.load( std::memory_order_acquire );
#else
		return set_;
#endif
	}

	T load() const {
#ifndef EFSW_LEGACY_CPP
		return set_.load( std::memory_order_acquire );
#else
		return set_;
#endif
	}

  private:
#ifndef EFSW_LEGACY_CPP
	std::atomic<T> set_;
#else
	volatile T set_;
#endif
};

} // namespace efsw

#endif
