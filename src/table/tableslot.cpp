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
#include <QComboBox>
#include <QFormLayout>
#include <QPainter>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QRandomGenerator>
#include <QSpinBox>
#include <QSvgRenderer>
// own
#include "compat/i18n_shim.hpp"
#include "settings.hpp"
#include "strategy/strategy.hpp"
#include "strategy/strategyinfo.hpp"
#include "table/tableslot.hpp"
#include "widgets/cards.hpp"
// own widgets
#include "widgets/base/frame.hpp"
#include "widgets/base/label.hpp"

TableSlot::TableSlot(
    StrategyInfo* strategies, QSvgRenderer* renderer, const bool is_active,
    QWidget* parent
)
    : Cards(renderer, parent)
    , highlight_opacity(0.0)
    , strategies(strategies) {
    highlight_anim = new QPropertyAnimation(this, "highlight_opacity", this);
    highlight_anim->setDuration(500);

    // QLabels:
    message_label = new CCLabel(i18n("TableSlot Weight: 0"));
    index_label = new CCLabel("0/0");
    weight_label = new CCLabel("weight: 0");
    strategy_hint_label = new CCLabel("");
    on_strategy_changed(0);

    // QComboBoxes:
    strategy_box = new QComboBox();
    on_new_strategy();
    connect(
        strategies, &StrategyInfo::new_strategy, this,
        &TableSlot::on_new_strategy
    );
    connect(
        strategy_box, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
        &TableSlot::on_strategy_changed
    );
    Settings& opts = Settings::instance();
    index_label->setVisible(opts.indexing());
    strategy_hint_label->setVisible(opts.strategy_hint());
    weight_label->setVisible(opts.training());
    connect(
        &opts, &Settings::indexing_changed, index_label, &CCLabel::setVisible
    );
    connect(
        &opts, &Settings::strategy_hint_changed, strategy_hint_label,
        &CCLabel::setVisible
    );
    connect(
        &opts, &Settings::training_changed, weight_label, &CCLabel::setVisible
    );

    // QFrames:
    answer_frame = new CCFrame();
    settings_frame = new CCFrame();
    settings_frame->show();
    control_frame = new CCFrame();

    // QSpinBoxes:
    weight_box = new QSpinBox();
    weight_box->setRange(-100, 100);

    deck_count = new QSpinBox();
    connect(
        deck_count, QOverload<int>::of(&QSpinBox::valueChanged), this,
        &TableSlot::activate
    );
    deck_count->setRange(is_active, 10);
    deck_count->setVisible(!opts.infinity_mode());
    connect(
        &opts, &Settings::infinity_mode_changed, deck_count,
        &QWidget::setVisible
    );

    // QPushButtons:
    auto* submit_button
        = new QPushButton(QIcon::fromTheme("answer"), i18n("&Submit"));
    submit_button->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    connect(
        submit_button, &QPushButton::clicked, this, &TableSlot::user_checking
    );

    auto* skip_button = new QPushButton(
        QIcon::fromTheme("media-skip-forward"), i18n("&Skip")
    );
    skip_button->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    // todo:    connect(skipButton, &QPushButton::clicked, this,
    // &TableSlot::skipping);

    auto* strategy_info_button
        = new QPushButton(QIcon::fromTheme("kt-info-widget"), i18n("&Info"));
    strategy_info_button->setSizePolicy(
        QSizePolicy::Maximum, QSizePolicy::Maximum
    );
    connect(
        strategy_info_button, &QPushButton::clicked, this,
        &TableSlot::strategy_info_assist
    );

    close_button = new QPushButton(QIcon::fromTheme("delete"), i18n("&Remove"));
    close_button->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    connect(
        close_button, &QPushButton::clicked, this,
        &TableSlot::table_slot_removed
    );
    close_button->hide();

    refresh_button
        = new QPushButton(QIcon::fromTheme("view-refresh"), i18n("&Reshuffle"));
    refresh_button->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    connect(
        refresh_button, &QPushButton::clicked, this, &TableSlot::reshuffle_deck
    );
    refresh_button->hide();

    swap_button = new QPushButton(
        QIcon::fromTheme("exchange-positions"), i18n("&Swap")
    );
    swap_button->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    connect(
        swap_button, &QPushButton::clicked, this,
        &TableSlot::swap_target_selected
    );

    // QFormLayouts:
    auto* settings = new QFormLayout(settings_frame);
    settings->setFormAlignment(Qt::AlignCenter);
    auto* answer = new QFormLayout(answer_frame);
    answer->setFormAlignment(Qt::AlignCenter);

    // Other Layouts:
    auto* box_layout = new QVBoxLayout(this);
    auto* info_layout = new QHBoxLayout();
    auto* control_layout = new QHBoxLayout(control_frame);
    auto* strategy_layout = new QHBoxLayout();

    strategy_layout->addWidget(strategy_box);
    strategy_layout->addWidget(strategy_info_button);

    info_layout->addWidget(weight_label);
    info_layout->addStretch();
    info_layout->addWidget(index_label);

    answer->addRow(tr("&Weight:"), weight_box);
    answer->addRow(submit_button, skip_button);

    settings->addRow(tr("&Number of Card Decks:"), deck_count);
    settings->addRow(tr("Type of Strategy:"), strategy_layout);

    control_layout->addWidget(close_button);
    control_layout->addWidget(refresh_button);
    control_layout->addWidget(swap_button);

    box_layout->addWidget(strategy_hint_label);
    box_layout->addStretch();
    box_layout->addWidget(message_label);
    box_layout->addWidget(answer_frame);
    box_layout->addWidget(settings_frame);
    box_layout->addWidget(control_frame);
    box_layout->addStretch();
    box_layout->addLayout(info_layout);
}

