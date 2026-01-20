cmake_minimum_required(VERSION 3.14)
project(sharity-client VERSION 0.1.0)

option(CONFIGURE_NSIS "Configures CPack for NSIS installer packaging" OFF)
option(CONFIGURE_WIX "Configures CPack for MSI installer packaging" OFF)
option(CONFIGURE_BUNDLE "Configures CPack for MacOS bundle installer packaging" OFF)

include(FetchContent)

if (EMSCRIPTEN)
  message(STATUS "Configuring for WASM target")

  FetchContent_Declare(
    LibDataChannelWasm
    GIT_REPOSITORY https://github.com/paullouisageneau/datachannel-wasm.git
    GIT_TAG v0.3.2)
  FetchContent_MakeAvailable(LibDataChannelWasm)

  set(LIBDATACHANNEL datachannel-wasm)

  set(Rust_CARGO_TARGET wasm32-unknown-emscripten)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions -sNO_DISABLE_EXCEPTION_CATCHING")
else ()
  find_package(LibDataChannel CONFIG REQUIRED)
  set(LIBDATACHANNEL LibDataChannel::LibDataChannel)
endif ()

FetchContent_Declare(
  Corrosion
  GIT_REPOSITORY https://github.com/corrosion-rs/corrosion.git
  GIT_TAG v0.6.1)
FetchContent_MakeAvailable(Corrosion)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set(CMAKE_AUTOMOC ON)
# set(CMAKE_AUTORCC ON)
# set(CMAKE_AUTOUIC ON)

corrosion_import_crate(MANIFEST_PATH
                       ${CMAKE_SOURCE_DIR}/../vodozemac/Cargo.toml)

if (MSVC AND (CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo"))
  message(STATUS "Compiling Rust library with Debug version of the MSVC Runtime Library")
  corrosion_set_env_vars(vodozemac "CFLAGS=-MDd" "CXXFLAGS=-MDd")
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Concurrent Gui Widgets Quick Qml LinguistTools)
qt_standard_project_setup(
  I18N_SOURCE_LANGUAGE en_GB
  I18N_TRANSLATED_LANGUAGES sv_SE de_DE)

if (QT_KNOWN_POLICY_QTP0004)
  qt_policy(SET QTP0004 NEW)
endif ()

if (QT_KNOWN_POLICY_QTP0002)
  qt_policy(SET QTP0002 NEW)
endif()


set(RESOURCE_FILES icons.qrc)
if (EMSCRIPTEN)
  message(STATUS "Adding twemoji resources")
  list(APPEND RESOURCE_FILES fonts.qrc)
endif ()
qt_add_resources(SHARITY_RESOURCES ${RESOURCE_FILES})

qt_add_executable(sharity-client
        src/main.cpp
        src/WebSocket.cpp
        src/SasVerification.cpp
        src/DownloaderPeer.cpp
        src/UploaderPeer.cpp
        ${SHARITY_RESOURCES})
target_link_libraries(
  sharity-client
  PRIVATE Qt6::Core
          Qt6::Concurrent
          Qt6::Gui
          Qt6::Widgets
          Qt6::Quick
          Qt6::Qml
          vodozemac
          ${LIBDATACHANNEL})

set_target_properties(sharity-client PROPERTIES
  QT_ANDROID_APP_NAME "Sharity"
  QT_ANDROID_APP_ICON "@mipmap/ic_launcher"
  QT_ANDROID_PACKAGE_NAME "xyz.instellate.sharity"
  QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_SOURCE_DIR}/android")

if (WIN32)
 target_link_libraries(sharity-client PRIVATE bcrypt)
endif ()

target_include_directories(sharity-client PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_include_directories(sharity-client
                           PUBLIC ${CMAKE_SOURCE_DIR}/../vodozemac/include)

file(GLOB_RECURSE QML_FILES ${PROJECT_SOURCE_DIR}/qml/*.qml)
foreach (QML_FILE IN LISTS QML_FILES)
  file(RELATIVE_PATH REL "${PROJECT_SOURCE_DIR}" "${QML_FILE}")
  string(REPLACE "qml/" "./" TRANSFORMED_PATH "${REL}")
  message(STATUS "Registering QML file: ${QML_FILE}")
  set_source_files_properties(${QML_FILE} PROPERTIES QT_RESOURCE_ALIAS
                                                     "${TRANSFORMED_PATH}")
endforeach ()

set(QML_SINGLETONS src/WebSocket.h)
set_source_files_properties(${QML_SINGLETONS} PROPERTIES
                                              QT_QML_SINGLETON_TYPE true)

qt_add_qml_module(sharity-client
  VERSION 1.0
  URI Sharity
  OUTPUT_DIRECTORY qmlGen
  RESOURCE_PREFIX /qt/qml
  QML_FILES ${QML_FILES}
  SOURCES src/WebSocket.h src/SasVerification.h src/DownloaderPeer.h src/UploaderPeer.h)

qt_add_translations(
  sharity-client
  TS_FILE_BASE qml
  TS_FILE_DIR i18n
  TS_OUTPUT_DIRECTORY i18n
  RESOURCE_PREFIX /qt/qml/Sharity/i18n)

if ((MSVC) AND (MSVC_VERSION GREATER_EQUAL 1914))
  target_compile_options(sharity-client PUBLIC "/Zc:__cplusplus")
  target_compile_options(sharity-client PRIVATE /W4)
  # MSVC warnings is weirder and triggers in more weird places.
  # I trust Linux compilers to do a better job on this
else ()
  target_compile_options(sharity-client PRIVATE -Wall -Wextra -Werror
                                                -Wno-language-extension-token)
endif ()

if (WIN32 AND NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
  set_property(TARGET sharity-client PROPERTY WIN32_EXECUTABLE true)
endif ()

if (WIN32 AND CONFIGURE_NSIS)
  message(STATUS "Configuring NSIS Generator")
  set(CPACK_GENERATOR NSIS)
  set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)

  # Files with extra commands
  # Handles such things as URL scheme registration
  file(READ "${CMAKE_SOURCE_DIR}/cmake/cpack/nsis/install.nsi" INSTALL_COMMANDS)
  string(REPLACE "\\" "\\\\" INSTALL_COMMANDS "${INSTALL_COMMANDS}")
  string(REPLACE "\"" "\\\"" CPACK_NSIS_EXTRA_INSTALL_COMMANDS "${INSTALL_COMMANDS}")

  file(READ "${CMAKE_SOURCE_DIR}/cmake/cpack/nsis/uninstall.nsi" UNINSTALL_COMMANDS)
  string(REPLACE "\\" "\\\\" UNINSTALL_COMMANDS "${UNINSTALL_COMMANDS}")
  string(REPLACE "\"" "\\\"" CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "${UNINSTALL_COMMANDS}")
elseif (WIN32 AND CONFIGURE_WIX)
  message(STATUS "Configuring WIX Generator")
  set(CPACK_GENERATOR WIX)
  set(CPACK_WIX_VERSION 4)
elseif (APPLE AND CONFIGURE_BUNDLE)
  message(STATUS "Configuring Bundle Generator")
  set_target_properties(sharity-client PROPERTIES MACOSX_BUNDLE TRUE)
  set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/bundle/sharity.icns"
                              MACOSX_PACKAGE_LOCATION "Resources")
  set_target_properties(sharity-client PROPERTIES
    PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/bundle/Info.plist")
endif ()

if (DEFINED CPACK_GENERATOR)
  include(cmake/ConfigureCPack.cmake)
endif ()
