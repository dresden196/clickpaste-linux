cmake_minimum_required(VERSION 3.16)
project(clickpaste VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# Find KDE Frameworks
find_package(ECM 6.0.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

find_package(KF6GlobalAccel REQUIRED)
find_package(LayerShellQt REQUIRED)

# Source files
set(SOURCES
    src/main.cpp
    src/application.cpp
    src/trayicon.cpp
    src/settingsdialog.cpp
    src/settings.cpp
    src/hotkeymanager.cpp
    src/inputemulator.cpp
    src/targetoverlay.cpp
    src/clipboardmanager.cpp
)

set(HEADERS
    src/application.h
    src/trayicon.h
    src/settingsdialog.h
    src/settings.h
    src/hotkeymanager.h
    src/inputemulator.h
    src/targetoverlay.h
    src/clipboardmanager.h
)

# Resources
qt_add_resources(RESOURCES resources/resources.qrc)

add_executable(clickpaste
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
)

target_include_directories(clickpaste PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(clickpaste
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    KF6::GlobalAccel
    LayerShellQt::Interface
)

# Install targets
install(TARGETS clickpaste DESTINATION ${KDE_INSTALL_BINDIR})
install(FILES resources/clickpaste.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES resources/icons/clickpaste.svg DESTINATION ${KDE_INSTALL_ICONDIR}/hicolor/scalable/apps)
