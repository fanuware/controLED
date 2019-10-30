cmake_minimum_required(VERSION 3.5.1)

project(controLED-server)

set(PROJECT_VERSION_MAJOR "1")
set(PROJECT_VERSION_MINOR "0")
set(PROJECT_VERSION_PATCH "0")

# use C++11 and flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O2 -Wall -pthread -MMD -MP -lwiringPi")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# build external project: ws281x library
include(ExternalProject)
ExternalProject_Add (
    rpi_ws281x
    EXCLUDE_FROM_ALL false
    GIT_REPOSITORY https://github.com/jgarff/rpi_ws281x
    GIT_TAG 48cec2139534a2da9f8cdf4f26fe253e8d6e05f2
    SOURCE_DIR "${CMAKE_BINARY_DIR}/rpi_ws281x"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND "scons" "--directory=${CMAKE_BINARY_DIR}/rpi_ws281x"
    INSTALL_COMMAND ""
)

# find ws281x library
add_library(ws28x-lib STATIC IMPORTED) # or STATIC instead of SHARED
set_target_properties(ws28x-lib PROPERTIES
    IMPORTED_LOCATION "${CMAKE_BINARY_DIR}/rpi_ws281x/libws2811.a"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_BINARY_DIR}"
)

set(HEADERS
    include/animation.h
    include/controller_led.h
    include/device_led.h
    include/device_led_74hc595.h
    include/device_led_ws2812.h
    include/fout.h
    include/labrinth_animation.h
    include/rgba.h
    include/snake_animation.h
    include/webserver.h
)

set(SOURCES
    src/animation.cpp
    src/controller_led.cpp
    src/device_led.cpp
    src/device_led_74hc595.cpp
    src/device_led_ws2812.cpp
    src/fout.cpp
    src/labrinth_animation.cpp
    src/main.cpp
    src/rgba.cpp
    src/snake_animation.cpp
    src/webserver.cpp
)

# executable
add_executable(controLED
    ${SOURCES} ${HEADERS}
)

# include directories
target_include_directories(controLED PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(controLED PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/lib")

# boost library
find_package(Boost COMPONENTS system filesystem REQUIRED)
target_link_libraries(controLED
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
)

# ws281x library
target_include_directories(controLED PRIVATE ws28x-lib)
target_link_libraries(controLED ws28x-lib)
add_dependencies(controLED ws28x-lib)

# OpenSSL library
find_package(OpenSSL REQUIRED)
target_link_libraries(controLED OpenSSL::Crypto)

find_library(wiringPi_LIB wiringPi)
target_link_libraries(controLED ${wiringPi_LIB})

# install executable
install(TARGETS controLED
    RUNTIME DESTINATION bin
    COMPONENT applications)

# systemd service
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/systemd/controled.service
        DESTINATION /usr/lib/systemd/system)

# Package builder for Debian
set(CPACK_GENERATOR "DEB")
set(CPACK_CMAKE_GENERATOR Ninja) # do not install preinstall target
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Control LED Matrix on Raspberry Pi")
set(CPACK_PACKAGE_CONTACT "fanuflip fanuflip@gmail.com")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS OFF)
set(CPACK_SET_DESTDIR true)

include(CPack)
