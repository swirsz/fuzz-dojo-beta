#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

namespace {

// Consume parsed values so the optimizer cannot trivially discard all of the
// filename/path processing performed by the fuzz target.
volatile std::size_t g_sink = 0;

void consume(const fs::path& path)
{
    g_sink ^= path.native().size();
}

} // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, std::size_t size)
{
    if (data == nullptr || size == 0) {
        return 0;
    }

    // Preserve the complete fuzz input, including embedded NUL bytes. All
    // operations below are lexical only; they do not access the filesystem.
    const std::string input(reinterpret_cast<const char*>(data), size);
    const fs::path path(input);

    // Exercise common filename/path parsing operations.
    consume(path.filename());
    consume(path.parent_path());
    consume(path.stem());
    consume(path.extension());
    consume(path.root_name());
    consume(path.root_directory());
    consume(path.relative_path());

    // Exercise normalization and path-component iteration as well. These are
    // useful for inputs containing repeated separators, '.', '..', unusual
    // extensions, or very deeply nested path strings.
    consume(path.lexically_normal());

    for (const auto& component : path) {
        consume(component);
    }

    // Simple filename classification commonly needed by cache/compiler tools.
    // No assumptions are made about encoding; fuzz bytes are handled as an
    // opaque byte string by fs::path on POSIX systems.
    const fs::path filename = path.filename();
    const bool has_extension = filename.has_extension();
    const bool has_stem = filename.has_stem();
    const bool is_absolute = path.is_absolute();
    const bool is_relative = path.is_relative();

    g_sink ^= static_cast<std::size_t>(has_extension);
    g_sink ^= static_cast<std::size_t>(has_stem) << 1;
    g_sink ^= static_cast<std::size_t>(is_absolute) << 2;
    g_sink ^= static_cast<std::size_t>(is_relative) << 3;

    return 0;
}
