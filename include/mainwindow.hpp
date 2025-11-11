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

#ifndef CARD_COUNTER_MAINWINDOW_HPP
#define CARD_COUNTER_MAINWINDOW_HPP

// Qt
#include <QLabel>
#include <QPointer>
#include <QSlider>
#ifdef KC_KDE
// KF
#include <KXmlGuiWindow>
using BaseMainWindow = KXmlGuiWindow;
class KGameClock;
class KToggleAction;
using ToggleAction = KToggleAction;
#else
#include <QMainWindow>
using BaseMainWindow = QMainWindow;
#include <QAction>
using ToggleAction = QAction;
#include <QElapsedTimer>
#include <QTimer>
class QActionGroup;
#endif

class Table;

class MainWindow final : public BaseMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private Q_SLOTS:

    void advance_time(const QString& elapsed_time) const;

    void load_settings() const;

    void new_game();

    void force_end_game();

    void on_game_over();

    void show_high_scores();

    void configure_settings();

    void card_mode_changed();

    void pause_game(bool paused);

    void on_score_update(bool inc);

    void update_lives_display() const;

private:
    void setup_actions();

    void closeEvent(QCloseEvent* event) override;

    Table* table;

#ifdef KC_KDE
    KGameClock* game_clock = nullptr;
#else
    QTimer* game_timer = nullptr;
    QElapsedTimer elapsed;

    qint64 elapsed_accumulated_ms = 0;
    bool elapsed_running = false;

    QActionGroup* difficulty_group = nullptr;
    int current_level = 1; // 1=Sequential, 3=Random, 10=Simultaneous

    bool game_running = false;
#endif
    ToggleAction* action_pause = nullptr;

    QPointer<QLabel> time_label = new QLabel;
    QPointer<QLabel> score_label = new QLabel;
    QPointer<QSlider> speed_slider = new QSlider(Qt::Horizontal);
    QPointer<QLabel> lives_label = new QLabel;

    int lives = 3;
    int max_lives = 3;

    QPair<qint32, qint32> score;
    QAction* action_end_game = nullptr;
};

#endif // CARD_COUNTER_MAINWINDOW_HPP
