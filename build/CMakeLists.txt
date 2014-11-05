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
  ${sd}/Blur.cpp
  ${sd}/BlurFBO.cpp
  ${sd}/FBO.cpp
  ${sd}/Ftp.cpp
  ${sd}/KankerApp.cpp
  ${sd}/KankerAbb.cpp
  ${sd}/KankerAbbController.cpp
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
    ${EXTERN_LIB_DIR}/libcurl.a
    ${fr_corefoundation}
    ${fr_avfoundation}
    ${fr_cocoa}
    ${fr_opengl}
    ${fr_iokit}  
    ${fr_corevideo}
    ${fr_coremedia}
    -lz
    -lbz2
    -lldap
    -lssl
    -lcrypto
    )
elseif(WIN32)
  set(app_libs
    ${EXTERN_LIB_DIR}/glfw3.lib
    ${EXTERN_LIB_DIR}/remoxly.lib
    ${EXTERN_LIB_DIR}/libpng16_static.lib
    ${EXTERN_LIB_DIR}/zlibstatic.lib
    ${EXTERN_LIB_DIR}/freetype.lib
    Opengl32.lib                                   
    ws2_32.lib                  # opengl / glfw
    psapi.lib                   # opengl / glfw
    iphlpapi.lib                # opengl / glfw
    Mfplat.lib                  # video capture                
    Mf.lib                      # video capture
    Mfuuid.lib                  # video capture
    Mfreadwrite.lib             # MFCreateSourceFreaderFromMediaSource
    Shlwapi.lib                 # QISearch (Callback)
    )
endif()

# The library
add_library(kanker ${lib_sources})
install(TARGETS kanker ARCHIVE DESTINATION lib)

# The font creator
add_executable(kankerfont ${sd}/main.cpp ${EXTERN_SRC_DIR}/glad.c)
target_link_libraries(kankerfont ${app_libs} kanker)
install(TARGETS kankerfont RUNTIME DESTINATION bin)

# The ABB app.
add_executable(kankerabb ${sd}/main_abb.cpp ${sd}/KankerAbbApp.cpp ${EXTERN_SRC_DIR}/glad.c)
target_link_libraries(kankerabb ${app_libs} kanker)
install(TARGETS kankerabb RUNTIME DESTINATION bin)

  
