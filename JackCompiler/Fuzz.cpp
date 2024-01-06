#include <fstream>
#include <filesystem>
#include <stdint.h>
#include <stddef.h>

#include "CompilationEngine.h"
#include "UnexpectedTokenException.h"

namespace fs = std::filesystem;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    try {
        const auto input = reinterpret_cast<const char *>(data);
        const fs::path jackFilePath = "./fuzz.jack";
        const fs::path vmFilePath = "./fuzz.vm";
        std::ofstream jackFile(jackFilePath);
        jackFile.write(input, size);
        jackFile.close();
        CompilationEngine compilationEngine(jackFilePath, vmFilePath);
        compilationEngine.compileClass();
        return 0;
    } catch (const UnexpectedTokenException&) {
        return 0;
    } catch (const std::exception &) {
        return 1;
    }
}

