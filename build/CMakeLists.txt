cmake_minimum_required(VERSION 2.8.11)

set(bd ${CMAKE_CURRENT_LIST_DIR}/../)
set(sd ${bd}/src)

include_directories(
  ${bd}/include
  ${TINYLIB_DIR}/src
  ${EXTERN_INC_DIR}
  ${EXTERN_INC_DIR}/freetype2/
)

set(lib_sources
  ${sd}/FreetypeFont.cpp
  ${sd}/KankerApp.cpp
  ${sd}/KankerFont.cpp
  ${sd}/KankerGlyph.cpp
  ${sd}/KankerDrawer.cpp
)

if (APPLE) 
  find_library(fr_corefoundation CoreFoundation)
  find_library(fr_avfoundation AVFoundation)
  find_library(fr_cocoa Cocoa)
  find_library(fr_opengl OpenGL)
  find_library(fr_iokit IOKit)
  find_library(fr_corevideo CoreVideo)
  find_library(fr_coremedia CoreMedia)

  set(app_libs
    ${EXTERN_LIB_DIR}/libglfw3.a
    ${EXTERN_LIB_DIR}/libremoxly.a
    ${EXTERN_LIB_DIR}/libpng.a
    ${EXTERN_LIB_DIR}/libz.a
    ${EXTERN_LIB_DIR}/libfreetype.a
    ${fr_corefoundation}
    ${fr_avfoundation}
    ${fr_cocoa}
    ${fr_opengl}
    ${fr_iokit}  
    ${fr_corevideo}
    ${fr_coremedia}
    -lz
    -lbz2
    )
endif()

add_library(kanker ${lib_sources})
add_executable(kankerfont ${sd}/main.cpp ${EXTERN_SRC_DIR}/glad.c)
target_link_libraries(kankerfont ${app_libs} kanker)
install(TARGETS kankerfont RUNTIME DESTINATION bin)
install(TARGETS kanker ARCHIVE DESTINATION lib)
  
