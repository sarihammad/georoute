add_executable(georoute_tests
    test_placeholder.cpp
    test_dijkstra.cpp
    test_segment_tree.cpp
    test_router.cpp
)

target_link_libraries(georoute_tests
    PRIVATE
        georoute_lib
        Catch2::Catch2WithMain
)

add_test(NAME georoute_placeholder_tests COMMAND georoute_tests)

