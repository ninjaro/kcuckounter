# kcuckounter

## ...one flew east, one flew west, <br>One flew over the multiple decks.

[//]: # ([![version]&#40;https://img.shields.io/github/v/release/ninjaro/kcuckounter?include_prereleases&#41;]&#40;https://github.com/ninjaro/kcuckounter/releases/latest&#41;)

[//]: # ([![Checks]&#40;https://github.com/ninjaro/kcuckounter/actions/workflows/tests.yml/badge.svg&#41;]&#40;https://github.com/ninjaro/kcuckounter/actions/workflows/tests.yml&#41;)

[//]: # ([![Docs & Coverage]&#40;https://github.com/ninjaro/kcuckounter/actions/workflows/html.yml/badge.svg&#41;]&#40;https://github.com/ninjaro/kcuckounter/actions/workflows/html.yml&#41;)

[//]: # ([![codecov]&#40;https://codecov.io/gh/ninjaro/kcuckounter/graph/badge.svg?token=MCNEJFWMDU&#41;]&#40;https://codecov.io/gh/ninjaro/kcuckounter&#41;)

[![license](https://img.shields.io/github/license/ninjaro/kcuckounter?color=e6e6e6)](https://github.com/ninjaro/kcuckounter/blob/master/license)

> Just play poker in Indian Casinos and stay single and live where and how he wants to, if people would let him,
> <br> P.R. said — but Chief released him.

They’re out there. <br>
Black Kings and Red Queens in suits — up before me, to be committed to the repository and get mopped up before you can
memorize them.

## Brief Description

**kcuckounter** is an educational game. The project aims to
improve basic arithmetic skills and memory retention by counting cards in
different table-slots with different strategies. The user has the option to use
one of the preloaded strategies (Hi-Lo, Hi-Opt I, Hi-Opt II, Zen Count and
other) or create their own custom strategy. The game is implemented in C++ with
using Qt6, KF6, KDEGames6 and carddeck packs from kdegames-card-data-kf6.

## Gameplay

| ![Game setup](screenshots/Screenshot_20230221_180009.png)           | ![Game process](screenshots/Screenshot_20230221_180045.png) |
|---------------------------------------------------------------------|-------------------------------------------------------------|
| ![Strategy information](screenshots/Screenshot_20230221_180212.png) | todo:![Choosing a theme](screenshots/Screenshot_null.png)   |

At the beginning of the game, the user customizes the number of table-slots, and
for each table-slot, they can choose the number of standard playing card decks
and the strategy. The abstract dealer then picks up one card from one or more
table-slots (depending on the level of difficulty). The user must keep track of
the sum of the weight of the cards in each table-slot, taking into account the
weight assigned to each card in the chosen strategy.

If the next picked card is a joker, the user must answer a joker's question to
continue playing. The joker card has no weight and is rare, so answering
correctly about the current weight of the table-slot with the joker card changes
the score.

The score is based on the player's ability to answer the joker questions
correctly and is measured by the number of jokers the player has guessed
correctly out of the total number of joker questions.

The main focus of the game is to improve arithmetic skills and memory, and the
score serves as a motivational tool.

## Requirements

To build **kcuckounter** you need a C++20 toolchain, CMake and the Qt 6
stack with KDE Frameworks 6. Typical packages are:

- `cmake` and a C++ compiler (e.g. `clang++`)
- Qt 6 base and svg modules
- KDE Frameworks 6: `kcoreaddons`, `ki18n`, `kxmlgui`, `kconfigwidgets`,
  `kwidgetsaddons`, `kio` and `libkdegames`
- `extra-cmake-modules`

## Setup and Installation

### KDE on (baseline)

 ```bash
 cmake -B build-kde/ -DCMAKE_INSTALL_PREFIX=$HOME/kde/usr/ -DQT_MAJOR_VERSION=6 -DKDE=ON
 cmake --build build-kde/ --parallel $(nproc)
# sudo cmake --install build-kde/
 ```

### KDE off

 ```bash
 cmake -B build-nokde/ -DCMAKE_INSTALL_PREFIX=$HOME/kde/usr/ -DQT_MAJOR_VERSION=6 -DKDE=OFF
 cmake --build build-nokde/ --parallel $(nproc)
# sudo cmake --install build-nokde/
 ```

### Run

 ```bash
 source build/prefix.sh
 kcuckounter
 ```

## Documentation and Contributing

For detailed documentation see the [Documentation](https://ninjaro.github.io/kcuckounter/doc/) page.

## Docker

The provided `Dockerfile` builds **kcuckounter** inside an Arch Linux
container. After the image is created the game can be started directly:

```bash
docker build -t kcuckounter .
docker run --rm kcuckounter
```

Unit tests and coverage generation can be enabled during the build by passing
additional build arguments:

```bash
docker build -t kcuckounter --build-arg COVERAGE=ON .
```

## Security Policy

Please report any security issues using GitHub's private vulnerability reporting
or by emailing [yaroslav.riabtsev@rwth-aachen.de](mailto:yaroslav.riabtsev@rwth-aachen.de).
See the [security policy](.github/SECURITY.md) for full details.

## License

This project is open-source and available under the MIT License.