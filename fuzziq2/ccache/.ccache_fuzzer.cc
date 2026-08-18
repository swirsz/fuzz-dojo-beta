#include <stdint.h>
#include <stddef.h>
#include "ccache.h" // Adjust include path as needed

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Parse/cache-lookup logic that doesn't modify global state
    // Example: ccache_parse_cache_entry(data, size);
    return 0;
}
