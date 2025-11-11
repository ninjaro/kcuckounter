cmake_minimum_required(VERSION 3.18)

project(kcuckounter)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(KDE "Enable KDE Frameworks and KDEGames integration" ON)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
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
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -mtune=native -DNDEBUG")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s")
endif ()


set(QT_MIN_VERSION "6.0.0")
if (KDE)
    set(KF6_MIN_VERSION "6.0.0")
    find_package(ECM 1.0.0 REQUIRED NO_MODULE)
    set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

    include(KDEInstallDirs)
    include(KDECMakeSettings)
    include(KDECompilerSettings NO_POLICY_SCOPE)
else ()
    list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
    include(GNUInstallDirs)
endif ()

include(FeatureSummary)

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED
        COMPONENTS Core Widgets Svg Quick Test
)

if (KDE)
    find_package(KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS
            CoreAddons I18n XmlGui ConfigWidgets WidgetsAddons KIO
    )
    find_package(KDEGames6 REQUIRED)
endif ()
feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(kcuckounter_HEADERS
        include/mainwindow.hpp
        include/table/table.hpp
        include/table/tableslot.hpp
        include/settings.hpp
        include/strategy/strategyinfo.hpp
        include/strategy/strategy.hpp
        include/widgets/cards.hpp
        include/widgets/carousel.hpp
        include/widgets/base/label.hpp
        include/widgets/base/frame.hpp
        include/compat/i18n_shim.hpp
        include/compat/config_shim.hpp)

set(kcuckounter_SOURCES
        src/mainwindow.cpp
        src/table/table.cpp
        src/table/tableslot.cpp
        src/settings.cpp
        src/strategy/strategyinfo.cpp
        src/strategy/strategy.cpp
        src/widgets/carousel.cpp
        src/widgets/cards.cpp
        src/widgets/base/label.cpp
        src/widgets/base/frame.cpp
        src/compat/config_shim.cpp)

add_library(kcuckounter_lib STATIC
        ${kcuckounter_HEADERS}
        ${kcuckounter_SOURCES}
        ${kcuckounter_resources})


target_link_libraries(kcuckounter_lib
        PUBLIC
        Qt6::Widgets
        Qt6::Svg
        $<$<BOOL:${KDE}>:
        KF6::CoreAddons;
        KF6::I18n;
        KF6::XmlGui;
        KF6::ConfigWidgets;
        KF6::WidgetsAddons;
        KF6::KIOCore;
        KDEGames6
        >
)

target_include_directories(kcuckounter_lib PUBLIC include)

set_target_properties(kcuckounter_lib PROPERTIES UNITY_BUILD OFF)

add_executable(kcuckounter src/main.cpp)

target_link_libraries(kcuckounter PRIVATE
        kcuckounter_lib)
target_compile_definitions(kcuckounter_lib PUBLIC  $<$<BOOL:${KDE}>:KC_KDE>)
target_compile_definitions(kcuckounter     PRIVATE $<$<BOOL:${KDE}>:KC_KDE>)


if (KDE)
    set(_INSTALL_ARGS ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
else()
    set(_INSTALL_ARGS
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif ()
install(TARGETS kcuckounter ${_INSTALL_ARGS})