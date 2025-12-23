include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(BUILD_GMOCK ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

# -------------------
# function tests
# -------------------
add_executable(function_tests test_functions.cpp)

target_link_libraries(function_tests
    PRIVATE
        gtest
        gmock
        functions
)

add_test(NAME function_tests COMMAND function_tests)

# -------------------
# indexer tests
# -------------------
add_executable(indexer_tests test_indexer.cpp)

target_link_libraries(indexer_tests
    PRIVATE
        gtest
        gmock
        indexer
)

add_test(NAME indexer_tests COMMAND indexer_tests)

# -------------------
# parser tests
# -------------------
add_executable(parser_tests test_parser.cpp)

target_link_libraries(parser_tests
    PRIVATE
        gtest
        gmock
        parser
)

add_test(NAME parser_tests COMMAND parser_tests)

# -------------------
# scorer tests
# -------------------
add_executable(scorer_test test_scorer.cpp)

target_link_libraries(scorer_test
    PRIVATE
        gtest
        gmock
        scorer
        tf-idf_indexer
)

add_test(NAME scorer_tests COMMAND scorer_test)

# -------------------
# tf-idf indexer tests
# -------------------
add_executable(tf-idf_indexer_test test_tf-idf_indexer.cpp)

target_link_libraries(tf-idf_indexer_test
    PRIVATE
        gtest
        gmock
        tf-idf_indexer
)

add_test(NAME tf-idf_indexer_tests COMMAND tf-idf_indexer_test)