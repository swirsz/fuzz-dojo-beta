#include <cstddef>
#include <cstdint>
#include <string>
#include <optional>

// Matches ccache's src/ccache/util/path.hpp via the -I/src/ccache/src flag in .build.sh
#include "/src/ccache/src/ccache/util/path.hpp"

namespace {
    // Prevent optimizer from discarding path processing work
    volatile std::size_t g_sink = 0;

    void consume(const std::string& s) { g_sink ^= s.size(); }
    void consume(std::size_t n)        { g_sink ^= n; }
    void consume(bool b)               { g_sink ^= static_cast<std::size_t>(b); }
} // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, std::size_t size)
{
    if (data == nullptr || size == 0) {
        return 0;
    }

    // Preserve the complete fuzz input, including embedded NUL bytes.
    const std::string input(reinterpret_cast<const char*>(data), size);

    // -----------------------------------------------------------------------
    // Directly exercise the public API from src/ccache/util/path.cpp
    // -----------------------------------------------------------------------

    // 1. Tilde expansion: handles ~, ~/file, ~user, ~user/file patterns
    // Triggers internal string parsing and HOME/userdb lookups (userdb safely fails)
    auto expanded = expand_tilde(input);
    if (expanded) consume(*expanded);

    // 2. Path component counting: /, /a, a/b/c, trailing slashes, etc.
    const size_t n_components = count_path_components(input);
    consume(n_components);

    // 3. Realpath resolution: normalizes path components and resolves symlinks
    // Returns std::nullopt on non-existent files, keeping the fuzzer stable
    auto resolved = realpath_str(input);
    if (resolved) consume(*resolved);

    // 4. Additional path utilities (add if exposed in your ccache version)
    // Example: auto joined = path_join(input, input); if (joined) consume(*joined);

    // Sink to ensure the compiler cannot trivially optimize away the calls
    g_sink ^= input.size();
    return 0;
}