void TableSlot::on_game_paused(const bool paused) {
    if (!paused && !settings_frame->isHidden()) {
        if (Settings::instance().infinity_mode()) {
            cards.clear();
        } else {
            const int decks = deck_count->value();
            if (decks < 1) {
                update();
                return;
            }
            cards = shuffle_cards(decks);
        }
        refresh_button->show();
        set_id(-1);
        settings_frame->hide();
        control_frame->show();
    }
    if (paused) {
        answer_frame->hide();
        set_name("blue_back");
        control_frame->show();
    } else {
        control_frame->hide();
        if (is_joker()) {
            set_name(get_card_name_by_current_id());
            user_quizzing();
        }
    }
    update();
}

bool TableSlot::is_fake() const { return fake; }

void TableSlot::pick_up_card() {
    const Settings& opts = Settings::instance();
    if (opts.infinity_mode()) {
        qint32 last_id = get_current_id();
        double k = 1.0 + static_cast<double>(slot_idx + 1) / total_slots;
        auto* rng = QRandomGenerator::global();
        qint32 new_card;
        if (rng->bounded(54) == 0) {
            qint32 colour = rng->bounded(2) ? Red : Black;
            new_card = (colour & 0xff) << 8;
        } else {
            int suit;
            int rank;
            bool have_last = last_id >= 0 && !Cards::is_joker(last_id);
            if (have_last && rng->generateDouble() < 1.0 / (4.0 * k)) {
                suit = Cards::get_suit(last_id);
            } else {
                QVector<int> suits;
                for (int s = Clubs; s <= Spades; ++s) {
                    if (!have_last || s != Cards::get_suit(last_id))
                        suits.append(s);
                }
                suit = suits[rng->bounded(suits.size())];
            }

            if (have_last && rng->generateDouble() < 1.0 / (13.0 * k)) {
                rank = Cards::get_rank(last_id);
            } else {
                QVector<int> ranks;
                for (int r = Ace; r <= King; ++r) {
                    if (!have_last || r != Cards::get_rank(last_id))
                        ranks.append(r);
                }
                rank = ranks[rng->bounded(ranks.size())];
            }
            new_card = ((suit & 0xff) << 8) | (rank & 0xff);
        }
        set_id(new_card);
    } else {
        if (cards.empty()) {
            set_name("back");
            emit table_slot_finished();
            settings_frame->show();
            control_frame->show();
            update();
            return;
        }
        //    if (isJoker()){
        //        messageLabel->hide();
        //    }
        set_id(cards.front());
        set_name(get_card_name_by_current_id());
        cards.pop_front();
    }
    if (!message_label->isHidden()) {
        message_label->hide();
    }
    update();
    if (!opts.infinity_mode()) {
        index_label->setText(i18n(
            "%1/%2", deck_count->value() * 54 - cards.size(),
            deck_count->value() * 54
        ));
    }
    if (is_joker()) {
        user_quizzing();
    } else {
        current_weight
            = strategy->update_weight(current_weight, get_current_rank());
        weight_label->setText(i18n("weight: %1", current_weight));
    }
    start_highlight();
}

