install(TARGETS sharity-client RUNTIME DESTINATION bin)

include(InstallRequiredSystemLibraries)

if (WIN32)
    # For Windows, installs LibDataChannel and all its dependencies
    install(
        IMPORTED_RUNTIME_ARTIFACTS LibDataChannel::LibDataChannel
        RUNTIME_DEPENDENCY_SET DATACHANNEL_SET
        LIBRARY DESTINATION bin)
    install(
        RUNTIME_DEPENDENCY_SET DATACHANNEL_SET
        PRE_EXCLUDE_REGEXES "api-ms-" "ext-ms-" "hvsifiletrust" "C:/msys64/mingw64/bin.*" ".*/msys64/mingw64/bin.*"
        POST_EXCLUDE_REGEXES ".*system32/.*\\.dll" ".*/msys64/mingw64/bin.*"
        LIBRARY DESTINATION bin)
endif ()

qt_generate_deploy_qml_app_script(
    TARGET sharity-client
    OUTPUT_SCRIPT DEPLOY_SCRIPT)
install(SCRIPT ${DEPLOY_SCRIPT})

set(CPACK_PACKAGE_NAME "Sharity")
set(CPACK_PACKAGE_VENDOR "Instellate")
set(CPACK_PACKAGE_DESCRIPTION "A End-To-End Encrypted Peer-to-Peer file sharing client")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/../LICENSE.txt")
set(CPACK_PACKAGE_EXECUTABLES sharity-client;Sharity)
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Sharity")

include(CPack)
