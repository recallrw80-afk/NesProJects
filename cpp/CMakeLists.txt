# 指定最低 CMake 版本
cmake_minimum_required(VERSION 3.12)
# 定义项目名称、版本、语言
project(Nes VERSION 1.0.0 DESCRIPTION "Nes" LANGUAGES CXX)
# 设置 C++ 标准为 17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)      # 必须支持 C++17
set(CMAKE_CXX_EXTENSIONS OFF)            # 禁用编译器扩展（使用标准 ISO C++）
# ============================================================================

# 添加源代码目录
# 添加源文件
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.cc" "src/*.c")
add_executable(${PROJECT_NAME} ${SOURCES} main.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 构建类型配置 (Debug / Release)

# 如果未指定构建类型，默认为 Release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING 
        "Choose the type of build: Debug Release RelWithDebInfo MinSizeRel" FORCE)
endif()

# 设置不同构建类型的编译选项
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os -DNDEBUG" CACHE STRING "" FORCE)

# ============================================================================
# 编译选项（跨平台）

# 添加编译选项（根据编译器类型）
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    # GCC 或 Clang 通用选项
    add_compile_options(
        -Wall                    # 启用大部分警告
        -Wextra                  # 启用额外警告
        -Wpedantic              # 严格遵循标准
        -Werror                 # 将警告视为错误（可选，开发时建议开启）
        -Wshadow                # 变量覆盖警告
        -Wconversion            # 隐式类型转换警告
    )
    
    # Release 模式额外优化
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(-flto)  # 链接时优化
    endif()
    
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # MSVC 选项
    add_compile_options(
        /W4                     # 警告等级 4
        /WX                     # 将警告视为错误
        /utf-8                  # 使用 UTF-8 编码
    )
endif()

# ============================================================================
# 输出目录设置

# 设置可执行文件和库文件的输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 为不同构建类型设置子目录
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/bin/${OUTPUTCONFIG})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/lib/${OUTPUTCONFIG})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/lib/${OUTPUTCONFIG})
endforeach()
# ============================================================================

# ============================================================================
# 安装配置

# 安装路径（默认 /usr/local）
include(GNUInstallDirs)

# 安装可执行文件
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# 安装头文件
install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)

# ============================================================================
# CPack 打包配置（可选）
# ============================================================================

include(CPack)

set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_VENDOR "Nes")
