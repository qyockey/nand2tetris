#include <iostream>
#include <fstream>
#include <filesystem>

#include "CompilationEngine.h"
#include "JackTokenizer.h"
#include "UnexpectedTokenException.h"

static void analyzeJackFile(const fs::path& jackFilePath);
static void analyzeDirectory(const fs::path& dirPath);
static fs::path getXmlFilePath(const fs::path& jackFilePath);

namespace fs = std::filesystem;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <path_to_directory_or_jack_file>\n";
        return EXIT_FAILURE;
    }

    if (fs::path path(argv[1]); fs::is_regular_file(path)) {
        analyzeJackFile(path);
    } else if (fs::is_directory(path)) {
        analyzeDirectory(path);
    } else {
        std::cerr << "Error: '" << argv[1] << "' is not a file or directory\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

static void analyzeDirectory(const fs::path& dirPath) {
    for (const auto& entry : fs::directory_iterator(dirPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".jack") {
            analyzeJackFile(entry.path().string());
        }
    }
}

static void analyzeJackFile(const fs::path& jackFilePath) {
    CompilationEngine compilationEngine(jackFilePath, getXmlFilePath(jackFilePath));
    try {
        compilationEngine.compileClass();
        std::cout << jackFilePath.string() << " compiled successfully\n";
    } catch (const UnexpectedTokenException& e) {
        std::cout << jackFilePath.string() << " compilation failed\n";
        std::cout << e.what() << "\n";
    }
}

static fs::path getXmlFilePath(const fs::path& jackFilePath) {
    const std::string jackFilePathStr = jackFilePath.string();
    std::string xmlFilePathStr = jackFilePathStr;
    xmlFilePathStr.replace(xmlFilePathStr.find(".jack"), 5, ".xml");
    return fs::path(xmlFilePathStr);
}

