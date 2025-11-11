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
#ifdef KC_KDE
// KDEGames
#include <KGameClock>
#include <KGameHighScoreDialog>
#include <KGameStandardAction>
// KF
#include <KActionCollection>
#include <KGameDifficultyLevel>
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
    game_clock = new KGameClock(this, KGameClock::FlexibleHourMinSec);
    connect(
        game_clock, &KGameClock::timeChanged, this, &MainWindow::advance_time
    );

    score_label->setText(i18n("Score: 0/0"));
    time_label->setText(i18n("Time: 00:00"));

    speed_slider->setRange(100, 1000);
    speed_slider->setValue(300);
    speed_slider->setToolTip(i18n("Card pickup interval (ms)"));
    lives_label->setText("");
    update_lives_display();

    statusBar()->insertPermanentWidget(0, score_label);
    statusBar()->insertPermanentWidget(1, time_label);
    statusBar()->insertPermanentWidget(2, speed_slider);
    statusBar()->insertPermanentWidget(3, lives_label);

    table = new Table;
    connect(speed_slider, &QSlider::valueChanged, table, &Table::set_speed);
    table->set_speed(speed_slider->value());
    connect(table, &Table::score_update, this, &MainWindow::on_score_update);
    connect(table, &Table::game_over, this, &MainWindow::on_game_over);

    const Settings& opts = Settings::instance();
    connect(
        &opts, &Settings::show_score_changed, score_label, &QWidget::setVisible
    );
    connect(
        &opts, &Settings::show_time_changed, time_label, &QWidget::setVisible
    );
    connect(
        &opts, &Settings::show_speed_changed, speed_slider, &QWidget::setVisible
    );
    connect(
        &opts, &Settings::card_theme_changed, table, &Table::set_card_theme
    );
    // connect(&opts, &Settings::card_background_changed, table,
    // qOverload<>(&QWidget::update));
    connect(
        &opts, &Settings::card_border_changed, table,
        qOverload<>(&QWidget::update)
    );

    setCentralWidget(table);
    setup_actions();
    new_game();
    load_settings();
}

void MainWindow::setup_actions() {
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
    game_clock->restart();
    game_clock->pause();
    table->create_new_game(
        KGameDifficulty::global()->currentLevel()->hardness()
    );
    action_pause->setEnabled(true);
    if (action_end_game) {
        action_end_game->setEnabled(false);
    }
    action_pause->setText(i18n("&Start"));
    action_pause->setIcon(QIcon::fromTheme("media-playback-start"));
    if (!action_pause->isChecked()) {
        action_pause->setChecked(true);
    } else {
        pause_game(true);
    }
    score = { 0, 0 };
    score_label->setText(i18n("Score: 0/0"));
    lives = max_lives;
    update_lives_display();
    KGameDifficulty::global()->setGameRunning(false);
    time_label->setText(i18n("Time: 00:00"));
}

void MainWindow::force_end_game() const {
    table->force_game_over();
    game_clock->pause();
    action_pause->setEnabled(false);
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
    game_clock->pause();
    action_pause->setEnabled(false);
    if (action_end_game) {
        action_end_game->setEnabled(false);
    }
    KGameDifficulty::global()->setGameRunning(false);
    QPointer scoreDialog = new KGameHighScoreDialog(
        KGameHighScoreDialog::Name | KGameHighScoreDialog::Time, this
    );
    scoreDialog->initFromDifficulty(KGameDifficulty::global());
    KGameHighScoreDialog::FieldInfo scoreInfo;
    scoreInfo[KGameHighScoreDialog::Score]
        = i18n("%1/%2", score.first, score.second);
    scoreInfo[KGameHighScoreDialog::Time] = game_clock->timeString();

    if (scoreDialog->addScore(scoreInfo, KGameHighScoreDialog::LessIsMore) != 0)
        scoreDialog->exec();

    delete scoreDialog;
}

