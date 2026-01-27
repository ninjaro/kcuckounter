#ifndef KCUCKOUNTER_CARD_SHEET_TESTS_HPP
#define KCUCKOUNTER_CARD_SHEET_TESTS_HPP

#include <QObject>

class card_sheet_tests : public QObject {
    Q_OBJECT

private slots:
    void loads_svg();
    void contains_expected_elements();
};

#endif // KCUCKOUNTER_CARD_SHEET_TESTS_HPP
