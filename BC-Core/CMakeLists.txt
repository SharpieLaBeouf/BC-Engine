file(GLOB_RECURSE ENGINE_SRC
    CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.hpp
)

add_library(BCEngineCore STATIC ${ENGINE_SRC})

set(CMAKE_CXX_STANDARD 20)

target_precompile_headers(BCEngineCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Source/BC_PCH.h)

if(ENABLE_IPO)
    set_property(TARGET BCEngineCore PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

target_compile_definitions(BCEngineCore PUBLIC 
    GLFW_INCLUDE_NONE
    GLFW_INCLUDE_VULKAN
    PX_PHYSX_STATIC_LIB
    GLM_ENABLE_EXPERIMENTAL
    IMGUI_DEFINE_MATH_OPERATORS
    YAML_CPP_STATIC_DEFINE
)

if(MSVC)
    target_compile_options(BCEngineCore PRIVATE /utf-8)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(BCEngineCore PRIVATE -finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

target_include_directories(BCEngineCore
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/Source

        ${PROJECT_SOURCE_DIR}/External/assimp/include
        ${PROJECT_SOURCE_DIR}/External/efsw/include
        ${PROJECT_SOURCE_DIR}/External/entt/src
        ${PROJECT_SOURCE_DIR}/External/glfw/include
        ${PROJECT_SOURCE_DIR}/External/glm/glm
        ${PROJECT_SOURCE_DIR}/External/imgui
        ${PROJECT_SOURCE_DIR}/External/imguizmo
        ${PROJECT_SOURCE_DIR}/External/imgui-node-editor
        ${PROJECT_SOURCE_DIR}/External/miniaudio
        ${PROJECT_SOURCE_DIR}/External/spdlog/include
        ${PROJECT_SOURCE_DIR}/External/stb
        ${PROJECT_SOURCE_DIR}/External/tinyfiledialogs
        ${PROJECT_SOURCE_DIR}/External/yaml-cpp/include
        
        ${PHYSX_INCLUDE_DIR}

        ${LIBCLANG_INCLUDE_DIR}
        ${Vulkan_INCLUDE_DIR}
)

if(UNIX AND NOT APPLE)
    target_link_libraries(BCEngineCore PUBLIC tinyfiledialogs)
endif()

target_link_libraries(BCEngineCore
    PUBLIC
        EnTT
        glfw
        assimp
        efsw
        imgui
        imguizmo
        imgui_node_editor
        miniaudio
        spdlog
        glm
        yaml-cpp

        #physx
        $<$<CONFIG:Debug>:${PHYSX_DEBUG_LIBS}>
        $<$<CONFIG:Release>:${PHYSX_RELEASE_LIBS}>

        ${LIBCLANG_LIB_PATH}
        ${Vulkan_LIBRARY}
)

if(UNIX)

    target_link_libraries(BCEngineCore 
        PUBLIC
            pthread
            dl
            m
    )

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    
        message("Debug Configuration")
        add_compile_definitions(_DEBUG)
        target_link_libraries(BCEngineCore 
            PUBLIC
                ${PHYSX_DEBUG_LIBS}
        )

    else()

        message("Release Configuration")
        add_compile_definitions(NDEBUG)
        target_link_libraries(BCEngineCore 
            PUBLIC
                ${PHYSX_RELEASE_LIBS}
        )

    endif()

endif()

set_target_properties(BCEngineCore PROPERTIES FOLDER "BC-Core")