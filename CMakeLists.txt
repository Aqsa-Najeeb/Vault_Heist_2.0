cmake_minimum_required(VERSION 3.28)
project(CMakeSFMLProject LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.1.0
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_Declare(nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_Declare(Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.7.1
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML nlohmann_json)

set(MODEL_NAMES BinaryTree BST AVL MaxHeap Validator EventLog)
set(MODEL_SOURCES "")
foreach(name ${MODEL_NAMES})
    if(EXISTS ${CMAKE_SOURCE_DIR}/src/${name}.cpp)
        list(APPEND MODEL_SOURCES src/${name}.cpp)
    endif()
endforeach()

file(GLOB GAME_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.cpp)
list(FILTER GAME_SOURCES EXCLUDE REGEX ".*/(test_|bench_).*")
foreach(name ${MODEL_NAMES})
    list(FILTER GAME_SOURCES EXCLUDE REGEX ".*/${name}\\.cpp$")
endforeach()

if(MODEL_SOURCES)
    add_library(vh_model STATIC ${MODEL_SOURCES})
    target_compile_features(vh_model PUBLIC cxx_std_17)
    target_include_directories(vh_model PUBLIC ${CMAKE_SOURCE_DIR}/src)
endif()

add_executable(main ${GAME_SOURCES})
target_compile_features(main PRIVATE cxx_std_17)
target_include_directories(main PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(main PRIVATE SFML::Graphics nlohmann_json::nlohmann_json)
if(MODEL_SOURCES)
    target_link_libraries(main PRIVATE vh_model)
endif()

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/test_*.cpp)
if(TEST_SOURCES AND MODEL_SOURCES)
    FetchContent_MakeAvailable(Catch2)
    add_executable(vh_tests ${TEST_SOURCES})
    target_link_libraries(vh_tests PRIVATE vh_model Catch2::Catch2WithMain)
    include(CTest)
    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
    include(Catch)
    catch_discover_tests(vh_tests)
endif()

file(GLOB BENCH_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/bench_*.cpp)
if(BENCH_SOURCES AND MODEL_SOURCES)
    add_executable(vh_bench ${BENCH_SOURCES})
    target_link_libraries(vh_bench PRIVATE vh_model)
    target_compile_options(vh_bench PRIVATE $<IF:$<CXX_COMPILER_ID:MSVC>,/O2,-O2>)
endif()

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT main)