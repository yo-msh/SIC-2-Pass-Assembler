// opcode.cpp
#include "opcode.h"
#include <algorithm>

Opcode::Opcode()
{
    initializeOpcodes();
}

void Opcode::initializeOpcodes()
{
    opcodeMap["LDA"] = "00";
    opcodeMap["LDX"] = "04";
    opcodeMap["LDL"] = "08";
    opcodeMap["STA"] = "0C";
    opcodeMap["STX"] = "10";
    opcodeMap["STL"] = "14";
    opcodeMap["ADD"] = "18";
    opcodeMap["SUB"] = "1C";
    opcodeMap["MUL"] = "20";
    opcodeMap["DIV"] = "24";
    opcodeMap["COMP"] = "28";
    opcodeMap["TIX"] = "2C";
    opcodeMap["JEQ"] = "30";
    opcodeMap["JGT"] = "34";
    opcodeMap["JLT"] = "38";
    opcodeMap["J"] = "3C";
    opcodeMap["AND"] = "40";
    opcodeMap["OR"] = "44";
    opcodeMap["JSUB"] = "48";
    opcodeMap["RSUB"] = "4C";
    opcodeMap["LDCH"] = "50";
    opcodeMap["STCH"] = "54";
    opcodeMap["RD"] = "D8";
    opcodeMap["WD"] = "DC";
    opcodeMap["TD"] = "E0";
}

bool Opcode::isOpcode(const std::string &op) const
{
    std::string upperOp = op;
    std::transform(upperOp.begin(), upperOp.end(), upperOp.begin(), ::toupper);
    return opcodeMap.find(upperOp) != opcodeMap.end();
}

// Get machine code implementation
std::string Opcode::getMachineCode(const std::string &op) const
{
    std::string upperOp = op;
    std::transform(upperOp.begin(), upperOp.end(), upperOp.begin(), ::toupper);
    auto it = opcodeMap.find(upperOp);
    return (it != opcodeMap.end()) ? it->second : "";
}