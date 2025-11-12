cmake_minimum_required(VERSION 3.18)

project(kcuckounter LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(KDE "Enable KDE Frameworks and KDEGames integration" OFF)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(
            -Og -g -fno-omit-frame-pointer
            -Werror
            -Wall
            -Wextra
            -Wpedantic
            -Wcast-align
            -Wcast-qual
            -Wconversion
            -Wctor-dtor-privacy
            -Wenum-compare
            -Wfloat-equal
            -Wnon-virtual-dtor
            -Wold-style-cast
            -Woverloaded-virtual
            -Wredundant-decls
            -Wsign-conversion
            -Wsign-promo
    )
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -mtune=native -DNDEBUG")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s")
endif()

set(qt_min_version "6.0.0")

if(KDE)
    set(kf6_min_version "6.0.0")
    find_package(ECM 1.0.0 REQUIRED NO_MODULE)
    set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
    include(KDEInstallDirs)
    include(KDECMakeSettings)
    include(KDECompilerSettings NO_POLICY_SCOPE)
else()
    list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
    include(GNUInstallDirs)
endif()

include(FeatureSummary)

find_package(Qt6 ${qt_min_version} CONFIG REQUIRED COMPONENTS Core Widgets)

if(KDE)
    find_package(KF6 ${kf6_min_version} REQUIRED COMPONENTS
            CoreAddons
            I18n
            XmlGui
            ConfigWidgets
            WidgetsAddons
            KIO
    )
    find_package(KDEGames6 REQUIRED)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

qt_add_executable(kcuckounter
        MANUAL_FINALIZATION
        src/main.cpp
        src/main_window.cpp
        include/main_window.hpp
)

target_include_directories(kcuckounter PRIVATE include)

target_link_libraries(kcuckounter
        PRIVATE
        Qt6::Core
        Qt6::Widgets
        $<$<BOOL:${KDE}>:
        KF6::CoreAddons
        KF6::I18n
        KF6::XmlGui
        KF6::ConfigWidgets
        KF6::WidgetsAddons
        KF6::KIOCore
        KDEGames6
        >
)

target_compile_definitions(kcuckounter PRIVATE $<$<BOOL:${KDE}>:KC_KDE>)

if(ANDROID)
    set_property(TARGET kcuckounter PROPERTY QT_ANDROID_PACKAGE_NAME org.example.kcuckounter)
endif()

qt_finalize_executable(kcuckounter)

if(KDE)
    set(install_args ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
else()
    set(install_args
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
endif()

if(NOT ANDROID)
    install(TARGETS kcuckounter ${install_args})
endif()

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
