/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2025 Yaroslav Riabtsev <yaroslav.riabtsev@rwth-aachen.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

// Qt
#include <QCheckBox>
#include <QCloseEvent>
#include <QStackedWidget>
#include <QSpinBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDir>
#include <QFormLayout>
#include <QIcon>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QSettings>
#include <QStandardPaths>
#include <QStatusBar>
#include <QSvgRenderer>
#include <QToolBar>
#include <QVBoxLayout>
#include <QPointer>
#include  <QTimer>
#include <QFile>
#ifdef KC_KDE
// KDEGames
#include <KGameClock>
#include <KGameHighScoreDialog>
#include <KGameStandardAction>
// KF
#include <KActionCollection>
#include <KGameDifficultyLevel>
#else
#include <QActionGroup>
#endif
// own
#include "compat/i18n_shim.hpp"
#include "mainwindow.hpp"
#include "settings.hpp"
#include "table/table.hpp"
#include "widgets/cards.hpp"
#include "widgets/carousel.hpp"

MainWindow::MainWindow(QWidget* parent)
    : BaseMainWindow(parent) {
    time_label = new QLabel(this);
    score_label = new QLabel(this);
    speed_slider = new QSlider(Qt::Horizontal, this);
    lives_label = new QLabel(this);
#ifdef KC_KDE
    game_clock = new KGameClock(this, KGameClock::FlexibleHourMinSec);
    connect(
        game_clock, &KGameClock::timeChanged, this, &MainWindow::advance_time
    );
#else
    game_timer = new QTimer(this);
    connect(game_timer, &QTimer::timeout, this, [this]() {
        const qint64 total_ms = elapsed_accumulated_ms
            + (elapsed_running ? elapsed.elapsed() : 0);
        const qint64 secs = total_ms / 1000;
        const int hh = static_cast<int>(secs / 3600);
        const int mm = static_cast<int>((secs % 3600) / 60);
        const int ss = static_cast<int>(secs % 60);
        const QString t = QString("%1:%2:%3")
                              .arg(hh, 2, 10, QLatin1Char('0'))
                              .arg(mm, 2, 10, QLatin1Char('0'))
                              .arg(ss, 2, 10, QLatin1Char('0'));
        advance_time(t);
    });
#endif
    score_label->setText(i18n("Score: 0/0"));
#ifdef KC_KDE
    time_label->setText(i18n("Time: 00:00"));
#else
    time_label->setText(i18n("Time: 00:00:00"));
#endif

    speed_slider->setRange(100, 1000);
    speed_slider->setValue(300);
    speed_slider->setToolTip(i18n("Card pickup interval (ms)"));
    lives_label->setText("");
    update_lives_display();

    statusBar()->insertPermanentWidget(0, score_label);
    statusBar()->insertPermanentWidget(1, time_label);
    statusBar()->insertPermanentWidget(2, speed_slider);
    statusBar()->insertPermanentWidget(3, lives_label);

    // removed: immediate Table construction and wiring
    // table = new Table;
    // connect(speed_slider, &QSlider::valueChanged, table, &Table::set_speed);
    // table->set_speed(speed_slider->value());
    // connect(table, &Table::score_update, this, &MainWindow::on_score_update);
    // connect(table, &Table::game_over, this, &MainWindow::on_game_over);

    const Settings& opts = Settings::instance();
    connect(
        &opts, &Settings::show_time_changed, time_label, &QWidget::setVisible
    );
    connect(
        &opts, &Settings::show_score_changed, score_label, &QWidget::setVisible
    );
    connect(
        &opts, &Settings::show_speed_changed, speed_slider, &QWidget::setVisible
    );
    // moved into create_and_wire_table():
    // connect(&opts, &Settings::card_theme_changed, table, &Table::set_card_theme);
    // connect(&opts, &Settings::card_border_changed, table, qOverload<>(&QWidget::update));

    // Build stacked UI (pre-setup + game pages)
    stack = new QStackedWidget(this);
    build_pre_setup_page();
    build_game_page();
    setCentralWidget(stack);
    stack->setCurrentWidget(pre_setup_page);

    setup_actions();

    // Pre-setup state: disable game-only actions and ensure timers are paused.
    if (action_pause) {
        action_pause->setEnabled(false);
        action_pause->setText(i18n("&Start"));
        action_pause->setIcon(QIcon::fromTheme("media-playback-start"));
        if (action_pause->isChecked()) {
            action_pause->setChecked(false);
        }
    }
    if (action_end_game) {
        action_end_game->setEnabled(false);
    }
#ifdef KC_KDE
    game_clock->pause();
#else
    elapsed_accumulated_ms = 0;
    elapsed_running = false;
    if (game_timer) {
        game_timer->stop();
    }
    advance_time(QStringLiteral("00:00:00"));
    game_running = false;
#endif

    // No auto-start
    // new_game();

    load_settings();
}

