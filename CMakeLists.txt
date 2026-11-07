cmake_minimum_required(VERSION 3.30)

project(main)

set(CMAKE_EXPORT_COMPILE_COMMANDS on)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_EXTENSIONS off)

add_executable(main
    src/main.cpp

    src/sxm/parser/parse.cpp
    src/sxm/matcher/pattern_bop.cpp
    src/sxm/matcher/pattern_num.cpp
    src/sxm/matcher/pattern_ref.cpp
    src/sxm/matcher/pattern_scope_end.cpp

    src/sxm/ast/uop.cpp
    src/sxm/ast/biop.cpp
    src/sxm/ast/fnmap.cpp
    src/sxm/ast/numvalue.cpp

    src/sxm/eval/pure.cpp
    src/sxm/eval/flat.cpp
    src/sxm/eval/linked.cpp
    src/sxm/eval/compile.cpp

    src/sxm/cmd/tkn.cpp
    src/sxm/cmd/tree.cpp
    src/sxm/cmd/perf.cpp
    src/sxm/cmd/perfsm.cpp
    src/sxm/cmd/assemble.cpp
    src/sxm/cmd/eval_pure.cpp
    src/sxm/cmd/eval_flat.cpp
    src/sxm/cmd/eval_linked.cpp
    src/sxm/cmd/eval_compile.cpp

    src/sxm/debug/print_ast.cpp
    src/sxm/debug/print_err_eval.cpp
    src/sxm/debug/print_err_parser.cpp
    src/sxm/debug/print_ast.cpp
    src/sxm/debug/tostr.cpp
    src/sxm/debug/valid_let.cpp
)

target_compile_options(main
    PRIVATE
        $<$<CONFIG:Debug>:-g>
        $<$<CONFIG:Debug>:-O1>
        $<$<CONFIG:Debug>:-Wall>
        $<$<CONFIG:Debug>:-Wextra>
        $<$<CONFIG:Debug>:-fsanitize=address,undefined>
)

target_link_options(main
    PRIVATE
        $<$<CONFIG:Debug>:-fsanitize=address,undefined>
)

target_compile_options(main
    PRIVATE
        $<$<CONFIG:Release>:-flto=auto>
        $<$<CONFIG:Release>:-O3>
)

target_link_options(main
    PRIVATE
        $<$<CONFIG:Release>:-flto=auto>
)

target_include_directories(main
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
