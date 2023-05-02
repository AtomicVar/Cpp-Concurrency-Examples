cmake_minimum_required(VERSION 3.22)

# 设置项目名称
project(CppConcurrentExamples)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 添加可执行文件
add_executable(thread_pool thread_pool/main.cc)

# 添加头文件搜索路径
include_directories(3rdparty/spdlog/include)

# 第三方库
add_subdirectory(3rdparty/spdlog)

# 链接库
target_link_libraries(thread_pool spdlog::spdlog)

# 设置编译选项
add_compile_options(-Wall -Wextra -Wpedantic)

# 安装目标文件和库
# install(TARGETS MyProject DESTINATION bin)
# install(TARGETS mylibrary DESTINATION lib)
# install(DIRECTORY include/ DESTINATION include)

# 添加测试目录
# enable_testing()
# add_subdirectory(tests)