void MainWindow::setup_actions() {
#ifdef KC_KDE
    KGameStandardAction::gameNew(
        this, &MainWindow::new_game, actionCollection()
    );
    KGameStandardAction::highscores(
        this, &MainWindow::show_high_scores, actionCollection()
    );

    KGameStandardAction::quit(this, &MainWindow::close, actionCollection());
    KStandardAction::preferences(
        this, &MainWindow::configure_settings, actionCollection()
    );

    action_pause = KGameStandardAction::pause(
        this, &MainWindow::pause_game, actionCollection()
    );
    action_end_game = KGameStandardAction::end(
        this, &MainWindow::force_end_game, actionCollection()
    );

    auto* diff = KGameDifficulty::global();
    diff->addLevel(new KGameDifficultyLevel(
        1, QByteArray("Sequential"), i18n("Sequential")
    ));
    diff->addLevel(
        new KGameDifficultyLevel(3, QByteArray("Random"), i18n("Random"))
    );
    diff->addLevel(new KGameDifficultyLevel(
        10, QByteArray("Simultaneous"), i18n("Simultaneous")
    ));
    KGameDifficultyGUI::init(this);
    connect(
        diff, &KGameDifficulty::currentLevelChanged, this,
        &MainWindow::card_mode_changed
    );

    setupGUI(Default);

    auto* main_tool_bar = addToolBar(i18n("Main Toolbar"));
    main_tool_bar->setObjectName(QStringLiteral("main_tool_bar"));
    main_tool_bar->addAction(
        actionCollection()->action(QStringLiteral("game_new"))
    );
    main_tool_bar->addAction(action_end_game);
    main_tool_bar->addAction(action_pause);
#else
    auto* main_tool_bar = addToolBar(i18n("Main Toolbar"));
    main_tool_bar->setObjectName(QStringLiteral("main_tool_bar"));

    QAction* act_new = new QAction(i18n("&New Game"), this);
    act_new->setIcon(QIcon::fromTheme(QStringLiteral("document-new")));
    connect(act_new, &QAction::triggered, this, &MainWindow::new_game);
    main_tool_bar->addAction(act_new);

    action_end_game = new QAction(i18n("&End Game"), this);
    action_end_game->setIcon(QIcon::fromTheme(QStringLiteral("process-stop")));
    action_end_game->setEnabled(false);
    connect(
        action_end_game, &QAction::triggered, this, &MainWindow::force_end_game
    );
    main_tool_bar->addAction(action_end_game);

    action_pause = new QAction(i18n("&Start"), this);
    action_pause->setCheckable(true);
    action_pause->setIcon(
        QIcon::fromTheme(QStringLiteral("media-playback-start"))
    );

    connect(action_pause, &QAction::toggled, this, [this](bool checked) {
        pause_game(!checked);
    });
    main_tool_bar->addAction(action_pause);

    QAction* act_prefs = new QAction(i18n("&Settings"), this);
    act_prefs->setIcon(QIcon::fromTheme(QStringLiteral("configure")));
    connect(
        act_prefs, &QAction::triggered, this, &MainWindow::configure_settings
    );
    main_tool_bar->addAction(act_prefs);

    difficulty_group = new QActionGroup(this);
    difficulty_group->setExclusive(true);

    auto* act_seq = new QAction(i18n("Sequential"), difficulty_group);
    act_seq->setCheckable(true);
    act_seq->setData(1);
    act_seq->setChecked(true);

    auto* act_rand = new QAction(i18n("Random"), difficulty_group);
    act_rand->setCheckable(true);
    act_rand->setData(3);

    auto* act_sim = new QAction(i18n("Simultaneous"), difficulty_group);
    act_sim->setCheckable(true);
    act_sim->setData(10);

    // connect(
    //     difficulty_group, &QActionGroup::triggered, this, [this](QAction* a)
    //     {
    //         current_level = a->data().toInt();
    //         card_mode_changed();
    //     }
    // );

    main_tool_bar->addSeparator();
    main_tool_bar->addActions(difficulty_group->actions());
#endif
}

