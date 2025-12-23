cmake_minimum_required(VERSION 3.20)
project(search LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ZLIB REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(MONGOCXX REQUIRED libmongocxx)
pkg_check_modules(BSONCXX REQUIRED libbsoncxx)
pkg_check_modules(GUMBO REQUIRED gumbo)

# -------------------
# parser library
# -------------------
add_library(parser libs/parser/parser.cpp libs/parser/parser.hpp)

target_include_directories(parser
    PUBLIC
        ${GUMBO_INCLUDE_DIRS}
)

target_link_libraries(parser
    PUBLIC
        ${GUMBO_LIBRARIES}
)

target_link_directories(parser
    PUBLIC
        ${GUMBO_LIBRARY_DIRS}
)

# -------------------
# db library
# -------------------
add_library(db libs/db/db.cpp libs/db/db.hpp)

target_include_directories(db
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${MONGOCXX_INCLUDE_DIRS}
        ${BSONCXX_INCLUDE_DIRS}
)

target_link_libraries(db
    PUBLIC
        ${MONGOCXX_LIBRARIES}
        ${BSONCXX_LIBRARIES}
)

target_link_directories(db
    PUBLIC
        ${MONGOCXX_LIBRARY_DIRS}
        ${BSONCXX_LIBRARY_DIRS}
)

# -------------------
# indexer library
# -------------------
add_library(indexer libs/indexer/indexer.cpp libs/indexer/indexer.hpp)

target_include_directories(indexer
    PUBLIC
    /usr/local/opt/wordnet/include
)

target_link_libraries(indexer 
    PUBLIC
    /usr/local/opt/wordnet/lib/libWN.a
)

# -------------------
# TF-IDF indexer library
# -------------------
add_library(tf-idf_indexer libs/tf-idf_indexer/tf-idf_indexer.cpp libs/tf-idf_indexer/tf-idf_indexer.hpp)

target_include_directories(tf-idf_indexer
    PUBLIC
    /usr/local/opt/wordnet/include
)

target_link_libraries(tf-idf_indexer
    PUBLIC
    ZLIB::ZLIB
    /usr/local/opt/wordnet/lib/libWN.a
)


# -------------------
# functions library
# -------------------
add_library(functions libs/functions/functions.hpp libs/functions/functions.cpp)

target_link_libraries(functions
    PUBLIC
        ZLIB::ZLIB
)

# -------------------
# scorer library
# -------------------
add_library(scorer libs/scorer/scorer.hpp libs/scorer/scorer.cpp)

target_link_libraries(scorer
    PUBLIC
        tf-idf_indexer
)


# -------------------
# tokenization executable
# -------------------
add_executable(tokenization tokenization.cpp)

target_link_libraries(tokenization
    PRIVATE
        db
        parser
        functions
)

# -------------------
# index_builder executable
# -------------------

add_executable(index_builder index_builder.cpp)

target_link_libraries(index_builder
    PRIVATE
        indexer
        db
)

# -------------------
# search executable
# -------------------

add_executable(search search.cpp)

target_link_libraries(search
    PRIVATE
        indexer
        db
        functions
)

# -------------------
# TF IDF index_builder executable
# -------------------

add_executable(tf-idf_index_builder tf-idf_index_builder.cpp)

target_link_libraries(tf-idf_index_builder
    PRIVATE
        tf-idf_indexer
        db
)

# -------------------
#  TF-IDF search executable
# -------------------

add_executable(tf-idf_search tf-idf_search.cpp)

target_link_libraries(tf-idf_search
    PUBLIC
        tf-idf_indexer
        db
        functions
        scorer
)


# -------------------
# tests
# -------------------
enable_testing()
add_subdirectory(tests)
