// main.cpp

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <algorithm>
// #include <set>
#include <unordered_map>
#include "opcode.h"
#include "symtab.h"

using namespace std;

void passOne(const string &inputFile, Symtab &symtab,
             vector<string> &intermediateLines, int &programLength, Opcode &opcodeTable);
void passTwo(const vector<string> &intermediateLines, const Symtab &symtab,
             const Opcode &opcodeTable, const string &outputFile, int programLength);

string trim(const string &str);
bool isAssemblerDirective(const string &str);
void separate(const string &line, string &label, string &opcode, string &operand, const Opcode &opcodeTable);
string intToHex(int value, int width);

int main()
{
    string inputFile = "input.txt";
    string outputFile = "output.txt";
    string intermediateFile = "intermediate.txt";
    string symtabFile = "symtab.txt";

    Symtab symtab;
    Opcode opcodeTable;
    vector<string> intermediateLines;
    int programLength = 0;

    // Pass One: Build Symbol Table and Intermediate File
    passOne(inputFile, symtab, intermediateLines, programLength, opcodeTable);

    // Write intermediate file
    ofstream intermediateFileStream(intermediateFile);
    if (!intermediateFileStream.is_open())
    {
        cerr << "Error: Cannot open intermediate file " << intermediateFile << " for writing." << endl;
        return 1;
    }
    for (const auto &line : intermediateLines)
    {
        intermediateFileStream << line << endl;
    }
    intermediateFileStream.close();

    // Write symbol table
    symtab.writeToFile(symtabFile);

    // Pass Two: Generate Object Code
    passTwo(intermediateLines, symtab, opcodeTable, outputFile, programLength);

    cout << "Assembly completed successfully. Output written to " << outputFile << endl;

    return 0;
}

// Helper Function: Trim leading and trailing whitespace, just in case (while testing had this issue)
string trim(const string &str)
{
    size_t first = str.find_first_not_of(" \t");
    if (first == string::npos)
        return "";
    size_t last = str.find_last_not_of(" \t");
    return str.substr(first, (last - first + 1));
}

// Separate function to parse a line into label, opcode, and operand
void separate(const string &line, string &label, string &opcode, string &operand, const Opcode &opcodeTable)
{
    string trimmedLine = trim(line);
    vector<string> tokens;
    stringstream ss(trimmedLine);
    string token;

    // Get all tokens from the line
    while (ss >> token)
    {
        tokens.push_back(token);
    }

    label = "";
    opcode = "";
    operand = "";

    if (tokens.empty())
    {
        return;
    }

    // If the first character is not whitespace, assume the first token is a label
    if (!isspace(line[0]))
    {
        if (tokens.size() >= 3)
        {
            label = tokens[0];
            opcode = tokens[1];
            operand = tokens[2];
        }
        else if (tokens.size() == 2)
        {
            label = tokens[0];
            opcode = tokens[1];
        }
        else if (tokens.size() == 1)
        {
            label = tokens[0];
        }
    }
    // Line has no label
    else
    {
        if (tokens.size() >= 2)
        {
            opcode = tokens[0];
            operand = tokens[1];
        }
        else if (tokens.size() == 1)
        {
            opcode = tokens[0];
        }
    }
}

// Check if a string is an assembler directive
bool isAssemblerDirective(const string &str)
{
    static const set<string> directives = {
        "START", "END", "WORD", "RESW", "RESB", "BYTE", "BASE", "NOBASE", "EQU"};
    string upperStr = str;
    transform(upperStr.begin(), upperStr.end(), upperStr.begin(), ::toupper);
    return directives.find(upperStr) != directives.end();
}

// Convert integer to hexadecimal string with leading zeros
string intToHex(int value, int width)
{
    stringstream ss;
    ss << uppercase << hex << setw(width) << setfill('0') << value;
    return ss.str();
}

