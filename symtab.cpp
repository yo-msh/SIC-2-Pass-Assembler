// symtab.cpp
#include "symtab.h"
#include <fstream>
#include <iostream>

void Symtab::addSymbol(const std::string &symbol, int address)
{
    symbolMap[symbol] = address;
}

bool Symtab::contains(const std::string &symbol) const
{
    return symbolMap.find(symbol) != symbolMap.end();
}

int Symtab::getAddress(const std::string &symbol) const
{
    auto it = symbolMap.find(symbol);
    if (it != symbolMap.end())
    {
        return it->second;
    }
    else
    {
        return 0; // technicalliy it means the symbol doesnt exists
    }
}

void Symtab::writeToFile(const std::string &filename) const
{
    std::ofstream outfile(filename);
    if (!outfile.is_open())
    {
        std::cerr << "Error: Cannot open symbol table file " << filename << " for writing." << std::endl;
        return;
    }

    for (const auto &entry : symbolMap)
    {
        outfile << entry.first << " " << std::hex << entry.second << std::dec << std::endl;
    }

    outfile.close();
}