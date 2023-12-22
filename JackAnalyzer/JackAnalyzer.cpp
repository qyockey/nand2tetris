#include <iostream>
#include <fstream>
#include <filesystem>

#include "JackTokenizer.h"

void analyzeJackFile(std::string jackFilePath);
void analyzeDirectory(std::string dirPath);
static std::string getXmlFilePath(std::string jackFilePath);

namespace fs = std::filesystem;

int main(int argc, char *argv[])
{
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <path_to_directory_or_jack_file>\n";
        return EXIT_FAILURE;
    }

    std::string path = argv[1];
    if (fs::is_regular_file(path)) {
        std::cout << "regular file: " << path << '\n';
        analyzeJackFile(path);
    } else if (fs::is_directory(path)) {
        std::cout << "directory: " << path << '\n';
        analyzeDirectory(path);
    } else {
        std::cerr << "Invalid path: " << path << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void analyzeDirectory(std::string dirPath) {
    for (const auto& entry : fs::directory_iterator(dirPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".jack") {
            std::cout << "analyzing file: " << entry << '\n';
            analyzeJackFile(entry.path().string());
        }
    }
}

void analyzeJackFile(std::string jackFilePath) {
    JackTokenizer tokenizer(jackFilePath);
    std::ofstream xmlFile(getXmlFilePath(jackFilePath));
    xmlFile << "<tokens>\n";
    while (tokenizer.hasMoreTokens()) {
        tokenizer.advance();
        switch (tokenizer.getTokenType()) {
            case JackTokenizer::TokenType::KEYWORD:
                xmlFile << "<keyword> " << tokenizer.keyword() << " </keyword>\n";
                break;
            case JackTokenizer::TokenType::SYMBOL:
                xmlFile << "<symbol> " << tokenizer.symbol() << " </symbol>\n";
                break;
            case JackTokenizer::TokenType::IDENTIFIER:
                xmlFile << "<identifier> " << tokenizer.identifier() << " </identifier>\n";
                break;
            case JackTokenizer::TokenType::INT_CONST:
                xmlFile << "<integerConstant> " << tokenizer.intVal() << " </integerConstant>\n";
                break;
            case JackTokenizer::TokenType::STRING_CONST:
                xmlFile << "<stringConstant> " << tokenizer.stringVal() << " </stringConstant>\n";
                break;
            default:
                xmlFile << "ERROR: UNKNOWN TOKEN TYPE" << '\n';
                break;
        }
    }
    xmlFile << "</tokens>\n";
    xmlFile.close();
}

std::string getXmlFilePath(std::string jackFilePath)
{
    return jackFilePath.replace(jackFilePath.find(".jack"), 5, "t.xml");
}