void passOne(const string &inputFile, Symtab &symtab,
             vector<string> &intermediateLines, int &programLength, Opcode &opcodeTable)
{
    ifstream input(inputFile);
    if (!input.is_open())
    {
        cerr << "Error: Cannot open input file " << inputFile << endl;
        return;
    }

    string line;
    int locctr = 0;
    int startAddress = 0;
    bool started = false;

    while (getline(input, line))
    {
        if (line.empty() || line[0] == '.')
        {
            continue;
        }

        string label, opcode, operand;
        string currentLoc = intToHex(locctr, 6);

        // Parse the line
        separate(line, label, opcode, operand, opcodeTable);
        cout << "Processing Line: " << line << endl;
        cout << "Label: " << label << ", Opcode: " << opcode << ", Operand: " << operand << endl;

        transform(opcode.begin(), opcode.end(), opcode.begin(), ::toupper);

        if (opcode == "START")
        {
            if (!started)
            {
                startAddress = stoi(operand, nullptr, 16);
                locctr = startAddress;
                started = true;
                intermediateLines.push_back(currentLoc + " " + label + " " + opcode + " " + operand);
                continue;
            }
        }

        if (!started)
        {
            startAddress = 0;
            locctr = 0;
            started = true;
        }

        // Add label to symbol table if present
        if (!label.empty())
        {
            if (symtab.contains(label))
            {
                cerr << "Error: Duplicate symbol " << label << endl;
            }
            else
            {
                symtab.addSymbol(label, locctr);
            }
        }

        // Create intermediate line
        intermediateLines.push_back(currentLoc + " " + label + " " + opcode + " " + operand);

        // Update location counter based on instruction type
        if (opcode == "END")
        {
            break;
        }
        else if (opcodeTable.isOpcode(opcode))
        {
            locctr += 3;
        }
        else if (opcode == "WORD")
        {
            locctr += 3;
        }
        else if (opcode == "RESW")
        {
            locctr += 3 * stoi(operand);
        }
        else if (opcode == "RESB")
        {
            locctr += stoi(operand);
        }
        else if (opcode == "BYTE")
        {
            if (toupper(operand[0]) == 'C')
            {
                // Character constant
                locctr += operand.length() - 3;
            }
            else if (toupper(operand[0]) == 'X')
            {
                // Hex constant
                locctr += (operand.length() - 3) / 2; // Two hex digits = 1 byte
            }
        }
        else if (!isAssemblerDirective(opcode))
        {
            cerr << "Error: Invalid opcode " << opcode << " in line: " << line << endl;
        }
    }

    programLength = locctr - startAddress;
    input.close();
}

