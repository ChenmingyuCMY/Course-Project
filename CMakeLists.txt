cmake_minimum_required(VERSION 3.16)
project(2DGame VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置Qt6的安装路径（根据您的实际情况修改）
set(Qt6_DIR "D:/Qt/6.10.1/mingw_64/lib/cmake/Qt6/" CACHE PATH "Path to Qt6 CMake config")
set(CMAKE_PREFIX_PATH ${Qt6_DIR} ${CMAKE_PREFIX_PATH})

# 查找Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets OpenGL OpenGLWidgets)

# 查找OpenGL
find_package(OpenGL REQUIRED)

# 打印找到的Qt信息
message(STATUS "Found Qt ${Qt6_VERSION} at ${Qt6_DIR}")
message(STATUS "Qt include directories: ${Qt6Core_INCLUDE_DIRS}")

# 启用MOC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 设置源文件
set(SOURCES
    src/main.cpp
    src/BaseRenderer.cpp
    src/ShaderManager.cpp
    src/GameWindow.cpp
    src/StartScreen.cpp
    src/MapScreen.cpp
    src/GameScreen.cpp
    src/BossScene.cpp
)

set(HEADERS
    src/BaseRenderer.h
    src/ShaderManager.h
    src/GameWindow.h
    src/StartScreen.h
    src/MapScreen.h
    src/GameScreen.h
    src/BossScene.h
)

# 添加可执行文件
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# 设置目标属性
target_include_directories(${PROJECT_NAME} PRIVATE src)

# 设置预处理器定义（如果需要）
target_compile_definitions(${PROJECT_NAME} PRIVATE 
    QT_CORE_LIB
    QT_WIDGETS_LIB
    QT_OPENGL_LIB
)

# 链接库
target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    OpenGL::GL
)

# 复制着色器和字体文件夹
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders
)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/vertex_shader.glsl
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders/
)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/fragment_shader.glsl
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders/
)