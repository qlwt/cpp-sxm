# Mathematical Expression Evaluator

Project is an exploration of basic string parsing and evaluation in c++.

## Build

```bash
git clone <repository>
cmake -S . -B build/debug -DCMAKE_BUILD_TYPE=Debug
cmake -S . -B build/release -DCMAKE_BUILD_TYPE=Release
cmake --build build/release
```

To vendor the project, copy `vendor/sxm` and integrate it into your CMake configuration.

## Implementations

The project explores four evaluation strategies:

| Implementation | Description |
|---|---|
| `flat` | Evaluates expressions using flat memory. General-purpose, one-time evaluation. |
| `linked` | Builds an IR using individually allocated nodes. General-purpose, one-time evaluation. |
| `pure` | Builds a reusable AST, separating parsing from evaluation. Exposes the tree for inspection. |
| `compile` | Compiles expressions into bytecode for repeated evaluation with changing variable values. |

## Expression Syntax

**Literals and operators**

- Numbers: `1`, `-1`, `1e5`, `+1.31`
- Parentheses: `3 * (1 + 1)`
- Variables and assignments: `pi`, `n = 31`
- Binary operators, ordered by precedence:
  1. Power: `^`
  2. Multiplication, division, modulo: `*`, `/`, `%`
  3. Addition, subtraction: `+`, `-`
  4. Assignment: `=`
  5. Logical: `>`, `>=`, `<`, `<=`, `==`, `!=`, `&&`, `||`

**Built-in Functions**

| Function | Arguments | Description |
|---|---:|---|
| `trunc`, `floor`, `ceil`, `round` | 1 | Rounding and integer-part operations |
| `min`, `max` | 1+ | Minimum and maximum |
| `log` | 2 | Logarithm with a specified base |
| `log2`, `log10`, `ln` | 1 | Logarithms |
| `sin`, `asin`, `cos`, `acos`, `tan`, `atan`, `ctan`, `actan` | 1 | Trigonometric functions |
| `if` | 1+ | Returns the first non-zero argument |
| `ife` | 3 | Returns the second argument if the first is non-zero; otherwise, the third |
| `prepare` | 1+ | Returns the last argument |
| `effect` | 1+ | Returns the first argument |

All arguments are evaluated, including those not used in the returned result.

Examples:

```cpp
n = !30 && 5                 // 0 (n = !30 evaluated first)
prepare(n = 3, b = 8, n * b) // 24
3 * (1 + 1)                  // 6
3 + 3 * 3                    // 12
-log(2, 2^10)                // -10
ife(n = 5 * 3 > 3, n, 0)     // 15
```

## Interactive CLI

When built as a standalone project, SXM provides an interactive CLI for evaluating expressions, inspecting intermediate representations, and benchmarking the implementations.

| Command | Description |
|---|---|
| `{expr}` | Evaluate an expression |
| `:tree {expr}` | Parse and print the AST |
| `:tkn {expr}` | Print expression tokens |
| `:bytecode {expr}` | Compile and print bytecode (`l` runtime variable prevents compile-time evaluation) |
| `:ep {expr}` | Evaluate using `pure` |
| `:ef {expr}` | Evaluate using `flat` |
| `:el {expr}` | Evaluate using `linked` |
| `:ec {expr}` | Evaluate using `compile` |
| `:perf {int}` | Benchmark one-time evaluation |
| `:perf_safe {int}` | Same benchmark, excluding `pure` |
| `:perfsm {int}*{int}` | Benchmark repeated evaluation (`input_size` × `repeats`) |
| `:exit` | Exit the CLI |

Benchmarks generate expressions of the specified size. `:perf_safe` excludes `pure`, which may exhaust the call stack on large inputs.

## API Overview

The implementations are exposed through separate namespaces:

- `sxm::feval` — flat evaluator
- `sxm::leval` — linked evaluator
- `sxm::peval` — AST-based evaluator
- `sxm::ceval` — bytecode compiler and evaluator

The `flat`, `linked`, and `pure` implementations provide two evaluation modes:

