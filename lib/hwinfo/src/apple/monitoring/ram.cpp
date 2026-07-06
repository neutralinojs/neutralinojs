#include "hwinfo/platform.h"

#ifdef HWINFO_APPLE

#include "hwinfo/monitoring/ram.h"

namespace hwinfo::monitoring::ram {

Data fetch() { return {}; }
uint64_t free_bytes() { return 0; }
uint64_t available_bytes() { return 0; }

}  // namespace hwinfo::monitoring::ram

#endif  // HWINFO_APPLE
