# C Style Toy Compiler

## Overview
This project is a toy compiler designed to demonstrate the components of a basic compiler. The project includes:

1. **Lexical Analysis**: Tokenizes the source code into meaningful symbols.
2. **Syntax Analysis**: Parses tokens into an Abstract Syntax Tree (AST).
3. **Semantic Analysis**: Ensures correctness of code semantics and detects type errors.
4. **Intermediate Code Generation (ICG)**: Generates intermediate representation, such as Three-Address Code (TAC), for further optimizations or machine code generation.

## Features

- **Symbol Table**: Supports variable/function declaration and scope management.
- **AST**: Represents the program structure for efficient semantic analysis and code generation.
- **Semantic Analysis**: type checking, duplicate declaration detection, and function validation.
- **ICG**: Produces Three-Address Code (TAC) for arithmetic, logical, and control statements.

## Components

### 1. Lexer (`lexer.l`)
The lexical analyzer uses a set of rules to convert the source code into tokens. These tokens are fed into the parser for syntax analysis.

### 2. Parser (`parser.y`)
The parser builds the Abstract Syntax Tree (AST) from the tokens using grammar rules.

### 3. Abstract Syntax Tree (AST)
<img width="1258" alt="Screenshot 2024-12-31 at 5 31 59 PM" src="https://github.com/user-attachments/assets/ea3bb4ec-bb55-4ac4-8bd9-91bb6398704b" />
<img width="1280" alt="Screenshot 2024-12-31 at 5 28 49 PM" src="https://github.com/user-attachments/assets/b7a4c2aa-7a73-4aae-9f2c-73b456fb0686" />
<img width="1280" alt="Screenshot 2024-12-31 at 5 29 38 PM" src="https://github.com/user-attachments/assets/60e84cb8-5f98-4c16-bf4e-788165573ec0" />
<img width="791" alt="Screenshot 2024-12-31 at 5 31 13 PM" src="https://github.com/user-attachments/assets/103f8975-a03f-4eee-bb6f-1c420322c9da" />

- **Files**: `ast.c`, `ast.h`
- **Key Features**:
  - Hierarchical representation of the source code structure.
  - Nodes for various constructs like declarations, expressions, and loops.
  - Includes functionality for traversal, visualization (JSON export), and memory management.

### 4. Symbol Table
<img width="962" alt="Screenshot 2024-12-31 at 5 37 34 PM" src="https://github.com/user-attachments/assets/80180998-6dda-4b1e-ac29-c84805b9549b" />

- **Files**: `symTable.c`, `symTable.h`
- **Key Features**:
  - Manages variable/function declarations.
  - Handles scope chaining (global and local).
  - Detects duplicate declarations and undeclared variables.

### 5. Semantic Analysis
<img width="842" alt="Screenshot 2024-12-31 at 5 31 34 PM" src="https://github.com/user-attachments/assets/62b66572-821a-4263-b9d9-b0a3d2c737e4" />
<img width="842" alt="Screenshot 2024-12-31 at 5 37 12 PM" src="https://github.com/user-attachments/assets/47490b37-0e8c-4552-b56c-caf1c35e86c5" />
<img width="842" alt="Screenshot 2024-12-31 at 5 39 11 PM" src="https://github.com/user-attachments/assets/ceeffd0f-e0b5-458e-b05b-58a702c20ebf" />


- **Files**: `semantic.c`, `semantic.h`
- **Key Features**:
  - Validates symbol usage, types, and return types.
  - Supports type promotion and compatibility checks.
  - Supports [Hoisting](https://developer.mozilla.org/en-US/docs/Glossary/Hoisting) (like in JavaScript) allowing variables usage before their declaration.
  - Ensures proper usage of control statements like `break` and `continue`.

### 6. Intermediate Code Generation (ICG)

##### Sample Input Code
```c
int a = 1, b = 2;
for(; ; );
while(2 == 2 && 1 <= 3 != 4);

if(a && 3) a = 3;
```
##### Three Address Code
<img width="740" alt="Screenshot 2024-12-31 at 5 40 24 PM" src="https://github.com/user-attachments/assets/80577f1c-4faa-44c3-8c3b-bf564f7b5f52" />


- **Files**: `icg.c`, `icg.h`
- **Key Features**:
  - Generates TAC for expressions, conditionals, and loops.
  - Handles logical and arithmetic operations.
  - Boolean short-circuit code generation uisng [Backpatching](https://www.geeksforgeeks.org/backpatching-in-compiler-design/)
  - > Note: TAC generation for functions is not implemented yet.

## Getting Started

### Prerequisites

- GCC or any compatible C compiler.
- Yacc (or Bison) for parsing.
- Flex (or Lex) for lexical analysis.
- Make (optional for build automation).

### Building the Compiler

1. Clone this repository.
2. Generate the lexer and parser C files (lex.yy.c, y.tab.c, y.tab.h)
    ```bash
    lex lexer.l && yacc -d parser.y
    ```
3. Compile the source files:
   ```bash
   gcc symTable.c ast.c semantic.c icg.c lex.yy.c y.tab.c -ll -ly
   ```

### Running the Compiler

1. Provide an input file containing source code written in the supported syntax.
2. Runtime flags:

- `--export-ast`: Exports the Abstract Syntax Tree (AST) to a JSON file.

- `--print-ast`: Prints the AST structure to the console.

- `--print-sym-table`: Prints the symbol table.

- `--debug`: Enables all debugging modes.

- `--debug-ast`: Enables debugging for AST operations.

- `--debug-semantic`: Enables debugging for semantic analysis.

- `--debug-icg`: Enables debugging for intermediate code generation.

3. Run the compiler:
   ```bash
   ./a.out<input.c
   ```
4. View the generated output such as TAC or AST visualization.

### Output

- **AST Visualization**: The compiler generates a JSON file `ast.json` and an HTML `index.html` in `AST_Vis/` folder. Run the HTML file to visualize the AST.
- **TAC**: Outputs the intermediate representation for further optimization or code generation.

## File Structure

- **`lexer.l`**: Lexical analyzer.
- **`parser.y`**: Syntax analyzer.
- **`symTable.c`**, **`symTable.h`**: Implementation of the symbol table.
- **`semantic.c`**, **`semantic.h`**: Implementation of semantic analysis.
- **`ast.c`**, **`ast.h`**: AST structures and operations.
- **`icg.c`**, **`icg.h`**: Intermediate code generation.


## License

This project is licensed under the MIT License. See the LICENSE file for details.

## Acknowledgments

This project is inspired by concepts taught in compiler design courses and the book *Compilers: Principles, Techniques, and Tools* by Alfred V. Aho, Monica S. Lam, Ravi Sethi, and Jeffrey D. Ullman (Dragon Book). It serves as a foundation for understanding the structure and functionality of modern compilers.