void TableSlot::user_quizzing() {
    answer_frame->show();
    emit user_quizzed();
}

void TableSlot::user_checking() {
    message_label->setText(i18n("TableSlot Weight: %1", current_weight));
    answer_frame->hide();
    const bool is_correct = weight_box->value() == current_weight;
    message_label->setPalette(QPalette(is_correct ? Qt::green : Qt::red));
    message_label->show();
    emit user_answered(is_correct);
}

void TableSlot::reshuffle_deck() {
    if (Settings::instance().infinity_mode()) {
        cards.clear();
    } else {
        cards = shuffle_cards(deck_count->value());
    }
    settings_frame->hide();
    // hide controlFrame if not paused
}

void TableSlot::on_can_remove(const bool can_remove) const {
    close_button->setVisible(can_remove);
}

void TableSlot::activate(const int value) {
    if (value > 0 && fake) {
        fake = false;
        control_frame->show();
        set_name("green_back");
        current_weight = 0;
        deck_count->setMinimum(1);
        emit table_slot_activated();
    }
}

void TableSlot::set_infinite_params(const int idx, const int total) {
    slot_idx = idx;
    total_slots = total > 0 ? total : 1;
}

void TableSlot::set_strategies(StrategyInfo* info) {
    if (strategies == info) {
        return;
    }
    if (strategies) {
        disconnect(
            strategies, &StrategyInfo::new_strategy, this,
            &TableSlot::on_new_strategy
        );
    }
    strategies = info;
    if (strategies) {
        connect(
            strategies, &StrategyInfo::new_strategy, this,
            &TableSlot::on_new_strategy
        );
        on_new_strategy();
    }
}

void TableSlot::on_new_strategy() const {
    QStringList items;
    for (const auto& item : strategies->get_strategies()) {
        items.push_back(item->get_name());
    }
    items.pop_back(); // last is fake
    strategy_box->clear();
    strategy_box->addItems(items);
}

void TableSlot::on_strategy_changed(const int index) {
    if (index >= 0) {
        strategy = strategies->get_strategy_by_id(index);
        strategy_hint_label->setText(strategy->get_name());
    }
}

void TableSlot::paintEvent(QPaintEvent* event) {
    Cards::paintEvent(event);

    QPainter painter(this);
    const Settings& opts = Settings::instance();
    QColor highlight = opts.card_border();
    QColor base = Qt::gray; // opts.card_background();
    QColor mix = QColor::fromRgbF(
        base.redF() + (highlight.redF() - base.redF()) * highlight_opacity,
        base.greenF()
            + (highlight.greenF() - base.greenF()) * highlight_opacity,
        base.blueF() + (highlight.blueF() - base.blueF()) * highlight_opacity
    );
    painter.setPen(QPen(mix, 6));
    painter.drawRoundedRect(rect().adjusted(3, 3, -3, -3), 3, 3);
}

void TableSlot::set_highlight_opacity(const float value) {
    highlight_opacity = value;
    update();
}

float TableSlot::get_highlight_opacity() const { return highlight_opacity; }

void TableSlot::start_highlight() {
    highlight_anim->stop();
    set_highlight_opacity(1.0);
    highlight_anim->setStartValue(1.0);
    highlight_anim->setEndValue(0.0);
    highlight_anim->start();
}
