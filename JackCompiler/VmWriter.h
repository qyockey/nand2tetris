#ifndef VM_WRITER_H
#define VM_WRITER_H

#include <filesystem>
#include <fstream>

#include "Variable.h"

namespace fs = std::filesystem;

class VmWriter {
public:
    enum class PushSegment {
            LOCAL, ARG, THIS, THAT, POINTER, STATIC, TEMP, CONSTANT  
    };
    enum class PopSegment {
            LOCAL, ARG, THIS, THAT, POINTER, STATIC, TEMP
    };
    enum class Command {
            ADD, SUB, AND, OR, NEG, NOT, LT, EQ, GT
    };
    explicit VmWriter(const fs::path& vmFilePath);
    ~VmWriter();
    void writePush(const PushSegment segment, const unsigned count);
    void writePush(const Variable& variable);
    void writePop(const PopSegment segment, const unsigned count);
    void writePop(const Variable& variable);
    void writeArithmetic(const Command command);
    void writeCall(const std::string& subroutineName, const unsigned numArgs);
    void writeFunction(const std::string& subroutineName,
            const unsigned numVars);
    void writeReturn();
    void writeLabel(const std::string& label);
    void writeGoto(const std::string& label);
    void writeIfGoto(const std::string& label);

private:
    std::ofstream vmFile;
};

#endif

