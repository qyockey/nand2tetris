#include <stdexcept>
#include <sstream>
#include <unordered_map>

#include "Variable.h"

Variable::Variable() = default;

Variable::Variable(const std::string_view& name, const std::string_view& type,
        const Variable::Kind& kind, unsigned index)
            : name(name), type(type), kind(kind), index(index) {}

std::string Variable::getName() const {
    return name;
}

std::string Variable::getType() const {
    return type;
}

Variable::Kind Variable::getKind() const {
    return kind;
}

unsigned Variable::getNumber() const {
    return index;
}

std::string Variable::getVmRepresentation() const {
    std::stringstream representationStream;
    switch (getKind()) {
        case Variable::Kind::ARG:
            representationStream << "argument";
            break;
        case Variable::Kind::FIELD:
            representationStream << "this";
            break;
        case Variable::Kind::STATIC:
            representationStream << "static";
            break;
        case Variable::Kind::VAR:
            representationStream << "local";
            break;
    }
    representationStream << ' ' << getNumber();
    return representationStream.str();
}

void Variable::setName(const std::string_view& newName) {
    name = newName;
}

void Variable::setType(const std::string_view& newType) {
    type = newType;
}

void Variable::setKind(const Variable::Kind& newKind) {
    kind = newKind;
}

void Variable::setIndex(const unsigned newIndex) {
    index = newIndex;
}

std::string Variable::kindToStr(const Variable::Kind kind) {
    static std::unordered_map<Kind, std::string> kindToStr = {
        {Kind::ARG, "arg"},
        {Kind::FIELD, "field"},
        {Kind::STATIC, "static"},
        {Kind::VAR, "var"}
    };
    return kindToStr.at(kind);
}

Variable::Kind Variable::strToKind(const std::string& str) {
    static std::unordered_map<std::string, Kind> strToKind = {
        {"arg", Kind::ARG},
        {"field", Kind::FIELD},
        {"static", Kind::STATIC},
        {"var", Kind::VAR}
    };
    return strToKind.at(str);
}