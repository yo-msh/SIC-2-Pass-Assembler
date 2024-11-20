// opcode.h
#ifndef OPCODE_H
#define OPCODE_H

#include <string>
#include <unordered_map>
#include <algorithm>

class Opcode
{
private:
    std::unordered_map<std::string, std::string> opcodeMap;

public:
    // Constructor declaration
    Opcode();

    // Member function declarations
    void initializeOpcodes();
    bool isOpcode(const std::string &op) const;
    std::string getMachineCode(const std::string &op) const;
};

#endif // OPCODE_H