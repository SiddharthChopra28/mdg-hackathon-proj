
cmake_minimum_required(VERSION 3.16)
project(OrionBackend CXX C)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/mem_events.bpf.o
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/ecc ${CMAKE_CURRENT_SOURCE_DIR}/mem_events.bpf.c
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/mem_events.bpf.c
    COMMENT "Compiling eBPF object mem_events.bpf.o with ecc"
)
add_custom_target(bpf_object DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/mem_events.bpf.o)

# ... (find_library and add_executable sections) ...
find_library(LIBBPF_LIBRARY NAMES bpf)
find_library(LIBELF_LIBRARY NAMES elf)
add_executable(ipc_server server/ipc_server.cpp)
add_executable(ram_logger ram_usage/rl_processor.cpp)
add_executable(pp_ram_log ram_detail/pp_ram_log.cpp)
add_executable(mem_ev_processor mem_ev_processor.cpp)

# ... (target_link_libraries sections) ...
target_link_libraries(mem_ev_processor PRIVATE ${LIBBPF_LIBRARY} ${LIBELF_LIBRARY})
target_link_libraries(ipc_server PRIVATE pthread)
target_link_libraries(ram_logger PRIVATE pthread)
target_link_libraries(pp_ram_log PRIVATE pthread)
target_link_libraries(mem_ev_processor PRIVATE pthread)

add_dependencies(mem_ev_processor bpf_object)

install(TARGETS
    ipc_server
    ram_logger
    pp_ram_log
    mem_ev_processor
    DESTINATION /usr/local/bin/orion
)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/mem_events.bpf.o DESTINATION /usr/local/bin/orion)

message(STATUS "CMake configuration complete. Run 'make' to build.")