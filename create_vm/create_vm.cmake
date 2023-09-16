


if(NOT DEFINED KAUTIL_THIRD_PARTY_DIR)
    set(KAUTIL_THIRD_PARTY_DIR ${CMAKE_BINARY_DIR})
    file(MAKE_DIRECTORY "${KAUTIL_THIRD_PARTY_DIR}")
endif()

macro(git_clone url)
    get_filename_component(file_name ${url} NAME)
    if(NOT EXISTS ${KAUTIL_THIRD_PARTY_DIR}/kautil_cmake/${file_name})
        file(DOWNLOAD ${url} "${KAUTIL_THIRD_PARTY_DIR}/kautil_cmake/${file_name}")
    endif()
    include("${KAUTIL_THIRD_PARTY_DIR}/kautil_cmake/${file_name}")
    unset(file_name)
endmacro()


git_clone(https://raw.githubusercontent.com/kautils/CMakeLibrarytemplate/v0.0.1/CMakeLibrarytemplate.cmake)
git_clone(https://raw.githubusercontent.com/kautils/CMakeFetchKautilModule/v0.0.1/CMakeFetchKautilModule.cmake)

CMakeFetchKautilModule(kautil_jvm_info GIT https://github.com/kautils/jvm_info.git REMOTE origin TAG v0.0.1 )
find_package(KautilJniJvmInfo0.0.1.interface REQUIRED)

CMakeFetchKautilModule(sharedlib GIT https://github.com/kautils/sharedlib.git REMOTE origin TAG v0.0.1 )
find_package(KautilSharedlib.0.0.1.static REQUIRED)


set(module_name create_vm)
unset(srcs)
file(GLOB srcs ${CMAKE_CURRENT_LIST_DIR}/*.cc)
set(${module_name}_common_pref
    #DEBUG_VERBOSE
    MODULE_PREFIX kautil jni
    MODULE_NAME ${module_name}
    INCLUDES $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}> $<INSTALL_INTERFACE:include> 
    SOURCES ${srcs}
    LINK_LIBS 
        kautil::jni::jvm_info::0.0.1::interface
        kautil::sharedlib::0.0.1::static
    EXPORT_NAME_PREFIX ${PROJECT_NAME}
    EXPORT_VERSION ${PROJECT_VERSION}
    EXPORT_VERSION_COMPATIBILITY AnyNewerVersion
        
    DESTINATION_INCLUDE_DIR include/kautil/jni
    DESTINATION_CMAKE_DIR cmake
    DESTINATION_LIB_DIR lib
)


CMakeLibraryTemplate(${module_name} EXPORT_LIB_TYPE static ${${module_name}_common_pref} )

set(__t ${${module_name}_static_tmain})
add_executable(${__t})
target_sources(${__t} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/unit_test.cc)
target_link_libraries(${__t} PRIVATE ${${module_name}_static} kautil::jni::jvm_info::0.0.1::interface)
target_compile_definitions(${__t} PRIVATE ${${module_name}_static_tmain_ppcs})