void MainWindow::new_game() {
    if (score.second > 0) {
        const auto res = QMessageBox::question(
            this, i18n("End Game"),
            i18n("End current game and save the result?"),
            QMessageBox::Yes | QMessageBox::No
        );
        if (res == QMessageBox::No) {
            return;
        }
        force_end_game();
    }

    // Reset timers and UI, disable game-only actions.
#ifdef KC_KDE
    game_clock->restart();
    game_clock->pause();
    KGameDifficulty::global()->setGameRunning(false);
#else
    elapsed_accumulated_ms = 0;
    elapsed_running = false;
    if (game_timer) {
        game_timer->stop();
    }
    advance_time(QStringLiteral("00:00:00"));
    game_running = false;
#endif
    score = { 0, 0 };
    score_label->setText(i18n("Score: 0/0"));
    lives = max_lives;
    update_lives_display();

    if (action_pause) {
        action_pause->setEnabled(false);
        action_pause->setText(i18n("&Start"));
        action_pause->setIcon(QIcon::fromTheme("media-playback-start"));
        if (action_pause->isChecked()) {
            action_pause->setChecked(false);
        }
    }
    if (action_end_game) {
        action_end_game->setEnabled(false);
    }

    // Destroy current game (if any) and go back to pre-setup.
    destroy_game();
    if (stack && pre_setup_page) {
        stack->setCurrentWidget(pre_setup_page);
    }
}

void MainWindow::on_pre_setup_continue() {
    const int slot_count = qMax(1, slots_spin ? slots_spin->value() : 1);

    create_and_wire_table(slot_count);

    const int level =
#ifdef KC_KDE
        KGameDifficulty::global()->currentLevel()->hardness();
#else
        current_level;
#endif

    table->set_initial_slot_count(slot_count);
    table->create_new_game(level);

    // Reset timers to zero but keep paused until Start.
#ifdef KC_KDE
    game_clock->restart();
    game_clock->pause();
    KGameDifficulty::global()->setGameRunning(false);
#else
    elapsed_accumulated_ms = 0;
    elapsed_running = false;
    if (game_timer) {
        game_timer->stop();
    }
    advance_time(QStringLiteral("00:00:00"));
    game_running = false;
#endif

    // Reset score/lives.
    score = { 0, 0 };
    score_label->setText(i18n("Score: 0/0"));
    lives = max_lives;
    update_lives_display();

    // Prepare actions for a not-yet-started game.
    if (action_pause) {
        action_pause->setEnabled(true);
        action_pause->setText(i18n("&Start"));
        action_pause->setIcon(QIcon::fromTheme("media-playback-start"));
        if (action_pause->isChecked()) {
            action_pause->setChecked(false);
        } else {
            pause_game(true);
        }
    }
    if (action_end_game) {
        action_end_game->setEnabled(false);
    }

    if (stack && game_page) {
        stack->setCurrentWidget(game_page);
    }
}

