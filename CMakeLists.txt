cmake_minimum_required(VERSION 3.5)
project(udger)

set(INC_DIR D:/user/include)
set(LINK_DIR D:/user/lib)
include_directories(${INC_DIR})
link_directories(${LINK_DIR})

set(CMKE_CXX_FLAGS"-std = c ++ 0x -Wno-deprecated")
set(CMAKE_CXX_STANDARD 11)
set(SOURCE_FILES main.cpp CppSQLite3.h CppSQLite3.cpp parser.cpp parser.h SQLQuery.h UaResult.h LRUCache.h WordDetector.h)
#set(SOURCE_FILES main.cpp WordDetector.h)
add_executable(udger ${SOURCE_FILES})

target_link_libraries(udger sqlite3)