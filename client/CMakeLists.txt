cmake_minimum_required(VERSION 3.16)

project(chatApp VERSION 0.1 LANGUAGES CXX)




set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(QML_IMPORT_PATH "C:/QT6/6.6.3/mingw_64/qml")


#  设置静态链接标志
if (MINGW) # 检查是否正在使用 MinGW 编译器
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
endif()




find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Quick Qml )
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Quick Qml quickwidgets  Gui Multimedia)


#这里更改为asio库的实际位置
include_directories("D:\\qtCode\\c++Qt\\chatApp\\chatApp\\asio-1.30.0\\include")



set(PROJECT_SOURCES
        main.cpp
        loggin.cpp
        loggin.h
        loggin.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(chatApp
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        picture.qrc
        shadowwindow.h shadowwindow.cpp
        reg.h reg.cpp reg.ui
        chat_server.h chat_server.cpp
        qml.qrc
        audio.h audio.cpp
        videohandler.h videohandler.cpp

    )

# Define target properties for Android with Qt 6 as:

#    set_property(TARGET chatApp APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR

#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)

# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

else()
    if(ANDROID)
        add_library(chatApp SHARED
            ${PROJECT_SOURCES}
        )

# Define properties for Android with Qt 5 after find_package() calls as:

#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")

    else()
        add_executable(chatApp
            ${PROJECT_SOURCES}
        )
    endif()

endif()


target_link_libraries(chatApp PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt6::Qml Qt6::Quick Qt6::QuickWidgets  ws2_32 Qt6::Gui Qt6::Multimedia)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.

# If you are developing for iOS or macOS you should consider setting an

# explicit, fixed bundle identifier manually though.

if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.chatApp)
endif()
set_target_properties(chatApp PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS chatApp
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(chatApp)
endif()