void MainWindow::force_end_game() {
    if (table) {
        table->force_game_over();
    }
#ifdef KC_KDE
    game_clock->pause();
#else
    if (game_timer) {
        if (elapsed_running) {
            elapsed_accumulated_ms += elapsed.elapsed();
            elapsed_running = false;
        }
        game_timer->stop();
    }
#endif
    if (action_pause) {
        action_pause->setEnabled(false);
    }
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (score.second > 0) {
        const auto res = QMessageBox::question(
            this, i18n("Quit"), i18n("End current game and save the result?"),
            QMessageBox::Yes | QMessageBox::No
        );
        if (res == QMessageBox::No) {
            event->ignore();
            return;
        }
        force_end_game();
    }
    event->accept();
}

void MainWindow::on_game_over() {
#ifdef KC_KDE
    game_clock->pause();
#else
    if (game_timer) {
        if (elapsed_running) {
            elapsed_accumulated_ms += elapsed.elapsed();
            elapsed_running = false;
        }
        game_timer->stop();
    }
#endif
    action_pause->setEnabled(false);
    if (action_end_game) {
        action_end_game->setEnabled(false);
    }

#ifdef KC_KDE
    KGameDifficulty::global()->setGameRunning(false);
    QPointer<KGameHighScoreDialog> score_dialog = new KGameHighScoreDialog(
        KGameHighScoreDialog::Name | KGameHighScoreDialog::Time, this
    );
    score_dialog->initFromDifficulty(KGameDifficulty::global());
    KGameHighScoreDialog::FieldInfo scoreInfo;
    scoreInfo[KGameHighScoreDialog::Score]
        = i18n("%1/%2", score.first, score.second);
    scoreInfo[KGameHighScoreDialog::Time] = game_clock->timeString();

    if (score_dialog->addScore(scoreInfo, KGameHighScoreDialog::LessIsMore) != 0) {
        score_dialog->exec();
    }

    delete score_dialog;
#else
    const QString time_text
        = time_label->text().mid(QStringLiteral("Time: ").size());
    QMessageBox::information(
        this, i18n("Game Over"),
        i18n("Score: %1/%2\nTime: %3", score.first, score.second, time_text)
    );
#endif
}

void MainWindow::advance_time(const QString& elapsed_time) const {
    time_label->setText(i18n("Time: %1", elapsed_time));
}

void MainWindow::show_high_scores() {
#ifdef KC_KDE
    QPointer<KGameHighScoreDialog> score_dialog = new KGameHighScoreDialog(
        KGameHighScoreDialog::Name | KGameHighScoreDialog::Time, this
    );
    score_dialog->initFromDifficulty(KGameDifficulty::global());
    score_dialog->exec();
    delete score_dialog;
#else
    QMessageBox::information(
        this, i18n("High Scores"),
        i18n("High scores are not available without KDE Games.")
    );
#endif
}

static QString findDeckSvg(const QString& id) {
    const auto rel_svgz = QStringLiteral("carddecks/svg-%1/%1.svgz").arg(id);
    const auto rel_svg = QStringLiteral("carddecks/svg-%1/%1.svg").arg(id);

#ifdef Q_OS_ANDROID
    const QString qrc_svgz = QStringLiteral(":/%1").arg(rel_svgz);
    if (QFile::exists(qrc_svgz)) {
        return qrc_svgz;
    }
    const QString qrc_svg = QStringLiteral(":/%1").arg(rel_svg);
    if (QFile::exists(qrc_svg)) {
        return qrc_svg;
    }
#endif

    QString path
        = QStandardPaths::locate(QStandardPaths::GenericDataLocation, rel_svgz);
    if (path.isEmpty()) {
        path = QStandardPaths::locate(
            QStandardPaths::GenericDataLocation, rel_svg
        );
    }
    return path;
}