- `eval` preserves the original variable map.
- `eval_mut` uses the provided variable map for intermediate assignments, allowing expressions such as `pi = 2` to modify it.

### Flat

```cpp
const char* src = "pi + 1";

sxm::HMapStatic<255, sxm::NumValue> vars{};
vars.insert({ "pi" }, { 3.14 });

auto result = sxm::feval::eval(src, {
    .varmap = vars,
});
```

### Linked

```cpp
const char* src = "1 + 1";

// Variables are optional.
auto result = sxm::leval::eval(src);
```

### Pure

Separates AST construction from evaluation, allowing the same tree to be inspected and evaluated multiple times.

AST nodes are not automatically destroyed and must be manually deleted.

```cpp
const char* src = "1 + 1";

auto ast = sxm::peval::build(src);

if (ast.has_value()) {
    // An explicit empty variable map can also be provided.
    auto result = sxm::peval::eval(*ast, {
        .varmap = sxm::SMapNull<sxm::NumValue>(),
    });

    delete *ast;
}
```

### Compile

The compile implementation (`sxm::ceval`) generates bytecode, separating expression compilation from execution.

It provides two main types:

- **`Declaration`** — produced by `build`. Holds compiled bytecode and can be evaluated repeatedly with different state values.
- **`Unit`** — created through `Declaration::init()`. Holds allocated memory and resolved pointers, reducing per-evaluation overhead, especially for small expressions.

A `Unit` depends on its originating `Declaration` and must not outlive it. It also contains shared mutable state and cannot be evaluated concurrently.

```cpp
const char* src = "1 + 1";

// Global variables: names mapped to numeric values.
// Resolved during compilation.
sxm::HMapStatic<255, sxm::NumValue> varmap_global{};
varmap_global.insert({ "pi" }, { 3.14 });

// Runtime variables: names mapped to state offsets.
// Values must be provided before execution.
sxm::HMapStatic<255, sxm::ceval::Offset> varmap_state{};
varmap_state.insert({ "time" }, { 0 });

auto decl = sxm::ceval::build(src, {
    .varmap_const = varmap_global,
    .varmap_state = varmap_state,
    .varmap_state_size = 1,
});

if (decl.has_value()) {
    // Evaluate with an initializer list.
    auto result = decl->eval({ { 0.0 } });

    // Or with an array.
    sxm::NumValue state[1] = { { 0.0 } };
    result = decl->eval({ state, state + 1 });

    // Initialize a reusable execution unit.
    auto unit = decl->init();
    unit.state_list[0] = 0.0;
    result = unit.eval();
}
```

## Custom Functions

Built-in function definitions are hardcoded and stored in `sxm::numfn::fnmap` and you can add global function by extending initialisation.
Custom functions are defined separately for each supported IR representation. A local function map can then be composed with the built-in map and passed to an evaluator.

```cpp
// Function for compiled and pure IR.
sxm::NumValue double_fn_raw(Range<sxm::NumValue> arg_list) noexcept {
    return { arg_list.p_start->real * 2 };
}

// Function for flat IR.
sxm::NumValue double_fn_feval(Range<sxm::feval::NodeNum*> arg_list) noexcept {
    return { (*arg_list.p_start).value->real * 2 };
}

// Function for linked IR.
sxm::NumValue double_fn_leval(Range<sxm::leval::NodeNum*> arg_list) noexcept {
    return { (*arg_list.p_start).value->real * 2 };
}

// Validate the number of arguments.
bool double_validator(std::size_t arg_size) noexcept {
    return arg_size == 1;
}

Def double_def {
    double_fn_raw,
    double_fn_feval,
    double_fn_leval,
    double_validator,
};

sxm::HMapStatic<32, sxm::numfn::Def> fnmap_local{};
fnmap_local.insert({ "double" }, double_def);

// Combine built-in and local function definitions.
sxm::HMapFallback<sxm::numfn::Def> fnmap_composed{
    sxm::numfn::fnmap,
    fnmap_local,
};

const char* src = "double(2)";

auto result = sxm::feval::eval(src, {
    .varmap = sxm::SMapNull<sxm::NumValue>(),
    .fnmap = fnmap_composed,
});
```
