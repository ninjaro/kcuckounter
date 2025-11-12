#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include <QMainWindow>

class QCheckBox;
class QComboBox;
class QSpinBox;
class QPushButton;

class main_window : public QMainWindow {
    Q_OBJECT

public:
    explicit main_window(QWidget* parent = nullptr);
    ~main_window() override;

private:
    QSpinBox* table_slots_count_spin_box;
    QCheckBox* training_check_box;
    QComboBox* quiz_type_combo_box;
    QComboBox* dealing_mode_combo_box;
    QPushButton* continue_button;

    void setup_ui();
};

#endif
