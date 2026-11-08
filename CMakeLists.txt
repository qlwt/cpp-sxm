cmake_minimum_required(VERSION 3.30)

project(main)

set(CMAKE_EXPORT_COMPILE_COMMANDS on)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_EXTENSIONS off)

add_executable(main
    src/main.cpp

    src/cmd/tkn.cpp
    src/cmd/tree.cpp
    src/cmd/perf.cpp
    src/cmd/perfsm.cpp
    src/cmd/bytecode.cpp
    src/cmd/eval_pure.cpp
    src/cmd/eval_flat.cpp
    src/cmd/eval_linked.cpp
    src/cmd/eval_compile.cpp
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

add_subdirectory(vendor/sxm)

target_link_libraries(main
    PRIVATE
        sxm::sxm
)

target_include_directories(main
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
