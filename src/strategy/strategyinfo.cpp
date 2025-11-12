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
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QSvgRenderer>
#include <QTextEdit>
// own
#include "compat/config_shim.hpp"
#include "compat/i18n_shim.hpp"
#include "strategy/strategy.hpp"
#include "strategy/strategyinfo.hpp"
#include "widgets/cards.hpp"
#include "widgets/carousel.hpp"

StrategyInfo::StrategyInfo(
    QSvgRenderer* renderer, QWidget* parent, const Qt::WindowFlags flags
)
    : QDialog(parent, flags)
    , renderer(renderer)
    , id(0) {
    setWindowTitle("Strategy Info");
    setModal(true);

    strategies_group
#ifdef KC_KDE
        = new KConfigGroup(KSharedConfig::openConfig(), "CCStrategies");
#else
        = new KConfigGroup("CCStrategies");
#endif

    init_strategies();

    auto* dialog_buttons = new QDialogButtonBox();
    save_button
        = new QPushButton(QIcon::fromTheme("document-save"), i18n("&Save"));
    auto* window = new QHBoxLayout(this);
    auto* left_panel = new QWidget;
    auto* left_panel_layout = new QVBoxLayout(left_panel);
    auto* search_box = new QLineEdit();
    list_widget = new QListWidget();
    auto* right_panel = new QWidget;
    auto* body = new QVBoxLayout(right_panel);
    auto* carousel = new Carousel(renderer->boundsOnElement("back").size());
    name = new QLabel(items[id]->get_name());
    description = new QLabel(items[id]->get_description());
    name_input = new QLineEdit();
    description_input = new QTextEdit();
    auto* title = new QWidget;
    auto* title_layout = new QHBoxLayout(title);
    auto* browser = new QWidget;
    auto* browser_layout = new QHBoxLayout(browser);

    left_panel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
    search_box->setPlaceholderText(tr("Search"));
    list_widget->setSelectionMode(QAbstractItemView::SingleSelection);
    list_widget->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    list_widget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    description->setWordWrap(true);
    description->setTextFormat(Qt::TextFormat::MarkdownText);
    description->setOpenExternalLinks(true);
    name->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    name_input->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    description->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    description_input->setSizePolicy(
        QSizePolicy::Expanding, QSizePolicy::Fixed
    );
    dialog_buttons->setStandardButtons(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel
    );

    dialog_buttons->addButton(save_button, QDialogButtonBox::ActionRole);
    left_panel_layout->addWidget(search_box);
    title_layout->addWidget(name_input);
    title_layout->addWidget(name);
    browser_layout->addWidget(description_input);
    browser_layout->addWidget(description);
    left_panel_layout->addWidget(list_widget);
    window->addWidget(left_panel);
    window->addWidget(right_panel);
    body->addWidget(title);
    body->addWidget(browser);
    for (int i = Cards::rank::Ace; i <= Cards::rank::King; i++) {
        auto* card = new Cards(renderer);
        auto* form = new QFormLayout(card);
        auto* spin = new QSpinBox();

        card->set_id(i);
        spin->setRange(-5, 5);
        spin->setValue(items[id]->get_weights(i - Cards::rank::Ace));
        spin->setReadOnly(!items[id]->is_custom());
        spin->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        form->setFormAlignment(Qt::AlignCenter);
        form->addRow(spin);
        carousel->add_widget(card);
        weights.push_back(spin);
    }
    body->addWidget(carousel);
    body->addStretch();
    body->addWidget(dialog_buttons);
    fill_list();

    list_widget->setCurrentItem(list_widget->item(0));
    connect(save_button, &QPushButton::clicked, this, [this] {
        // todo: check if the name is new
        QVector<qint32> currentWeights;
        for (const auto& weight : weights) {
            currentWeights.push_back(weight->value());
        }
        items[id] = new Strategy(
            name->text(), description->text(), currentWeights, true
        );
        KConfigGroup strategyGroup
            = strategies_group->group(items[id]->get_name());
        strategyGroup.writeEntry("description", items[id]->get_description());
        strategyGroup.writeEntry("weights", currentWeights.toList());
        strategies_group->config()->sync();
        list_widget->currentItem()->setText(name->text());
        add_fake_strategy();
        emit new_strategy();
    });
    connect(list_widget, &QListWidget::itemSelectionChanged, this, [this] {
        show_strategy_by_name(list_widget->currentItem()->text());
    });
    connect(
        search_box, &QLineEdit::textChanged, this, [this](const QString& text) {
            for (int i = 0; i < list_widget->count(); i++) {
                auto* item = list_widget->item(i);
                const bool matches
                    = item->text().contains(text, Qt::CaseInsensitive);
                item->setHidden(!matches);
            }
        }
    );
    connect(
        name_input, &QLineEdit::textChanged, this,
        [this](const QString& text) { name->setText(text); }
    );
    connect(description_input, &QTextEdit::textChanged, this, [this] {
        description->setText(description_input->toMarkdown());
    });
    connect(
        dialog_buttons, &QDialogButtonBox::accepted, this, &QDialog::accept
    );
    connect(
        dialog_buttons, &QDialogButtonBox::rejected, this, &QDialog::reject
    );

    save_button->hide();
    name_input->hide();
    description_input->hide();
}

Strategy* StrategyInfo::get_strategy_by_id(const qint32 id) const {
    if (id < 0 || id >= items.size()) {
        qWarning() << "get_strategy_by_id: out of range:" << id
                   << "(size:" << items.size() << ")";
        return nullptr;
    }
    return items[id];
}

