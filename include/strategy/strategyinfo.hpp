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

#ifndef CARD_COUNTER_STRATEGYINFO_HPP
#define CARD_COUNTER_STRATEGYINFO_HPP

// Qt
#include <QDialog>
#include <memory>

class Strategy;

class QSvgRenderer;

class QLabel;

class QSpinBox;

class QPushButton;

class QLineEdit;

class QTextEdit;

class QListWidget;

class KConfigGroup;

/**
 * @brief Dialog for browsing and editing card counting strategies.
 */
class StrategyInfo final : public QDialog {
    Q_OBJECT
public:
    explicit StrategyInfo(
        QSvgRenderer* renderer, QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags()
    );

    /**
     * @brief Retrieve a strategy object by its index.
     */
    Strategy* get_strategy_by_id(qint32 id) const;

    /**
     * @brief Display the strategy details for the given name.
     */
    void show_strategy_by_name(const QString& name);

    /**
     * @brief Access to the internal list of strategies.
     */
    const QVector<Strategy*>& get_strategies() const;

signals:

    void new_strategy();

private:
    QVector<Strategy*> items;
    QSvgRenderer* renderer;
    qint32 id;
    QLabel* name;
    QLabel* description;
    QLineEdit* name_input;
    QTextEdit* description_input;
    QPushButton* save_button;
    QListWidget* list_widget;
    QVector<QSpinBox*> weights;
    KConfigGroup* strategies_group;

    void init_strategies();

    void add_fake_strategy();

    void fill_list();
};

#endif // CARD_COUNTER_STRATEGYINFO_HPP
