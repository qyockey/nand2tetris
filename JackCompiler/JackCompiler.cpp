#include <iostream>
#include <filesystem>

#include "CompilationEngine.h"
#include "JackTokenizer.h"
#include "UnexpectedTokenException.h"

static void compileJackFile(const fs::path& jackFilePath);
static void compileDirectory(const fs::path& dirPath);
static fs::path getVmFilePath(const fs::path& jackFilePath);

namespace fs = std::filesystem;

int main(int argc, char *argv[]) {
    std::string compilationPathStr;
    if (argc == 1) {
        compilationPathStr = ".";
    } else if (argc == 2) {
        compilationPathStr = argv[1];
    } else {
        std::cerr << "Usage: " << argv[0] << " <path_to_directory_or_jack_file>\n";
        return EXIT_FAILURE;
    }
    if (fs::path compilationPath(compilationPathStr);
            fs::is_regular_file(compilationPath)) {
        compileJackFile(compilationPath);
    } else if (fs::is_directory(compilationPath)) {
        compileDirectory(compilationPath);
    } else {
        std::cerr << "Error: '" << compilationPathStr <<
                "' is not a file or directory\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

static void compileDirectory(const fs::path& dirPath) {
    for (const auto& entry : fs::directory_iterator(dirPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".jack") {
            compileJackFile(entry.path().string());
        }
    }
}

static void compileJackFile(const fs::path& jackFilePath) {
    CompilationEngine compilationEngine(jackFilePath, getVmFilePath(jackFilePath));
    try {
        compilationEngine.compileClass();
        std::cout << jackFilePath.string() << " compiled successfully\n";
    } catch (const UnexpectedTokenException& e) {
        std::cout << jackFilePath.string() << " compilation failed\n";
        compilationEngine.printJackFilePosition();
        std::cout << e.what() << "\n";
    }
}

static fs::path getVmFilePath(const fs::path& jackFilePath) {
    const std::string jackFilePathStr = jackFilePath.string();
    std::string vmFilePathStr = jackFilePathStr;
    vmFilePathStr.replace(vmFilePathStr.find(".jack"), 5, ".vm");
    return fs::path(vmFilePathStr);
}

