#ifndef VARIABLE_H
#define VARIABLE_H

#include <string>

class Variable {
public:
    enum class Kind {
        STATIC,
        FIELD,
        ARG,
        VAR
    };
    Variable();
    Variable(const std::string_view& name, const std::string_view& type,
            const Kind& kind, unsigned index);
    std::string getName() const;
    std::string getType() const;
    Kind getKind() const;
    unsigned getNumber() const;
    std::string getVmRepresentation() const;
    void setName(const std::string_view& newName);
    void setType(const std::string_view& newType);
    void setKind(const Kind& newKind);
    void setIndex(const unsigned newNumber);
    static std::string kindToStr(const Kind kind);
    static Kind strToKind(const std::string& str);

private:
    std::string name {};
    std::string type {};
    Kind kind {};
    unsigned index {};
};

#endif

