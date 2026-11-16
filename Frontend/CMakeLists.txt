cmake_minimum_required(VERSION 3.16)
project(NetworkRecoverySimulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Support Homebrew Qt paths on macOS
if(NOT Qt6_DIR)
    if(EXISTS "/opt/homebrew/opt/qtbase")
        list(PREPEND CMAKE_PREFIX_PATH "/opt/homebrew/opt/qtbase")
    elseif(EXISTS "/opt/homebrew/opt/qt")
        list(PREPEND CMAKE_PREFIX_PATH "/opt/homebrew/opt/qt")
    elseif(EXISTS "/usr/local/opt/qtbase")
        list(PREPEND CMAKE_PREFIX_PATH "/usr/local/opt/qtbase")
    endif()
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)

set(PROJECT_SOURCES
    main.cpp
    MainWindow.h
    MainWindow.cpp
    api/ApiClient.h
    api/ApiClient.cpp
    models/Router.h
    models/Connection.h
    models/NetworkEvent.h
    widgets/TopBar.h
    widgets/TopBar.cpp
    widgets/Sidebar.h
    widgets/Sidebar.cpp
    widgets/StatCard.h
    widgets/StatCard.cpp
    widgets/CustomChart.h
    widgets/CustomChart.cpp
    widgets/NetworkCanvas.h
    widgets/NetworkCanvas.cpp
    views/DashboardView.h
    views/DashboardView.cpp
    views/TopologyView.h
    views/TopologyView.cpp
    views/RouterView.h
    views/RouterView.cpp
    views/ConnectionView.h
    views/ConnectionView.cpp
    views/TopologyEditorView.h
    views/TopologyEditorView.cpp
    views/FailureSimulationView.h
    views/FailureSimulationView.cpp
    views/RecoveryView.h
    views/RecoveryView.cpp
    views/HistoryView.h
    views/HistoryView.cpp
    views/AnalyticsView.h
    views/AnalyticsView.cpp
    views/SettingsView.h
    views/SettingsView.cpp
    resources/resources.qrc
)

add_executable(NetworkRecoverySimulator ${PROJECT_SOURCES})

target_include_directories(NetworkRecoverySimulator PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/api
    ${CMAKE_CURRENT_SOURCE_DIR}/models
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets
    ${CMAKE_CURRENT_SOURCE_DIR}/views
)

target_link_libraries(NetworkRecoverySimulator PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
)
