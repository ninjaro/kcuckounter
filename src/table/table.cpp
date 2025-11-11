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
#include <QRandomGenerator>
#include <QStandardPaths>
#include <QSvgRenderer>
#include <QTimer>
#include <QVBoxLayout>
#include <QtMath>
// own
#include "strategy/strategyinfo.hpp"
#include "table/table.hpp"
#include "table/tableslot.hpp"

Table::Table(QWidget* parent)
    : QWidget(parent) {
    countdown = new QTimer(this);
    connect(countdown, &QTimer::timeout, this, &Table::pick_up_cards);

    layout = new QGridLayout();
    setLayout(layout);

    set_card_theme("tigullio-international");
}

void Table::set_speed(const int interval_ms) const {
    countdown->setInterval(interval_ms);
}

void Table::on_table_slot_activated() {
    const auto* table_slot = qobject_cast<TableSlot*>(sender());
    available.insert(layout->indexOf(table_slot));
    add_new_table_slot();
    calculate_new_column_count(
        size(), bounds.size(), static_cast<qint32>(items.count())
    );
    emit can_remove(table_slot_count_limit < available.size());
}

void Table::add_new_table_slot(const bool is_active) {
    auto* table_slot = new TableSlot(strategy_info, renderer, is_active, this);
    if (is_active) {
        available.insert(static_cast<qint32>(items.size()));
    }
    connect(
        table_slot, &TableSlot::table_slot_activated, this,
        &Table::on_table_slot_activated
    );
    connect(
        table_slot, &TableSlot::table_slot_finished, this,
        &Table::on_table_slot_finished
    );
    connect(
        table_slot, &TableSlot::table_slot_removed, this,
        &Table::on_table_slot_removed
    );
    connect(
        table_slot, &TableSlot::table_slot_reshuffled, this,
        &Table::on_table_slot_reshuffled
    );
    connect(
        table_slot, &TableSlot::user_quizzed, this, &Table::on_user_quizzed
    );
    connect(
        table_slot, &TableSlot::user_answered, this, &Table::on_user_answered
    );
    connect(
        table_slot, &TableSlot::swap_target_selected, this,
        &Table::on_swap_target_selected
    );
    connect(
        table_slot, &TableSlot::strategy_info_assist, this,
        &Table::on_strategy_info_assist
    );
    connect(this, &Table::game_paused, table_slot, &TableSlot::on_game_paused);
    connect(
        this, &Table::table_slot_resized, table_slot,
        [table_slot](const QSize newFixedSize) {
            table_slot->setFixedSize(newFixedSize);
            table_slot->set_rotated(
                newFixedSize.width() > newFixedSize.height()
            );
        }
    );
    connect(this, &Table::can_remove, table_slot, &TableSlot::on_can_remove);
    items.push_back(table_slot);
}

void Table::on_table_slot_finished() {
    const auto* table_slot = qobject_cast<TableSlot*>(sender());
    available.remove(layout->indexOf(table_slot));
    //    qDebug() << available;
}

void Table::on_table_slot_removed() {
    const auto* table_slot = qobject_cast<TableSlot*>(sender());
    items.remove(layout->indexOf(table_slot));
    available.remove(layout->indexOf(table_slot));
    jokers.remove(layout->indexOf(table_slot));
    calculate_new_column_count(
        size(), bounds.size(), static_cast<qint32>(items.count())
    );
    emit can_remove(available.size() > table_slot_count_limit);
}

void Table::on_table_slot_reshuffled() {
    const auto* table_slot = qobject_cast<TableSlot*>(sender());
    available.insert(layout->indexOf(table_slot));
}

void Table::on_user_quizzed() {
    countdown->stop();
    const auto* table_slot = qobject_cast<TableSlot*>(sender());
    jokers.insert(layout->indexOf(table_slot));
    available.remove(layout->indexOf(table_slot));
}

void Table::on_user_answered(const bool correct) {
    const auto* table_slot = qobject_cast<TableSlot*>(sender());
    jokers.remove(layout->indexOf(table_slot));
    available.insert(layout->indexOf(table_slot));
    if (jokers.empty()) {
        countdown->stop();
        countdown->start(countdown->interval());
    }
    emit score_update(correct);
}

void Table::calculate_new_column_count(
    const QSizeF& table_size, const QSizeF& aspect_ratio, int item_count
) {
    int best_column_count = 1;
    double best_scale = 0.0;
    bool rotated = false;

    auto test_orientation = [&](const QSizeF& size, const bool swapped) {
        for (int column_count = 1; column_count <= item_count; column_count++) {
            const int row_count
                = qCeil(static_cast<double>(item_count) / column_count);
            const double scale = 0.9
                * qMin(size.width() / (column_count * aspect_ratio.width()),
                       size.height() / (row_count * aspect_ratio.height()));
            if (scale > best_scale) {
                best_scale = scale;
                best_column_count = column_count;
                rotated = swapped;
            }
        }
    };

    test_orientation(table_size, false);
    test_orientation(QSizeF(table_size.height(), table_size.width()), true);
    reorganize_table(best_column_count, best_scale, rotated);
}

