#include <QApplication>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <memory>

#include "main_window.hpp"

#ifdef KC_KDE
#include <KAboutData>
#include <KLocalizedString>
#endif

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

#ifdef KC_KDE
    KLocalizedString::setApplicationDomain("kcuckounter");

    KAboutData about_data(
        QStringLiteral("kcuckounter"), QStringLiteral("kcuckounter"),
        QStringLiteral("1.0"),
        QStringLiteral("A tool for card counting training."),
        KAboutLicense::MIT, QStringLiteral("(c) 2025, Yaroslav Riabtsev"),
        QString(), QStringLiteral("https://github.com/ninjaro/kcuckounter"),
        QStringLiteral("yaroslav.riabtsev@rwth-aachen.de")
    );

    about_data.addAuthor(
        QStringLiteral("Yaroslav Riabtsev"), QStringLiteral("Original author"),
        QStringLiteral("yaroslav.riabtsev@rwth-aachen.de"),
        QStringLiteral("https://github.com/ninjaro"), QStringLiteral("ninjaro")
    );

    KAboutData::setApplicationData(about_data);

    QCommandLineParser parser;
    about_data.setupCommandLine(&parser);
    parser.process(app);
    about_data.processCommandLine(&parser);
#else
    QCoreApplication::setApplicationName(QStringLiteral("kcuckounter"));
    QCoreApplication::setOrganizationName(QStringLiteral("kcuckounter"));
    QCoreApplication::setApplicationVersion(QStringLiteral("1.0"));

    QCommandLineParser parser;
    parser.setApplicationDescription(
        QStringLiteral("A tool for card counting training.")
    );
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(app);
#endif

    auto window = std::make_unique<main_window>();
    window->show();

    int result = QApplication::exec();
    window.reset();
    return result;
}
