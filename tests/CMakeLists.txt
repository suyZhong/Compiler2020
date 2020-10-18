add_subdirectory(lab1)
add_subdirectory(lab2)

add_executable(test_ast test_ast.cpp)
target_link_libraries(test_ast syntax common)