void StrategyInfo::show_strategy_by_name(const QString& name) {
    qint32 id = -1;
    for (int i = 0; i < items.size(); i++) {
        if (items[i]->get_name() == name) {
            id = i;
            break;
        }
    }
    if (id != -1 && this->id != id) {
        this->id = id;
        this->name->setText(items[id]->get_name());
        description->setText(items[id]->get_description());
        if (id + 1 < items.size()) {
            name_input->setText(this->name->text());
            description_input->setText(description->text());
        }
        const bool is_custom = items[id]->is_custom();
        description_input->setHidden(!is_custom);
        name_input->setHidden(!is_custom);
        save_button->setHidden(!is_custom);
        for (int i = Cards::rank::Ace; i <= Cards::rank::King; i++) {
            weights[i - Cards::rank::Ace]->setValue(
                items[id]->get_weights(i - Cards::rank::Ace)
            );
            weights[i - Cards::rank::Ace]->setReadOnly(!is_custom);
        }
    }
}

const QVector<Strategy*>& StrategyInfo::get_strategies() const { return items; }

void StrategyInfo::init_strategies() {

    items.push_back(new Strategy(
        "Hi-Opt I Count",
        "The Hi-Opt I blackjack card counting system was developed by Charles "
        "Einstein and introduced in his book "
        "\"The World's Greatest Blackjack Book\" in 1980. The Hi-Opt I system "
        "assigns point values to each card in "
        "the deck and is a more complex system than the Hi-Lo system, with "
        "additional point values for some cards. "
        "It is considered a more powerful system than the Hi-Lo, but also more "
        "difficult to learn "
        "and use effectively.",
        { 0, 0, 1, 1, 1, 1, 0, 0, 0, -1, -1, -1, -1 }
    ));

    items.push_back(new Strategy(
        "Hi-Lo Count",
        "The Hi-Lo blackjack card counting system was first introduced by "
        "Harvey Dubner in 1963. Dubner's goal was "
        "to create a simple yet effective system that could be used by anyone "
        "to increase their odds of winning "
        "at blackjack.",
        { -1, 1, 1, 1, 1, 1, 0, 0, 0, -1, -1, -1, -1 }
    ));

    items.push_back(new Strategy(
        "Hi-Opt II Count",
        "The Hi-Opt II blackjack card counting system is a more advanced "
        "version of the Hi-Opt I system, "
        "developed by Lance Humble and Carl Cooper in their book \"The World's "
        "Greatest Blackjack Book\" in 1980. "
        "The Hi-Opt II system assigns point values to each card in the deck, "
        "with additional point values "
        "for some cards, and is considered one of the most powerful card "
        "counting systems. It is also one of "
        "the most difficult to learn and use effectively.",
        { 0, 1, 1, 2, 2, 1, 1, 0, 0, -2, -2, -2, -2 }
    ));

    items.push_back(new Strategy(
        "KO Count",
        "The Knock-Out (KO) blackjack card counting system was developed by "
        "Olaf Vancura and Ken Fuchs in their "
        "book \"Knock-Out Blackjack\" in 1998. The KO system assigns point "
        "values to each card in the deck, with "
        "the additional advantage that it does not require a true count "
        "conversion for betting, making it easier "
        "to use than some other systems.",
        { -1, 1, 1, 1, 1, 1, 1, 0, 0, -1, -1, -1, -1 }
    ));

    items.push_back(new Strategy(
        "Omega II Count",
        "The Omega II blackjack card counting system was developed by Bryce "
        "Carlson and introduced in his book "
        "\"Blackjack for Blood\" in 2001. The Omega II system assigns point "
        "values to each card in the deck, with "
        "additional point values for some cards, and is considered one of the "
        "most powerful card counting systems, "
        "especially for multi-deck games.",
        { 0, 1, 1, 2, 2, 2, 1, 0, -1, -2, -2, -2, -2 }
    ));

    items.push_back(new Strategy(
        "Zen Count",
        "The Zen Count blackjack card counting system was developed by Arnold "
        "Snyder and introduced in his book "
        "\"Blackbelt in Blackjack\" in 1983. The Zen Count system assigns "
        "point values to each card in the deck, "
        "with additional point values for some cards, and is considered a "
        "powerful system for both single "
        "and multi-deck games.",
        { -1, 1, 1, 2, 2, 2, 1, 0, 0, -2, -2, -2, -2 }
    ));

    items.push_back(new Strategy(
        "10 Count",
        "The 10 Count blackjack card counting system was developed by Edward "
        "O. Thorp, a mathematician and author "
        "of the classic book \"Beat the Dealer\" in 1962. The 10 Count system "
        "assigns point values to each card in "
        "the deck, with a focus on the 10-value cards, and is considered one "
        "of the earliest "
        "and most basic card counting systems.",
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, -2, -2, -2, -2 }
    ));

    // QStringList strategyNames = strategiesGroup->groupList();
    for (const auto& strategyName : strategies_group->groupList()) {
        KConfigGroup strategyGroup = strategies_group->group(strategyName);
        items.push_back(new Strategy(
            strategyName, strategyGroup.readEntry("description", ""),
            QVector<int>::fromList(
                strategyGroup.readEntry("weights", QList<int>())
            ),
            true
        ));
    }
}

void StrategyInfo::add_fake_strategy() {
    items.push_back(new Strategy(
        "New Strategy", "Some Notes (use Markdown)",
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, true
    ));
    list_widget->sortItems();
    auto* widgetItem = new QListWidgetItem(items.last()->get_name());
    list_widget->addItem(widgetItem);
}

void StrategyInfo::fill_list() {
    for (const auto& item : items) {
        auto* widgetItem = new QListWidgetItem(item->get_name());
        list_widget->addItem(widgetItem);
    }
    add_fake_strategy();
}
