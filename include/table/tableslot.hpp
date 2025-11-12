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

#ifndef CARD_COUNTER_TABLESLOT_HPP
#define CARD_COUNTER_TABLESLOT_HPP

// own
#include "widgets/cards.hpp"

class QSvgRenderer;

class QSpinBox;

class QVBoxLayout;

class QPushButton;

class CCFrame;

class CCLabel;

class StrategyInfo;

class Strategy;

class QComboBox;

class QPropertyAnimation;

/**
 * @brief Interactive widget displaying a deck of cards.
 *
 * Represents one slot on the table. It maintains its own deck and
 * counting strategy and communicates with the Table via signals.
 */
class TableSlot final : public Cards {
    Q_OBJECT
    Q_PROPERTY(
        float highlight_opacity READ get_highlight_opacity WRITE
            set_highlight_opacity
    )
public:
    explicit TableSlot(
        StrategyInfo* strategies, QSvgRenderer* renderer,
        bool is_active = false, QWidget* parent = nullptr
    );

    /**
     * @brief Indicates whether the slot is still inactive.
     */
    [[nodiscard]] bool is_fake() const;

    /**
     * @brief Draw the next card and update weights.
     */
    void pick_up_card();

    void set_infinite_params(int idx, int total);
    void set_target_size(const QSize& s);
    QSize sizeHint() const override;


protected:
    void paintEvent(QPaintEvent* event) override;

signals:

    void table_slot_activated();

    void table_slot_removed();

    void table_slot_finished();

    void table_slot_reshuffled();

    void user_quizzed();

    void user_answered(bool correct);

    void swap_target_selected();

    void strategy_info_assist();

public Q_SLOTS:

    void on_game_paused(bool paused);

    void on_can_remove(bool can_remove) const;

    void on_new_strategy() const;

    void on_strategy_changed(int index);

    void user_checking();

    void reshuffle_deck();

    void activate(int value);

    /**
     * @brief Replace the strategies object used by this slot.
     */
    void set_strategies(StrategyInfo* info);

private:
    void user_quizzing();

    float get_highlight_opacity() const;
    void set_highlight_opacity(float value);

    void start_highlight();

    float highlight_opacity;
    QPropertyAnimation* highlight_anim;

    QList<qint32> cards;
    Strategy* strategy {};
    StrategyInfo* strategies;
    qint32 current_weight = 0;
    bool fake = true;

    CCFrame* answer_frame;
    CCFrame* settings_frame;
    CCFrame* control_frame;

    QSpinBox* deck_count;
    QSpinBox* weight_box;

    CCLabel* message_label;
    CCLabel* index_label;
    CCLabel* weight_label;
    CCLabel* strategy_hint_label;

    QPushButton* refresh_button;
    QPushButton* swap_button;
    QPushButton* close_button;

    QComboBox* strategy_box;

    qint32 slot_idx = 0;
    qint32 total_slots = 1;
    QSize target_size {};
};

#endif // CARD_COUNTER_TABLESLOT_HPP
