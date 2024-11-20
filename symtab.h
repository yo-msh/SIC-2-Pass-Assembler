// symtab.h
#ifndef SYMTAB_H
#define SYMTAB_H

#include <string>
#include <unordered_map>

class Symtab
{
public:
    void addSymbol(const std::string &symbol, int address);
    bool contains(const std::string &symbol) const;
    int getAddress(const std::string &symbol) const;
    void writeToFile(const std::string &filename) const;

private:
    std::unordered_map<std::string, int> symbolMap;
};

#endif // SYMTAB_H