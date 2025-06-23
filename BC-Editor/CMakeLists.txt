file(GLOB_RECURSE APP_SRC
    CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.hpp
)

add_executable(BCEngineEditor ${APP_SRC})

# Compile Options and Definitions
if(ENABLE_IPO)
    set_property(TARGET BCEngineEditor PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

if(UNIX)
    target_compile_options(BCEngineEditor PUBLIC $<$<CONFIG:Release>:-O3>)

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_definitions(_DEBUG)
    else()
        add_compile_definitions(NDEBUG)
    endif()

    set_target_properties(BCEngineEditor PROPERTIES
        INSTALL_RPATH "$ORIGIN"
    )
elseif(WIN32)
    target_compile_options(BCEngineEditor PUBLIC $<$<CONFIG:Release>:/O2>)
endif()

if(MSVC)
    target_compile_options(BCEngineEditor PRIVATE /utf-8)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(BCEngineEditor PRIVATE -finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

set(CMAKE_CXX_STANDARD 20)

target_compile_definitions(BCEngineCore PUBLIC 
    GLFW_INCLUDE_NONE
    GLFW_INCLUDE_VULKAN
    PX_PHYSX_STATIC_LIB
    GLM_ENABLE_EXPERIMENTAL
    IMGUI_DEFINE_MATH_OPERATORS
    YAML_CPP_STATIC_DEFINE
)

# Output
set_target_properties(BCEngineEditor PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    FOLDER "BC-Editor"
)

target_include_directories(BCEngineEditor 
    PRIVATE 
        "${PROJECT_SOURCE_DIR}/BC-Core/Source"
        "${PROJECT_SOURCE_DIR}/BC-Editor/Source"
)
target_link_libraries(BCEngineEditor PRIVATE BCEngineCore)

# ---------------------------------------------------------------------------
# Always copy Native Script Core source folder before building BCEngineEditor
# ---------------------------------------------------------------------------

set(SCRIPT_CORE_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../BC-Native-Script-Core/Source")
set(SCRIPT_CORE_DST "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Script Core API")

add_custom_target(copy_script_core ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${SCRIPT_CORE_SRC}" "${SCRIPT_CORE_DST}"
    COMMENT "Copying Native Script Core into Resources/Script Core API..."
)

add_dependencies(BCEngineEditor copy_script_core)