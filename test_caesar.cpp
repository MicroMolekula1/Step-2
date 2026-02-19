/*
 * test_caesar.cpp - Test program for libcaesar
 *
 * Demonstrates dynamic runtime loading via dlopen/dlsym.
 *
 * Usage:
 *   ./test_caesar <library_path> <key> <input_file> <output_file>
 *
 * key can be:
 *   - single character (e.g. K)
 *   - number 0..255 (e.g. 75)
 */

#include <dlfcn.h>

#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

using set_key_func = void (*)(char);
using caesar_func = void (*)(void*, void*, int);

static bool parse_key(const char* s, char& out_key)
{
    if (s == nullptr || s[0] == '\0') {
        return false;
    }

    if (s[1] == '\0') {
        out_key = s[0];
        return true;
    }

    char* end = nullptr;
    errno = 0;
    long v = std::strtol(s, &end, 0);
    if (errno != 0 || end == s || *end != '\0') {
        return false;
    }
    if (v < 0 || v > 255) {
        return false;
    }

    out_key = static_cast<char>(static_cast<unsigned char>(v));
    return true;
}

static bool read_file_all(const char* path, std::vector<std::uint8_t>& out)
{
    std::FILE* fp = std::fopen(path, "rb");
    if (!fp) {
        return false;
    }

    if (std::fseek(fp, 0, SEEK_END) != 0) {
        std::fclose(fp);
        return false;
    }
    long size = std::ftell(fp);
    if (size < 0) {
        std::fclose(fp);
        return false;
    }
    if (std::fseek(fp, 0, SEEK_SET) != 0) {
        std::fclose(fp);
        return false;
    }

    out.assign(static_cast<std::size_t>(size), 0);
    if (!out.empty()) {
        const std::size_t n = std::fread(out.data(), 1, out.size(), fp);
        if (n != out.size()) {
            std::fclose(fp);
            return false;
        }
    }

    std::fclose(fp);
    return true;
}

static bool write_file_all(const char* path, const std::vector<std::uint8_t>& data)
{
    std::FILE* fp = std::fopen(path, "wb");
    if (!fp) {
        return false;
    }

    if (!data.empty()) {
        const std::size_t n = std::fwrite(data.data(), 1, data.size(), fp);
        if (n != data.size()) {
            std::fclose(fp);
            return false;
        }
    }

    std::fclose(fp);
    return true;
}

int main(int argc, char** argv)
{
    if (argc != 5) {
        std::fprintf(stderr,
                     "Usage: %s <library_path> <key> <input_file> <output_file>\n"
                     "Example: %s ./libcaesar.so K input.txt output.bin\n"
                     "         %s ./libcaesar.so 75 input.txt output.bin\n",
                     argv[0], argv[0], argv[0]);
        return 1;
    }

    const char* lib_path = argv[1];
    const char* key_str = argv[2];
    const char* input_path = argv[3];
    const char* output_path = argv[4];

    char key = 0;
    if (!parse_key(key_str, key)) {
        std::fprintf(stderr, "Invalid key: '%s' (use single char or 0..255)\n", key_str);
        return 1;
    }

    void* handle = dlopen(lib_path, RTLD_LAZY);
    if (!handle) {
        std::fprintf(stderr, "dlopen('%s') failed: %s\n", lib_path, dlerror());
        return 1;
    }

    dlerror();
    auto set_key = reinterpret_cast<set_key_func>(dlsym(handle, "set_key"));
    const char* e1 = dlerror();
    if (e1 != nullptr || set_key == nullptr) {
        std::fprintf(stderr, "dlsym(set_key) failed: %s\n", e1 ? e1 : "unknown error");
        dlclose(handle);
        return 1;
    }

    dlerror();
    auto caesar = reinterpret_cast<caesar_func>(dlsym(handle, "caesar"));
    const char* e2 = dlerror();
    if (e2 != nullptr || caesar == nullptr) {
        std::fprintf(stderr, "dlsym(caesar) failed: %s\n", e2 ? e2 : "unknown error");
        dlclose(handle);
        return 1;
    }

    std::vector<std::uint8_t> input;
    if (!read_file_all(input_path, input)) {
        std::fprintf(stderr, "Failed to read input file: %s\n", input_path);
        dlclose(handle);
        return 1;
    }

    std::vector<std::uint8_t> output(input.size(), 0);

    set_key(key);
    caesar(input.data(), output.data(), static_cast<int>(output.size()));

    if (!write_file_all(output_path, output)) {
        std::fprintf(stderr, "Failed to write output file: %s\n", output_path);
        dlclose(handle);
        return 1;
    }

    dlclose(handle);
    return 0;
}