void passTwo(const vector<string> &intermediateLines, const Symtab &symtab,
             const Opcode &opcodeTable, const string &outputFile, int programLength)
{
    ofstream output(outputFile);
    if (!output.is_open())
    {
        cerr << "Error: Cannot open output file " << outputFile << " for writing." << endl;
        return;
    }

    string headerRecord, endRecord;
    vector<string> textRecords;
    int startAddress = 0;

    // Ensure there is at least one line for the header
    if (intermediateLines.empty())
    {
        cerr << "Error: Intermediate file is empty." << endl;
        return;
    }

    // Parse Header Record from intermediateLines[0]
    string headerLine = intermediateLines[0];
    stringstream fs(headerLine);
    string locctrStr, label, opcodeStr, operand;
    fs >> locctrStr >> label >> opcodeStr >> operand;

    // Header Record Construction
    string programName = label.empty() ? "      " : label;
    if (programName.length() < 6)
        programName += string(6 - programName.length(), ' ');
    else if (programName.length() > 6)
        programName = programName.substr(0, 6);

    if (opcodeStr == "START")
    {
        startAddress = stoi(operand, nullptr, 16); // Operand is starting address in hex
    }
    else
    {
        startAddress = 0;
    }

    // Create Header Record
    headerRecord = "H " + programName + " " + intToHex(startAddress, 6) + " " + intToHex(programLength, 6);
    output << headerRecord << endl;

    // Initialize Text Records
    string currentTextAddress = "";
    vector<string> currentObjectCodes; // Stores object codes
    int currentRecordLength = 0;       // in bytes
    endRecord = "";                    // Initialize endRecord

    // Iterate through intermediateLines starting from the second line
    for (size_t i = 1; i < intermediateLines.size(); ++i)
    {
        string line = intermediateLines[i];
        stringstream ss(line);
        string currLocctrStr, labelField, opcodeField, operandField;
        vector<string> tokens;
        string token;

        // Split the line into tokens
        while (ss >> token)
        {
            tokens.push_back(token);
        }

        if (tokens.empty())
            continue;

        // Determine if the line has a label based on the number of tokens
        if (tokens.size() == 4)
        {
            currLocctrStr = tokens[0];
            labelField = tokens[1];
            opcodeField = tokens[2];
            operandField = tokens[3];
        }
        else if (tokens.size() == 3)
        {
            currLocctrStr = tokens[0];
            labelField = "";
            opcodeField = tokens[1];
            operandField = tokens[2];
        }
        else if (tokens.size() == 2)
        {
            currLocctrStr = tokens[0];
            labelField = "";
            opcodeField = tokens[1];
            operandField = "";
        }
        else
        {
            cerr << "Error: Invalid format in intermediate line: " << line << endl;
            continue;
        }

        // Handle assembler directives
        if (isAssemblerDirective(opcodeField))
        {
            if (opcodeField == "END")
            {
                // Determine execution address
                string execAddress = "";
                if (!operandField.empty())
                {
                    if (symtab.contains(operandField))
                    {
                        execAddress = intToHex(symtab.getAddress(operandField), 6);
                    }
                    else
                    {
                        // If operand not found, default to startAddress
                        execAddress = intToHex(startAddress, 6);
                    }
                }
                else
                {
                    execAddress = intToHex(startAddress, 6);
                }

                // Store End Record to write later
                endRecord = "E " + execAddress;
                // Continue processing without writing
                continue;
            }
            else if (opcodeField == "WORD")
            {
                // Convert operand to object code
                int value = stoi(operandField);
                string objCode = intToHex(value, 6);

                // Add to text records
                if (currentObjectCodes.empty())
                {
                    currentTextAddress = currLocctrStr;
                }
                if (currentRecordLength + 3 > 30)
                {
                    // Flush current text record
                    string textRecord = "T " + currentTextAddress + " " + intToHex(currentRecordLength, 2);
                    for (const auto &obj : currentObjectCodes)
                    {
                        textRecord += " " + obj;
                    }
                    textRecords.push_back(textRecord);

                    // Start new text record
                    currentTextAddress = currLocctrStr;
                    currentObjectCodes.clear();
                }

                currentObjectCodes.push_back(objCode);
                currentRecordLength += 3;
            }
            else if (opcodeField == "BYTE")
            {
                // Handle BYTE directive
                string objCode = "";
                if (toupper(operandField[0]) == 'C')
                {
                    // Character constant
                    string chars = operandField.substr(2, operandField.size() - 3); // Extract between C'...' or c'...'
                    for (char c : chars)
                    {
                        objCode += intToHex(static_cast<unsigned char>(c), 2);
                    }
                }
                else if (toupper(operandField[0]) == 'X')
                {
                    // Hex constant
                    objCode = operandField.substr(2, operandField.size() - 3); // Extract between X'...'
                }

                // Group into 6-character chunks (3 bytes)
                for (size_t j = 0; j < objCode.length(); j += 6)
                {
                    string group = objCode.substr(j, 6);
                    // Pad with zeros if necessary
                    while (group.length() < 6)
                        group += '0';

                    // Check if we need to start a new text record
                    if (currentObjectCodes.empty())
                    {
                        currentTextAddress = currLocctrStr;
                    }

                    if (currentRecordLength + 3 > 30)
                    {
                        // Flush current text record
                        string textRecord = "T " + currentTextAddress + " " + intToHex(currentRecordLength, 2);
                        for (const auto &obj : currentObjectCodes)
                        {
                            textRecord += " " + obj;
                        }
                        textRecords.push_back(textRecord);

                        // Start new text record
                        currentTextAddress = currLocctrStr;
                        currentObjectCodes.clear();
                        currentObjectCodes.push_back(group);
                        currentRecordLength = 3;
                    }
                    else
                    {
                        currentObjectCodes.push_back(group);
                        currentRecordLength += 3;
                    }
                }
            }
            else if (opcodeField == "RESW" || opcodeField == "RESB")
            {
                // RESW and RESB do not generate object code
                // Flush current text record if any
                if (!currentObjectCodes.empty())
                {
                    string textRecord = "T " + currentTextAddress + " " + intToHex(currentRecordLength, 2);
                    for (const auto &obj : currentObjectCodes)
                    {
                        textRecord += " " + obj;
                    }
                    textRecords.push_back(textRecord);
                    currentObjectCodes.clear();
                    currentRecordLength = 0;
                }
            }
            else
            {
                cerr << "Error: Unsupported directive " << opcodeField << " in line: " << line << endl;
            }
        }
        else
        {
            // It's a machine instruction opcode
            if (!opcodeTable.isOpcode(opcodeField))
            {
                cerr << "Error: Invalid opcode " << opcodeField << " in line: " << line << endl;
                continue; // Skip to next line
            }

            // Generate object code
            string machineCode = opcodeTable.getMachineCode(opcodeField);
            string objCode = "";

            if (machineCode.empty())
            {
                cerr << "Error: No machine code for opcode " << opcodeField << endl;
                continue; // Skip to next line
            }

            if (opcodeField == "RSUB")
            {
                // RSUB has no operand, fixed object code
                objCode = machineCode + "0000";
            }
            else
            {
                // Handle operands
                // Check for immediate addressing or indexed addressing
                bool isImmediate = false;
                bool isIndexed = false;
                string symbol = operandField;
                if (!operandField.empty())
                {
                    if (operandField[0] == '#')
                    {
                        isImmediate = true;
                        symbol = operandField.substr(1);
                    }
                    if (operandField.find(",X") != string::npos || operandField.find(",x") != string::npos)
                    {
                        isIndexed = true;
                        symbol = operandField.substr(0, operandField.find(","));
                    }
                }

                // Lookup symbol address
                int address = 0;
                if (!symbol.empty())
                {
                    if (symtab.contains(symbol))
                    {
                        address = symtab.getAddress(symbol);
                    }
                    else
                    {
                        cerr << "Error: Undefined symbol " << symbol << " in line: " << line << endl;
                        address = 0; // Or handle error appropriately
                    }
                }

                // Construct address with flags
                if (isImmediate)
                {
                    // Immediate addressing: no flags set
                    objCode = machineCode + intToHex(address, 4);
                }
                else
                {
                    // Direct addressing
                    if (isIndexed)
                    {
                        // Set indexed flag (bit 0)
                        // Assuming 16-bit address and indexed flag is the highest bit
                        address += 0x8000;
                    }
                    objCode = machineCode + intToHex(address, 4);
                }
            }

            // Ensure object code is exactly 6 hex digits
            if (objCode.length() < 6)
                objCode = string(6 - objCode.length(), '0') + objCode;
            else if (objCode.length() > 6)
                objCode = objCode.substr(0, 6);

            // Add to text records
            if (currentObjectCodes.empty())
            {
                currentTextAddress = currLocctrStr;
            }

            if (currentRecordLength + 3 > 30) // Max 30 bytes per text record
            {
                // Flush current text record
                string textRecord = "T " + currentTextAddress + " " + intToHex(currentRecordLength, 2);
                for (const auto &obj : currentObjectCodes)
                {
                    textRecord += " " + obj;
                }
                textRecords.push_back(textRecord);

                // Start new text record
                currentTextAddress = currLocctrStr;
                currentObjectCodes.clear();
                currentObjectCodes.push_back(objCode);
                currentRecordLength = 3;
            }
            else
            {
                currentObjectCodes.push_back(objCode);
                currentRecordLength += 3;
            }
        }
    }

    // After processing all lines, flush any remaining text record
    if (!currentObjectCodes.empty())
    {
        string textRecord = "T " + currentTextAddress + " " + intToHex(currentRecordLength, 2);
        for (const auto &obj : currentObjectCodes)
        {
            textRecord += " " + obj;
        }
        textRecords.push_back(textRecord);
    }

    // Write all Text Records first
    for (const auto &record : textRecords)
    {
        output << record << endl;
    }

    // Write the End Record after all Text Records
    if (!endRecord.empty())
    {
        output << endRecord << endl;
    }

    output.close();
}

// pass two has been modified with a little help of chatGPT