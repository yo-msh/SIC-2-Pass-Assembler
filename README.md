# SIC 2-Pass Assembler

This project implements a **2-pass assembler** for the Simplified Instructional Computer (SIC). It takes an assembly language program as input and generates the corresponding object program and intermediate files. 

---

## **Features**

- Implements the **2-pass assembly process**:
  1. **Pass 1**: Constructs the symbol table and calculates memory addresses.
  2. **Pass 2**: Generates the object code.
- Reads assembly language input from a text file.
- Produces:
  - An **intermediate file** with addresses and object codes.
  - An **object program** ready for execution.
- **Error handling** for invalid opcodes, symbols, and formats.

---

### Prerequisites
- A C++ compiler (e.g., GCC).
- Basic understanding of SIC assembly language.

### Clone the Repository
```bash
git clone https://github.com/yo-msh/SIC-2-Pass-Assembler.git
cd SIC-2-Pass-Assembler
```

- While changing the **"input.txt"** make sure to add all the correspoding opcodes in **"opcode.cpp"** 

### To Run
```cpp
g++ -o main.cpp
./main
```

