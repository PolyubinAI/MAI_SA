cmake_minimum_required(VERSION 3.2)

project(user_service)

SET(BINARY "user_service")

find_package(Poco REQUIRED COMPONENTS Foundation Util Net JSON Crypto Data)

if(NOT ${Poco_FOUND})
    message(FATAL_ERROR "Poco C++ Libraries not found.")
endif()

include_directories(${Poco_INCLUDE_DIRS})

SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(
    ${BINARY}
    main.cpp
    handlers/user_handler.cpp
    ../config/config.cpp
    ../database/postgres.cpp
    ../database/user.cpp
)

target_include_directories(${BINARY} PRIVATE "${CMAKE_BINARY_DIR}")
target_compile_options(${BINARY} PRIVATE -Wall -Wextra -pedantic -Werror )
set_target_properties(${BINARY} PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(${BINARY} PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)

target_link_libraries(
    ${BINARY} PRIVATE 
    ${CMAKE_THREAD_LIBS_INIT} 
    ${Poco_LIBRARIES}
    "PocoDataPostgreSQL"
)




