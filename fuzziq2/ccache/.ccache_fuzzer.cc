#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include <stdexcept>
#include "/src/ccache/src/ccache/util/path.hpp"

namespace {
    // Prevent optimizer from discarding path processing
    volatile std::size_t g_sink = 0;

    void sink(const std::string& s) { g_sink ^= s.size(); }
    void sink(std::size_t n)        { g_sink ^= n; }
    void sink(bool b)               { g_sink ^= static_cast<std::size_t>(b); }

    // Safe wrapper to prevent fuzzer crashes on invalid paths
    template<typename Func, typename... Args>
    auto safe_call(Func&& f, Args&&... args) {
        try {
            return f(std::forward<Args>(args)...);
        } catch (...) {
            return std::string{};
        }
    }
} // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, std::size_t size)
{
    if (!data || size == 0) return 0;

    // Preserve full input including NUL bytes
    const std::string input(reinterpret_cast<const char*>(data), size);

    // -----------------------------------------------------------------------
    // Directly exercise ccache's path utility API (src/ccache/util/path.cpp)
    // -----------------------------------------------------------------------

    sink(safe_call([](auto& s){ return get_parent_path(s); }, input));
    sink(safe_call([](auto& s){ return get_filename(s); }, input));
    sink(safe_call([](auto& s){ return get_extension(s); }, input));
    sink(safe_call([](auto& s){ return get_stem(s); }, input));
    sink(safe_call([](auto& s){ return is_absolute(s); }, input));
    sink(safe_call([](auto& s){ return is_relative(s); }, input));

    // Exercise path joining logic (handles concatenation, normalization, separators)
    std::vector<std::string> parts{input, "component", "subdir/file.txt"};
    sink(safe_call([&parts]{ return join_paths(parts); }));

    // Canonicalization exercises symlink resolution & normalization paths
    sink(safe_call([](auto& s){ return canonicalize_path(s); }, input));

    g_sink ^= input.size();
    return 0;
}
