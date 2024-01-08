#include <unordered_map>

#include "VmWriter.h"

VmWriter::VmWriter(const fs::path& vmFilePath) : vmFile(vmFilePath) {}

VmWriter::~VmWriter() {
    vmFile.close();
}

void VmWriter::writePush(const PushSegment segment, const unsigned count) {
    static std::unordered_map<PushSegment, std::string> pushSegmentToStr {
        {PushSegment::LOCAL, "local"},
        {PushSegment::ARG, "argument"},
        {PushSegment::THIS, "this"},
        {PushSegment::THAT, "that"},
        {PushSegment::POINTER, "pointer"},
        {PushSegment::STATIC, "static"},
        {PushSegment::TEMP, "temp"},
        {PushSegment::CONSTANT, "constant"}
    };
    vmFile << "push " << pushSegmentToStr.at(segment) << ' ' << count << '\n';
}

void VmWriter::writePush(const Variable& variable) {
    vmFile << "push " << variable.getVmRepresentation() << '\n';
}

void VmWriter::writePop(const PopSegment segment, const unsigned count) {
    static std::unordered_map<PopSegment, std::string> popSegmentToStr {
        {PopSegment::LOCAL, "local"},
        {PopSegment::ARG, "argument"},
        {PopSegment::THIS, "this"},
        {PopSegment::THAT, "that"},
        {PopSegment::POINTER, "pointer"},
        {PopSegment::STATIC, "static"},
        {PopSegment::TEMP, "temp"}
    };
    vmFile << "pop " << popSegmentToStr.at(segment) << ' ' << count << '\n';
}

void VmWriter::writePop(const Variable& variable) {
    vmFile << "pop " << variable.getVmRepresentation() << '\n';
}

void VmWriter::writeArithmetic(const Command command) {
    static std::unordered_map<Command, std::string> commandToStr {
        {Command::ADD, "add"},
		{Command::SUB, "sub"},
		{Command::AND, "and"},
		{Command::OR, "or"},
		{Command::NEG, "neg"},
		{Command::NOT, "not"},
		{Command::LT, "lt"},
		{Command::EQ, "eq"},
		{Command::GT, "gt"},
    };
    vmFile << commandToStr.at(command) << '\n';
}

void VmWriter::writeCall(const std::string& subroutineName,
        const unsigned numArgs) {
    vmFile << "call " << subroutineName << ' ' << numArgs << '\n';
}

void VmWriter::writeFunction(const std::string& subroutineName,
        const unsigned numVars) {
    vmFile << "function " << subroutineName << ' ' << numVars << '\n';
}

void VmWriter::writeReturn() {
    vmFile << "return\n";
}

void VmWriter::writeLabel(const std::string &label) {
    vmFile << "label " << label << '\n';
}

void VmWriter::writeGoto(const std::string &label) {
    vmFile << "goto " << label << '\n';
}

void VmWriter::writeIfGoto(const std::string &label) {
    vmFile << "if-goto " << label << '\n';
}

