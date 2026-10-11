cmake_minimum_required(VERSION 3.14)

project(maty_server LANGUAGES CXX C)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Sql Network WebSockets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Sql Network WebSockets)
find_package(Threads)

# Crypt
set (CRYPT_DIR "lib/crypt")
file (GLOB CRYPT_SRC "${CRYPT_DIR}/*.h" "${CRYPT_DIR}/*.c")
source_group(${CRYPT_DIR} FILES ${CRYPT_SRC})
include_directories (${CRYPT_DIR})

# Sources
include_directories("src")
file(GLOB_RECURSE SOURCES "src/*.h" "src/*.cpp" "src/*.cc")
source_group("src" FILES ${SOURCES})
set (SOURCES ${SOURCES} ${CRYPT_SRC})

# Executable
add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Sql
Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::WebSockets Threads::Threads)

