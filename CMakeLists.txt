cmake_minimum_required(VERSION 4.2)
project(sdtp LANGUAGES C VERSION 1.0 DESCRIPTION "sdtp")

set(CMAKE_C_STANDARD 11)

add_library(sdtp SHARED src/libsdtp.c)

set_target_properties(sdtp PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(sdtp PROPERTIES PUBLIC_HEADER src/libsdtp.h)

target_include_directories(sdtp PRIVATE src)

include(GNUInstallDirs)
install(TARGETS sdtp
LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

configure_file(libsdtp.pc.in libsdtp.pc @ONLY)

install(FILES ${CMAKE_BINARY_DIR}/libsdtp.pc DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)