void Table::reorganize_table(
    const qint32 new_column_count, const double new_scale,
    const bool new_rotated
) {
    // for (TableSlot* item : std::as_const(items)) {
    //     layout->removeWidget(item);
    // }
    while (layout->count()) {
        const QLayoutItem* item = layout->takeAt(0);
        item->widget()->hide();
        delete item;
    }

    const QSizeF new_fixed_size(
        new_rotated ? bounds.height() * new_scale : bounds.width() * new_scale,
        new_rotated ? bounds.width() * new_scale : bounds.height() * new_scale
    );
    emit table_slot_resized(new_fixed_size.toSize());

    const qint32 items_count = static_cast<qint32>(items.count());
    for (qint32 i = 0; i < items_count; i++) {
        TableSlot* item = items[i];
        layout->addWidget(item, i / new_column_count, i % new_column_count);
        item->set_infinite_params(i, items_count);
        item->show();
    }
    column_count = new_column_count;
    scale = new_scale;
    rotated = new_rotated;
}

void Table::on_swap_target_selected() {
    swap_target.push_back(layout->indexOf(qobject_cast<TableSlot*>(sender())));
    if (swap_target.size() == 2) {
        items.swapItemsAt(swap_target[0], swap_target[1]);
        swap_target.clear();
    }
    reorganize_table(column_count, scale, rotated);
}

void Table::pick_up_cards() {
    //    qDebug() << available;
    if (available.empty()) {
        countdown->stop();
        emit game_over();
        return;
    }
    if (mode == card_mode::Simultaneous) {
        for (const qint32 key : available) {
            items[key]->pick_up_card();
        }
        return;
    }

    qint32 key;
    if (mode == card_mode::Random) {

        do {
            const qint32 idx = QRandomGenerator::global()->bounded(
                static_cast<qint32>(available.size())
            );
            {
                auto it = available.begin();
                std::advance(it, idx);
                key = *it;
            }
        } while (available.size() > 1 && key == order_index);
        order_index = key;
    } else {
        do {
            key = order_index++;
            order_index %= static_cast<qint32>(items.size());
        } while (!available.contains(key));
    }

    items[key]->pick_up_card();
}

void Table::set_card_theme(const QString& theme) {
    const QString path = QStandardPaths::locate(
        QStandardPaths::GenericDataLocation,
        QString("carddecks/svg-%1/%1.svgz").arg(theme)
    );
    if (path.isEmpty()) {
        qWarning("Card theme '%s' could not be found.", qPrintable(theme));
        return;
    }
    current_theme = theme;
    delete renderer;
    renderer = new QSvgRenderer(path);
    bounds = renderer->boundsOnElement("back");
    const StrategyInfo* old_strategy_info = strategy_info;
    strategy_info = new StrategyInfo(renderer);
    for (TableSlot* slot : items) {
        slot->set_renderer(renderer);
        slot->set_strategies(strategy_info);
    }
    delete old_strategy_info;
    calculate_new_column_count(
        size(), bounds.size(), static_cast<qint32>(items.count())
    );
}

void Table::create_new_game(const int level) {
    countdown->stop();
    launching = true;
    while (!items.empty()) {
        TableSlot* last = items.last();
        last->hide();
        items.pop_back();
        delete last;
    }
    available.clear();
    jokers.clear();
    order_index = 0;
    table_slot_count_limit = 1;
    switch (static_cast<qint32>(level)) {
    case 1:
        mode = card_mode::Ordered;
        break;
    case 3:
        mode = card_mode::Random;
        break;
    case 10:
        mode = card_mode::Simultaneous;
        break;
    default:
        break;
    }
    add_new_table_slot();
    calculate_new_column_count(
        size(), bounds.size(), static_cast<qint32>(items.count())
    );
}

bool Table::is_launching() const { return launching; }

void Table::set_card_mode(const int level) {
    switch (static_cast<qint32>(level)) {
    case 1:
        mode = card_mode::Ordered;
        break;
    case 3:
        mode = card_mode::Random;
        break;
    case 10:
        mode = card_mode::Simultaneous;
        break;
    default:
        break;
    }
}

void Table::pause(const bool paused) {
    if (launching && !paused) {
        if (available.isEmpty()) {
            emit game_paused(true);
            return;
        }
        launching = false;
        if (const TableSlot* last = items.last(); last->is_fake()) {
            items.pop_back();
            delete last;
            calculate_new_column_count(size(), bounds.size(), layout->count());
        }
    }
    emit game_paused(paused);
    if (paused) {
        countdown->stop();
    } else if (jokers.empty()) {
        countdown->stop();
        countdown->start(countdown->interval());
    }
}

void Table::force_game_over() {
    countdown->stop();
    emit game_over();
}

void Table::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);

    calculate_new_column_count(
        size(), bounds.size(), static_cast<qint32>(items.count())
    );
}

void Table::on_strategy_info_assist() const { strategy_info->show(); }