void MainWindow::advance_time(const QString& elapsed_time) const {
    time_label->setText(i18n("Time: %1", elapsed_time));
}

void MainWindow::show_high_scores() {
    QPointer score_dialog = new KGameHighScoreDialog(
        KGameHighScoreDialog::Name | KGameHighScoreDialog::Time, this
    );
    score_dialog->initFromDifficulty(KGameDifficulty::global());
    score_dialog->exec();
    delete score_dialog;
}

void MainWindow::configure_settings() {
    Settings& opts = Settings::instance();

    QDialog dialog(this);
    dialog.setWindowTitle(i18n("Settings"));

    auto* tabs = new QTabWidget(&dialog);
    auto* general = new QWidget;
    auto* generalForm = new QFormLayout(general);

    auto* indexing = new QCheckBox(general);
    indexing->setChecked(opts.indexing());
    generalForm->addRow(indexing, new QLabel(i18n("Use card indexing")));

    auto* strategy_hint = new QCheckBox(general);
    strategy_hint->setChecked(opts.strategy_hint());
    generalForm->addRow(
        strategy_hint, new QLabel(i18n("Add name of strategy"))
    );

    auto* training = new QCheckBox(general);
    training->setChecked(opts.training());
    generalForm->addRow(training, new QLabel(i18n("Is training")));

    auto* show_time = new QCheckBox(general);
    show_time->setChecked(opts.show_time());
    generalForm->addRow(show_time, new QLabel(i18n("Show time")));

    auto* show_score = new QCheckBox(general);
    show_score->setChecked(opts.show_score());
    generalForm->addRow(show_score, new QLabel(i18n("Show score")));

    auto* show_speed = new QCheckBox(general);
    show_speed->setChecked(opts.show_speed());
    generalForm->addRow(show_speed, new QLabel(i18n("Show speed control")));

    auto* infinity_mode = new QCheckBox(general);
    infinity_mode->setChecked(opts.infinity_mode());
    generalForm->addRow(infinity_mode, new QLabel(i18n("Infinity mode")));

    // theme page with preview
    auto* theme_page = new QWidget;
    auto* theme_layout = new QVBoxLayout(theme_page);
    auto* theme_combo = new QComboBox(theme_page);
    const QDir card_dir(QStringLiteral("/usr/share/carddecks"));
    for (const QStringList dirs
         = card_dir.entryList(QStringList() << "svg-*", QDir::Dirs);
         const QString& dir : dirs) {
        const QString id = dir.mid(4);
        QSettings deck(
            card_dir.filePath(dir + "/index.desktop"), QSettings::IniFormat
        );
        const QString name = deck.value(QStringLiteral("Name"), id).toString();
        theme_combo->addItem(name, id);
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
        const QString path = QStandardPaths::locate(
            QStandardPaths::GenericDataLocation,
            QStringLiteral("carddecks/svg-%1/%1.svgz").arg(id)
        );
        if (path.isEmpty()) {
            QMessageBox::warning(
                this, i18n("Missing Theme"),
                i18n("Card theme '%1' could not be found.", id)
            );
            return;
        }
        delete theme_renderer;
        theme_renderer = new QSvgRenderer(path);
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

void MainWindow::pause_game(const bool paused) const {
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
        game_clock->pause();
        KGameDifficulty::global()->setGameRunning(false);
    } else {
        game_clock->resume();
        KGameDifficulty::global()->setGameRunning(true);
    }
}

void MainWindow::load_settings() const {
    const Settings& opts = Settings::instance();
    score_label->setVisible(opts.show_score());
    time_label->setVisible(opts.show_time());
    speed_slider->setVisible(opts.show_speed());
    table->set_card_theme(opts.card_theme());
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
    const int level = KGameDifficulty::global()->currentLevel()->hardness();
    if (table->is_launching()) {
        table->set_card_mode(level);
    } else {
        new_game();
    }
}