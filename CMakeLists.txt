cmake_minimum_required(VERSION 4.2)
project(sdtp LANGUAGES C VERSION 1.0 DESCRIPTION "sdtp")

set(CMAKE_C_STANDARD 11)

add_library(sdtp SHARED
        src/api/buffer.c
        src/api/packet.c
        src/api/instance.c
        src/api/buffer_api.c
        src/api/misc.c)

set_target_properties(sdtp PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(sdtp PROPERTIES PUBLIC_HEADER include/api/libsdtp.h)

include_directories(${CMAKE_SOURCE_DIR}/include)
target_include_directories(sdtp PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

include(GNUInstallDirs)
install(TARGETS sdtp
LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

configure_file(libsdtp.pc.in libsdtp.pc @ONLY)

install(FILES ${CMAKE_BINARY_DIR}/libsdtp.pc DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)
