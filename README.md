Project is an exploration project for c++. It implements mathematical syntax evaluator.

## How to use

- Clone
- Init CMake `cmake -S . -B ./build/release -DCMAKE_BUILD_TYPE=Release && cmake -S . -B ./build/debug -DCMAKE_BUILD_TYPE=Debug`
- Build `cmake --build ./build/release`
- Run `./build/release/main`

## Functionality

- Program expects input after launch. It can accept either known command or a math formula
- Known commands include
    - `${string}` - evaluates math expression
    - `:tree ${string}` - parses string and prints AST
    - `:tkn ${string}` - parses string and outputs tokens
    - `:ep ${string}` - evaluates string with `pure` implementation
    - `:ef ${string}` - evaluates string with `flat` implementation
    - `:el ${string}` - evaluates string with `linked` implementation
    - `:perf ${int}` - tests performance of each implementations of evaluators, accepts size of generated token sequence
    - `:perf_safe ${int}` - like `:perf`, but does not invoke `pure` implementation, as it might exceed the call stack on big numbers
    - `:exit` - normal finish
- Math Syntax is specified as following:
    - Sybmols: integer, double or variable
    - Binary Operators:
        - Arithmetic Operations: +, -, *, /, %, ^, does what you expect it to
        - Comparison Operations: >, <, >=, <=, !=, ==, does what you expect it to
        - Assignment Operation: =, defines variable in lvalue if undefined, assignes rvalue to lvalue, returns lvalue
        - Execution order is intuitive: (^), (%, *, /), (+, -), (=), (>, <, >=, <=, ==, !=)
    - Perfix Operators: - and +
    - Priority Brackets ()
    - Predefined functions, syntax is like `log(2, 2 ^ 3)`
        - `int(...1)`, `real(...1)`
        - `log(...2)`, `log2(...1)`, `log10(...1)`, `ln(...)`
        - `sin(...1)`, `cos(...1)`, `tan(...1)`, `ctan(...1)`
        - `if(...)` - retuns first non-zero value
        - `ife(...3)` - if (`$1`) `$2` else `$3`
- Syntax is evaluated from left to right, all statements are evaluated

## Project Structure

- `src/algo/` - just general stuff
- `src/sxm/parser/` - tokenisation
- `src/sxm/matcher/` - patterns for tokeniser
- `src/sxm/debug/` - just some debug directives
- `src/sxm/cmd/` - functions for command execution
- `src/sxm/st/` - ast related utilities
- `src/sxm/eval/` - evaluators

## Implementations

- Project implements three ways of evaluating the tree
- Pure Evaluation
    - Uses separate allocations for AST nodes
    - AST is separate from evaluation `eval_pure_build` builds `AST`, `eval_pure` evaluates it
    - Deeply nested, will overflow callstack on deletion
- Linked Evaluation
    - Uses separate allocations for AST nodes
    - AST is evaluated during build, folding finished expressions
- Flat Evaluation
    - Operates on the `malloc`'ed batch of memory instead of relying on individual allocations
    - Folds finished expressions
