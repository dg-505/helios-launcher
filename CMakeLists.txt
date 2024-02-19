cmake_minimum_required(VERSION 3.5)

# extract version number from InnoSetup script
file(READ "./util/win/packInstaller.iss" ISS)
string(REGEX MATCH "Version \"([0-9]+\\.[0-9]+\\.[0-9]+)\"" _ ${ISS})
set(VERSION_NUMBER "${CMAKE_MATCH_1}")
message("Helios++ launcher Version: ${VERSION_NUMBER}")
# Replace old version number with new version number
set(MAIN_FILE "./src/main.cpp")
file(READ ${MAIN_FILE} MAIN)
string(REGEX REPLACE "\"([0-9]+\\.[0-9]+\\.[0-9]+)\";" "\"${VERSION_NUMBER}\";" MOD "${MAIN}")
file(WRITE ${MAIN_FILE} "${MOD}")

project(helios-launcher VERSION ${VERSION_NUMBER} LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

set(TS_FILES i18n/en_US.ts)

include_directories(${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/ui)

set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/res/helios-launcher.rc")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
file(COPY ${CMAKE_PREFIX_PATH}/plugins/platforms DESTINATION ${CMAKE_BINARY_DIR}/plugins)
if (UNIX AND NOT APPLE)
    foreach(LIB IN ITEMS "libicudata.so.56" "libicudata.so.56.1" "libicui18n.so.56" "libicui18n.so.56.1" "libicuuc.so.56" "libicuuc.so.56.1" "libQt5Core.so.5" "libQt5Core.so.5.15.16" "libQt5DBus.so.5" "libQt5DBus.so.5.15.16" "libQt5Gui.so.5" "libQt5Gui.so.5.15.16" "libQt5Widgets.so.5" "libQt5Widgets.so.5.15.16" "libQt5XcbQpa.so.5" "libQt5XcbQpa.so.5.15.16")
        file(COPY ${CMAKE_PREFIX_PATH}/lib/${LIB} DESTINATION ${CMAKE_BINARY_DIR}/lib)
    endforeach()
    file(COPY "/usr/lib/x86_64-linux-gnu/libxcb-xinerama.so.0" DESTINATION ${CMAKE_BINARY_DIR}/lib)
    file(COPY "/usr/lib/x86_64-linux-gnu/libxcb-xinerama.so.0.0.0" DESTINATION ${CMAKE_BINARY_DIR}/lib)
endif ()

add_executable(helios-launcher
    ${app_icon_resource_windows}
    include/launcher.h include/basedir.h include/help.h
    src/main.cpp src/launcher.cpp src/basedir.cpp src/help.cpp
    ui/launcher.ui ui/basedir.ui ui/help.ui
    res/resources.qrc
    ${TS_FILES}
)

qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})

target_link_libraries(helios-launcher PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.helios-launcher)
endif()
set_target_properties(helios-launcher PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS helios-launcher
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
