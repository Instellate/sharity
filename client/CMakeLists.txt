cmake_minimum_required(VERSION 4.0)
project(sharity-client)

include(FetchContent)

if (EMSCRIPTEN)
  message(STATUS "Configuring for WASM target")

  FetchContent_Declare(
    LibDataChannelWasm
    GIT_REPOSITORY https://github.com/paullouisageneau/datachannel-wasm.git
    GIT_TAG v0.3.2)
  FetchContent_MakeAvailable(LibDataChannelWasm)

  set(LIBDATACHANNEL datachannel-wasm)

  FetchContent_Declare(json 
    URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz)
  FetchContent_MakeAvailable(json)

  set(Rust_CARGO_TARGET wasm32-unknown-emscripten)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions -sNO_DISABLE_EXCEPTION_CATCHING")
else ()
  find_package(LibDataChannel CONFIG REQUIRED)
  find_package(nlohmann_json REQUIRED)

  set(LIBDATACHANNEL LibDataChannel::LibDataChannel)
endif ()

FetchContent_Declare(
  Corrosion
  GIT_REPOSITORY https://github.com/corrosion-rs/corrosion.git
  GIT_TAG v0.5)
FetchContent_MakeAvailable(Corrosion)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

corrosion_import_crate(MANIFEST_PATH
                       ${CMAKE_SOURCE_DIR}/../vodozemac/Cargo.toml)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Quick Qml)
qt_standard_project_setup()

if (QT_KNOWN_POLICY_QTP0004)
  qt_policy(SET QTP0004 NEW)
endif ()

if (EMSCRIPTEN)
  message(STATUS "Adding twemoji resources")
  qt_add_resources(SHARITY_RESOURCES resources.qrc)
endif ()

qt_add_executable(sharity-client src/main.cpp src/WebSocket.cpp src/Websocket.h ${SHARITY_RESOURCES}
        src/SasVerification.cpp
        src/SasVerification.h)
target_link_libraries(
  sharity-client
  PRIVATE Qt6::Core
          Qt6::Gui
          Qt6::Widgets
          Qt6::Quick
          Qt6::Qml
          vodozemac
          ${LIBDATACHANNEL}
          nlohmann_json::nlohmann_json)

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
  SOURCES src/Websocket.h src/SasVerification.h)

if ((MSVC) AND (MSVC_VERSION GREATER_EQUAL 1914))
  target_compile_options(sharity-client PUBLIC "/Zc:__cplusplus")
  target_compile_options(sharity-client PRIVATE /W4 /WX)
else ()
  target_compile_options(sharity-client PRIVATE -Wall -Wextra -Werror
                                                -Wno-language-extension-token)
endif ()
