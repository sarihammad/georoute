add_executable(georoute_bench
    bench_routing.cpp
)

target_link_libraries(georoute_bench
    PRIVATE
        georoute_lib
)

add_executable(georoute_bench_main
    bench_main.cpp
)

target_link_libraries(georoute_bench_main
    PRIVATE
        georoute_lib
)