void MainWindow::configure_settings() {
    Settings& opts = Settings::instance();

    QDialog dialog(this);
    dialog.setWindowTitle(i18n("Settings"));

    auto* tabs = new QTabWidget(&dialog);
    auto* general = new QWidget;
    auto* general_form = new QFormLayout(general);

    auto* indexing = new QCheckBox(general);
    indexing->setChecked(opts.indexing());
    general_form->addRow(indexing, new QLabel(i18n("Use card indexing")));

    auto* strategy_hint = new QCheckBox(general);
    strategy_hint->setChecked(opts.strategy_hint());
    general_form->addRow(
        strategy_hint, new QLabel(i18n("Add name of strategy"))
    );

    auto* training = new QCheckBox(general);
    training->setChecked(opts.training());
    general_form->addRow(training, new QLabel(i18n("Is training")));

    auto* show_time = new QCheckBox(general);
    show_time->setChecked(opts.show_time());
    general_form->addRow(show_time, new QLabel(i18n("Show time")));

    auto* show_score = new QCheckBox(general);
    show_score->setChecked(opts.show_score());
    general_form->addRow(show_score, new QLabel(i18n("Show score")));

    auto* show_speed = new QCheckBox(general);
    show_speed->setChecked(opts.show_speed());
    general_form->addRow(show_speed, new QLabel(i18n("Show speed control")));

    auto* infinity_mode = new QCheckBox(general);
    infinity_mode->setChecked(opts.infinity_mode());
    general_form->addRow(infinity_mode, new QLabel(i18n("Infinity mode")));

    // theme page with preview
    auto* theme_page = new QWidget;
    auto* theme_layout = new QVBoxLayout(theme_page);
    auto* theme_combo = new QComboBox(theme_page);
    // const QDir card_dir(QStringLiteral("/usr/share/carddecks"));
    // for (const QStringList dirs
    //      = card_dir.entryList(QStringList() << "svg-*", QDir::Dirs);
    //      const QString& dir : dirs) {
    //     const QString id = dir.mid(4);
    //     QSettings deck(
    //         card_dir.filePath(dir + "/index.desktop"), QSettings::IniFormat
    //     );
    //     const QString name = deck.value(QStringLiteral("Name"),
    //     id).toString(); theme_combo->addItem(name, id);
    // }
    auto add_deck_from = [&](const QString& base, bool isQrc) {
        QDir d(base);
        const auto dirs = d.entryList(QStringList() << "svg-*", QDir::Dirs);
        for (const QString& dir : dirs) {
            const QString id = dir.mid(4);
            QString idx = isQrc
                ? QStringLiteral(":/carddecks/%1/index.desktop").arg(dir)
                : QDir(base).filePath(dir + "/index.desktop");
            QSettings deck(idx, QSettings::IniFormat);
            const QString name
                = deck.value(QStringLiteral("Name"), id).toString();
            theme_combo->addItem(name, id);
        }
    };

#ifdef Q_OS_ANDROID
    add_deck_from(QStringLiteral(":/carddecks"), /*isQrc=*/true);
#endif
    if (QDir(QStringLiteral("/usr/share/carddecks")).exists()) {
        add_deck_from(QStringLiteral("/usr/share/carddecks"), /*isQrc=*/false);
    }
    for (int i = 0; i < theme_combo->count(); ++i) {
        if (theme_combo->itemData(i).toString() == opts.card_theme()) {
            theme_combo->setCurrentIndex(i);
            break;
        }
    }

    QVector<Cards*> preview_cards;
    QSvgRenderer* theme_renderer = nullptr;
    auto* carousel = new Carousel(QSizeF(60, 90));

    auto update_preview = [&](const QString& id) {
        const QString path = findDeckSvg(id);
        if (path.isEmpty()) {
            QMessageBox::warning(
                this, i18n("Missing Theme"),
                i18n("Card theme '%1' could not be found.", id)
            );
            return;
        }
        delete theme_renderer;
        theme_renderer = new QSvgRenderer(path, this);
        if (!theme_renderer->isValid()) {
            qCritical() << "Failed to load deck SVG:" << path;
            return;
        }
        if (preview_cards.isEmpty()) {
            const auto deck = Cards::generate_deck(1);
            for (const qint32 c : deck) {
                auto* card = new Cards(theme_renderer);
                card->set_id(c);
                carousel->add_widget(card);
                preview_cards.push_back(card);
            }
        } else {
            for (auto* card : preview_cards) {
                card->set_renderer(theme_renderer);
            }
        }
        carousel->refresh();
    };

    update_preview(theme_combo->currentData().toString());
    theme_layout->addWidget(theme_combo);
    theme_layout->addWidget(carousel);
    connect(theme_combo, &QComboBox::currentIndexChanged, this, [=](int) {
        update_preview(theme_combo->currentData().toString());
    });

    // compact theme page with colour options
    auto* short_page = new QWidget;
    auto* short_layout = new QVBoxLayout(short_page);
    auto* short_combo = new QComboBox(short_page);
    for (int i = 0; i < theme_combo->count(); ++i) {
        short_combo->addItem(
            theme_combo->itemText(i), theme_combo->itemData(i)
        );
    }
    short_combo->setCurrentIndex(theme_combo->currentIndex());
    short_layout->addWidget(short_combo);

    // auto* bg_button = new QPushButton(i18n("Background"), short_page);
    auto* border_button = new QPushButton(i18n("Border"), short_page);
    // QColor bg_color = opts.card_background();
    QColor border_color = opts.card_border();
    auto set_btn_color = [](QPushButton* b, const QColor& c) {
        b->setStyleSheet(QStringLiteral("background-color:%1").arg(c.name()));
    };
    // set_btn_color(bg_button, bg_color);
    set_btn_color(border_button, border_color);
    // connect(bg_button, &QPushButton::clicked, short_page, [&]() {
    //     QColor c = QColorDialog::getColor(bg_color, short_page);
    //     if (c.isValid()) {
    //         bg_color = c;
    //         set_btn_color(bg_button, c);
    //     }
    // });
    connect(border_button, &QPushButton::clicked, short_page, [&] {
        const QColor c = QColorDialog::getColor(border_color, short_page);
        if (c.isValid()) {
            border_color = c;
            set_btn_color(border_button, c);
        }
    });
    // short_layout->addWidget(bg_button);
    short_layout->addWidget(border_button);

    connect(
        theme_combo, &QComboBox::currentIndexChanged, short_combo,
        &QComboBox::setCurrentIndex
    );
    connect(
        short_combo, &QComboBox::currentIndexChanged, theme_combo,
        &QComboBox::setCurrentIndex
    );

    tabs->addTab(general, i18n("General"));
    tabs->addTab(theme_page, i18n("Card Theme"));
    tabs->addTab(short_page, i18n("Theme List"));

    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel
        | QDialogButtonBox::Apply
    );
    auto* layout = new QVBoxLayout(&dialog);
    layout->addWidget(tabs);
    layout->addWidget(buttons);

    auto apply = [&] {
        opts.set_indexing(indexing->isChecked());
        opts.set_strategy_hint(strategy_hint->isChecked());
        opts.set_training(training->isChecked());
        opts.set_show_time(show_time->isChecked());
        opts.set_show_score(show_score->isChecked());
        opts.set_show_speed(show_speed->isChecked());
        opts.set_infinity_mode(infinity_mode->isChecked());
        opts.set_card_theme(theme_combo->currentData().toString());
        // opts.set_card_background(bg_color);
        opts.set_card_border(border_color);
    };

    connect(buttons, &QDialogButtonBox::accepted, &dialog, [&]() {
        apply();
        dialog.accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    connect(
        buttons->button(QDialogButtonBox::Apply), &QPushButton::clicked,
        &dialog, apply
    );

    dialog.exec();
    delete theme_renderer;
}

void MainWindow::pause_game(const bool paused) {
    if (!table) {
        return;
    }
    const bool was_launching = table->is_launching();
    table->pause(paused);
    if (was_launching && !table->is_launching()) {
        action_pause->setText(i18n("&Pause"));
        action_pause->setIcon(QIcon::fromTheme("media-playback-pause"));
        if (action_end_game) {
            action_end_game->setEnabled(true);
        }
    }
    if (paused) {
#ifdef KC_KDE
        game_clock->pause();
        KGameDifficulty::global()->setGameRunning(false);
#else
        game_running = false;
        if (game_timer) {
            if (elapsed_running) {
                elapsed_accumulated_ms += elapsed.elapsed();
                elapsed_running = false;
            }
            game_timer->stop();
        }
#endif
    } else {
#ifdef KC_KDE
        game_clock->resume();
        KGameDifficulty::global()->setGameRunning(true);
#else
        game_running = true;
        if (game_timer) {
            if (!elapsed_running) {
                elapsed.restart();
                elapsed_running = true;
            }
            game_timer->start(1000);
        }
#endif
    }
}

void MainWindow::load_settings() const {
    const Settings& opts = Settings::instance();
    score_label->setVisible(opts.show_score());
    time_label->setVisible(opts.show_time());
    speed_slider->setVisible(opts.show_speed());
    if (table) {
        table->set_card_theme(opts.card_theme());
    }
}

void MainWindow::on_score_update(const bool inc) {
    score.second++;
    score.first += inc;
    score_label->setText(i18n("Score: %1/%2", score.first, score.second));
    if (!inc) {
        if (lives > 0) {
            --lives;
            update_lives_display();
            if (lives == 0) {
                force_end_game();
            }
        }
    }
}

void MainWindow::update_lives_display() const {
    QString hearts;
    if (lives) {
        hearts += QString("<font color='red'>%1</font>")
                      .arg(QString("&#x2665;").repeated(lives));
    }
    if (max_lives > lives) {
        hearts += QString("<font color='black'>%1</font>")
                      .arg(QString("&#x2665;").repeated(max_lives - lives));
    }
    lives_label->setText(hearts);
}

void MainWindow::card_mode_changed() {
    const int level
#ifdef KC_KDE
        = KGameDifficulty::global()->currentLevel()->hardness();
#else
        = current_level;
#endif
    if (!table) {
        return; // stay in pre-setup; do not auto-start
    }
    if (table->is_launching()) {
        table->set_card_mode(level);
    } else {
        new_game();
    }
}

// Build pre-setup page: spin + continue button.
void MainWindow::build_pre_setup_page() {
    pre_setup_page = new QWidget(this);
    pre_setup_page->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto* layout = new QVBoxLayout(pre_setup_page);
    slots_spin = new QSpinBox(pre_setup_page);
    slots_spin->setMinimum(1);
    slots_spin->setValue(1);
    continue_button = new QPushButton(i18n("Set up / Continue"), pre_setup_page);
    layout->addWidget(slots_spin);
    layout->addWidget(continue_button);
    layout->addStretch(1);
    connect(continue_button, &QPushButton::clicked, this, &MainWindow::on_pre_setup_continue);
    stack->addWidget(pre_setup_page);
}

// Build empty game page (Table will be added later).
void MainWindow::build_game_page() {
    game_page = new QWidget(this);
    game_page->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto* layout = new QVBoxLayout(game_page);
    stack->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    stack->setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    stack->addWidget(game_page);
    game_page->layout()->setSizeConstraint(QLayout::SetNoConstraint);
}

// Create Table, wire it, and attach to game page.
void MainWindow::create_and_wire_table(qint32 /*slot_count*/) {
    destroy_game();

    table = new Table(game_page);
    table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto* layout = qobject_cast<QVBoxLayout*>(game_page->layout());
    if (!layout) {
        layout = new QVBoxLayout(game_page);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
    }
    layout->addWidget(table);

    connect(speed_slider, &QSlider::valueChanged, table, &Table::set_speed);
    table->set_speed(speed_slider->value());
    connect(table, &Table::score_update, this, &MainWindow::on_score_update);
    connect(table, &Table::game_over, this, &MainWindow::on_game_over);

    const Settings& opts = Settings::instance();
    connect(&opts, &Settings::card_theme_changed, table, &Table::set_card_theme);
    connect(&opts, &Settings::card_border_changed, table, qOverload<>(&QWidget::update));

    load_settings();

    QTimer::singleShot(0, table, &Table::recalculate_layout); // after insert
}

// Remove and delete Table from the game page.
void MainWindow::destroy_game() {
    if (!table) {
        return;
    }
    if (auto* layout = qobject_cast<QVBoxLayout*>(game_page->layout())) {
        layout->removeWidget(table);
    }
    table->deleteLater();
    table = nullptr;
}

