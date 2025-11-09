add_executable(georoute_bench
    bench_routing.cpp
)

target_link_libraries(georoute_bench
    PRIVATE
        georoute_lib
)

