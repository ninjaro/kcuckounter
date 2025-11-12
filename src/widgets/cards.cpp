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
#include <QPainter>
#include <QRandomGenerator>
#include <QSvgRenderer>
// own
#include "widgets/cards.hpp"

// #include "settings.hpp"

QList<qint32> Cards::shuffle_cards(
    const qint32 deck_count, const qint32 shuffle_coefficient
) {
    QList<qint32> deck = generate_deck(deck_count);
    const qint32 threshold
        = static_cast<qint32>(deck.size()) / (deck_count * shuffle_coefficient);
    bool flag;

    do {
        flag = false;
        std::ranges::shuffle(deck, *QRandomGenerator::global());
        qint32 lastJokerIndex = -1;
        for (int i = 0; i < deck.size(); i++) {
            if (is_joker(deck[i])) {
                if (i - lastJokerIndex < threshold) {
                    flag = true;
                    break;
                }
                lastJokerIndex = i;
            }
        }
    } while (flag);

    return deck;
}

QString Cards::card_name(const qint32 id, const qint32 standard) {
    const qint32 rank = get_rank(id);
    const qint32 suit = get_suit(id);
    QString name = get_rank_name(rank, standard & 1);

    if (is_joker(id)) {
        name = get_colour_name(suit) + name;
    } else {
        name += get_suit_name(suit);
    }

    return name;
}

QList<qint32> Cards::generate_deck(const qint32 deck_count) {
    QList<qint32> deck;
    for (qint32 i = 0; i < deck_count; i++) {
        for (qint32 rank = Ace; rank <= King; rank++) {
            for (qint32 suit = Clubs; suit <= Spades; suit++) {
                deck.append(((suit & 0xff) << 8) | (rank & 0xff));
            }
        }
        for (int colour = Black; colour <= Red; colour++) {
            deck.append((colour & 0xff) << 8);
        }
    }
    return deck;
}

QString Cards::get_colour_name(const qint32 colour) {
    switch (colour) {
    case Black:
        return QStringLiteral("black_");
    case Red:
        return QStringLiteral("red_");
    default:
        return "";
    }
}

QString Cards::get_suit_name(const qint32 suit) {
    switch (suit) {
    case Clubs:
        return QStringLiteral("_club");
    case Diamonds:
        return QStringLiteral("_diamond");
    case Hearts:
        return QStringLiteral("_heart");
    case Spades:
        return QStringLiteral("_spade");
    default:
        return "";
    }
}

QString Cards::get_rank_name(const qint32 rank, const bool standard) {
    switch (rank) {
    case King:
        return QStringLiteral("king");
    case Queen:
        return QStringLiteral("queen");
    case Jack:
        return QStringLiteral("jack");
    case Joker: {
        if (standard) {
            return QStringLiteral("joker");
        }
        return QStringLiteral("jocker");
    }
    case Ace:
        if (standard) {
            return QStringLiteral("ace");
        }
        [[fallthrough]];
    default:
        return QString::number(rank);
    }
}

bool Cards::is_joker(const qint32 id) noexcept { return !get_rank(id); }

qint32 Cards::get_rank(const qint32 id) noexcept {
    return static_cast<rank>(id & 0xff);
}

qint32 Cards::get_suit(const qint32 id) noexcept { return (id >> 8) & 0xff; }

void Cards::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event)

    update_pixmap();

    QPainter painter(this);
    painter.drawPixmap(0, 0, pixmap);
}

void Cards::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    pixmap_dirty = true;
}

Cards::Cards(QSvgRenderer* renderer, QWidget* parent)
    : QWidget(parent)
    , svg_name("back")
    , current_card_id(-1)
    , renderer(renderer) {
    if (!renderer || !renderer->isValid()) {
        qCritical() << "Cards: invalid QSvgRenderer";
        return;
    }
    const auto elt = get_card_name_by_current_id(current_card_id);
    if (!renderer->elementExists(elt)) {
        qWarning() << "Cards: element missing in deck:" << elt;
        return;
    }
    setFixedSize(renderer->boundsOnElement("back").size().toSize());
    pixmap_dirty = true;
}

void Cards::set_id(const qint32 id) {
    current_card_id = id;
    set_name(card_name(current_card_id));
}

void Cards::set_name(QString name) {
    svg_name = std::move(name);
    pixmap_dirty = true;
}

QString Cards::get_card_name_by_current_id(const qint32 standard) const {
    return card_name(current_card_id, standard);
}

bool Cards::is_joker() const noexcept { return is_joker(current_card_id); }

qint32 Cards::get_current_rank() const noexcept {
    return get_rank(current_card_id);
}

void Cards::set_rotated(const bool rotated) {
    if (rotated_svg != rotated) {
        rotated_svg = rotated;
        pixmap_dirty = true;
        update();
    }
}

void Cards::set_renderer(QSvgRenderer* r) {
    renderer = r;
    setFixedSize(renderer->boundsOnElement("back").size().toSize());
    pixmap_dirty = true;
    update();
}

void Cards::update_pixmap() {
    if (!pixmap_dirty) {
        return;
    }
    pixmap = QPixmap(size());
    pixmap.fill(Qt::transparent);
    if (renderer->isValid() && renderer->elementExists(svg_name)) {
        QPainter p(&pixmap);
        if (rotated_svg) {
            p.translate(width() / 2.0, height() / 2.0);
            p.rotate(90);
            p.translate(-height() / 2.0, -width() / 2.0);
            renderer->render(
                &p, svg_name,
                QRectF(
                    0, 0, static_cast<qreal>(height()),
                    static_cast<qreal>(width())
                )
            );
        } else {
            renderer->render(
                &p, svg_name, QRectF(QPointF(0, 0), QSizeF(size()))
            );
        }
    }
    pixmap_dirty = false;
}

//
// void Cards::onResized(QSize newFixedSize) {
//    if (size() != newFixedSize) {
//        setFixedSize(newFixedSize);
//    }
//}
