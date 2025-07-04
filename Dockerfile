FROM archlinux:latest

ENV LANG=en_US.UTF-8

RUN pacman -Syu --noconfirm && \
    pacman -S --noconfirm \
    cmake clang llvm  lcov \
    kcoreaddons ki18n kxmlgui kconfigwidgets kwidgetsaddons kio \
    libkdegames base-devel extra-cmake-modules \
    qt6-svg \
    && pacman -Scc --noconfirm
#    xcb-util xcb-util-image xcb-util-keysyms xcb-util-renderutil xcb-util-wm \

WORKDIR /usr/src/app

COPY . .

ARG COVERAGE=OFF

RUN cmake -S . -B build \
      -DCMAKE_CXX_COMPILER=clang++ \
      -DCMAKE_INSTALL_PREFIX=/usr/local/kde/usr \
      -DQT_MAJOR_VERSION=6 \
      -DCMAKE_BUILD_TYPE=Release \
      -DBUILD_TESTS=${COVERAGE} \
      -DCOVERAGE=${COVERAGE}

RUN if [ "${COVERAGE}" = "ON" ]; then \
        cmake --build build --target coverage --parallel $(nproc); \
    else \
        cmake --build build --parallel $(nproc); \
    fi

RUN cmake --install build

ENV PATH="/usr/local/kde/usr/bin:${PATH}"

CMD ["kcuckounter"]
