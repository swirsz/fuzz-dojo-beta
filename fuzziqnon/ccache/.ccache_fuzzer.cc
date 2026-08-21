#include "/src/ccache/src/ccache/util/path.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <filesystem>


namespace fs = std::filesystem;

// Sink to prevent dead-code elimination of path processing
static volatile size_t g_sink = 0;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, std::size_t size)
{
    if (!data || size == 0) return 0;

    const std::string input(reinterpret_cast<const char*>(data), size);
    fs::path p_input(input);

    // make_relative_path has DEBUG_ASSERT(dir1.is_absolute()). 
    // We ensure absolute paths by appending a dummy suffix.
    fs::path dir1 = fs::absolute(input + "/dummy1");
    fs::path dir2 = fs::absolute(input + "/dummy2");
    fs::path target = fs::absolute(input + "/target/path");

    // 1. add_exe_suffix: Appends .exe on Windows if missing
    auto s1 = util::add_exe_suffix(p_input);
    g_sink ^= s1.native().size();

    // 2. apparent_cwd: Compares PWD env var with actual CWD
    auto s2 = util::apparent_cwd(p_input);
    g_sink ^= s2.native().size();

    // 3. get_dev_null_path: Returns platform-specific /dev/null or nul:
    g_sink ^= std::strlen(util::get_dev_null_path());

    // 4. lexically_normal: Cleans up ., .., and duplicate separators
    auto s3 = util::lexically_normal(p_input);
    g_sink ^= s3.native().size();

    // 5. make_relative_path: Complex path resolution, sorting, and equivalent checks
    auto s4 = util::make_relative_path(dir1, dir2, target);
    g_sink ^= s4.native().size();

    // 6. path_components_equal_case_aware: Compares individual path components
    bool b1 = util::path_components_equal_case_aware(dir1.filename(), dir2.filename());
    g_sink ^= static_cast<size_t>(b1);

    // 7. path_component_starts_with_case_aware: Prefix check for components
    bool b2 = util::path_component_starts_with_case_aware(dir1.filename(), p_input.filename());
    g_sink ^= static_cast<size_t>(b2);

    // 8. path_starts_with (single prefix): Checks if path begins with prefix
    bool b3 = util::path_starts_with(dir1, dir2);
    g_sink ^= static_cast<size_t>(b3);

    // 9. path_starts_with (vector of prefixes): Multi-prefix check
    std::vector<fs::path> prefixes = {dir1, dir2, target};
    bool b4 = util::path_starts_with(dir1, prefixes);
    g_sink ^= static_cast<size_t>(b4);

    g_sink ^= input.size();
    return 0;
}
