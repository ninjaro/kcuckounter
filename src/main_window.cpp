#include "main_window.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QWidget>

main_window::main_window(QWidget* parent)
    : QMainWindow(parent)
    , table_slots_count_spin_box(nullptr)
    , training_check_box(nullptr)
    , quiz_type_combo_box(nullptr)
    , dealing_mode_combo_box(nullptr)
    , continue_button(nullptr) {
    setup_ui();
}

main_window::~main_window() = default;

void main_window::setup_ui() {
    auto central_widget = new QWidget(this);
    auto main_layout = new QVBoxLayout;
    auto form_layout = new QFormLayout;

    table_slots_count_spin_box = new QSpinBox(central_widget);
    table_slots_count_spin_box->setMinimum(1);
    table_slots_count_spin_box->setMaximum(16);
    table_slots_count_spin_box->setValue(4);

    training_check_box
        = new QCheckBox(QStringLiteral("Training"), central_widget);
    training_check_box->setChecked(true);

    quiz_type_combo_box = new QComboBox(central_widget);
    quiz_type_combo_box->addItems(
        QStringList() << QStringLiteral("No jokers; global pause; single slot")
                      << QStringLiteral("Jokers in deck; local pause per slot")
                      << QStringLiteral("No jokers; global pause; all slots")
    );

    dealing_mode_combo_box = new QComboBox(central_widget);
    dealing_mode_combo_box->addItems(
        QStringList() << QStringLiteral("Sequential")
                      << QStringLiteral("Random")
                      << QStringLiteral("Simultaneous")
    );

    form_layout->addRow(
        QStringLiteral("Table slots count"), table_slots_count_spin_box
    );
    form_layout->addRow(QStringLiteral("Training"), training_check_box);
    form_layout->addRow(QStringLiteral("Quiz type"), quiz_type_combo_box);
    form_layout->addRow(QStringLiteral("Dealing mode"), dealing_mode_combo_box);

    continue_button
        = new QPushButton(QStringLiteral("Continue"), central_widget);

    main_layout->addLayout(form_layout);
    main_layout->addWidget(continue_button);
    main_layout->addStretch();

    central_widget->setLayout(main_layout);
    setCentralWidget(central_widget);
    setWindowTitle(QStringLiteral("kcuckounter"));
}
