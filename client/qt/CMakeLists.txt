cmake_minimum_required(VERSION 3.19)
project(stardew_client_qt LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(stardew_client_qt
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.h mainwindow.cpp
    usernamescreen.h usernamescreen.cpp
    startmenuscreen.h startmenuscreen.cpp
    newgamescreen.h newgamescreen.cpp
    loadscreen.h loadscreen.cpp
    joinscreen.h joinscreen.cpp
    spritebutton.h spritebutton.cpp
)

target_link_libraries(stardew_client_qt
    PRIVATE
        Qt::Core
        Qt::Widgets
)

target_compile_definitions(stardew_client_qt
    PRIVATE
        ASSETS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../../common/assets"
)

install(TARGETS stardew_client_qt
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET stardew_client_qt
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
