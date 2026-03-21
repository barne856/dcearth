cmake_minimum_required(VERSION 3.14)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(dcearth LANGUAGES CXX)

include(kallistios)
include(FetchContent)

FetchContent_Declare(
    squint
    GIT_REPOSITORY https://github.com/barne856/squint.git
    GIT_TAG        main
)

set(SQUINT_BLAS_BACKEND "NONE" CACHE STRING "" FORCE)
set(SQUINT_USE_CUDA OFF CACHE BOOL "" FORCE)
set(SQUINT_USE_AVX2 OFF CACHE BOOL "" FORCE)
set(SQUINT_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(SQUINT_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(squint)
get_target_property(SQUINT_INC SQUINT INTERFACE_INCLUDE_DIRECTORIES)
set_target_properties(SQUINT PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${SQUINT_INC}")

add_executable(dcearth src/main.cpp)
target_include_directories(dcearth PRIVATE include)
target_link_libraries(dcearth PRIVATE kosutils SQUINT::SQUINT)
target_compile_features(dcearth PRIVATE cxx_std_23)

set(DC_IP "192.168.1.118" CACHE STRING "Dreamcast IP address for dc-tool-ip")

add_custom_target(run
    COMMAND dc-tool-ip -t ${DC_IP} -x $<TARGET_FILE:dcearth>
    DEPENDS dcearth
    USES_TERMINAL
    COMMENT "Uploading to Dreamcast at ${DC_IP}"
)

add_custom_target(disc
    COMMAND mkdcdisc -e $<TARGET_FILE:dcearth> -o ${CMAKE_SOURCE_DIR}/dcearth.cdi
    DEPENDS dcearth
    COMMENT "Creating CDI disc image"
)
