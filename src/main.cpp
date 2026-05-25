#include "cli/CommandDispatcher.h"

#ifdef QKAGE_ENABLE_QT_UI
#include <QApplication>
#include <QTimer>
#include <QtGlobal>

#include <string>

#include "ui/MainWindow.h"

namespace {
bool isGuiArgument(const char *argument)
{
    const std::string value = argument == nullptr ? "" : argument;
    return value == "--gui" || value == "gui";
}
}
#endif

int main(int argc, char *argv[])
{
#ifdef QKAGE_ENABLE_QT_UI
    const bool explicitGui = argc == 2 && isGuiArgument(argv[1]);
    if (explicitGui) {
        QApplication app(argc, argv);
        MainWindow window;
        window.showMaximized();

        bool ok = false;
        const int exitAfterMs = qEnvironmentVariableIntValue("QKAGE_QT_EXIT_AFTER_MS", &ok);
        if (ok && exitAfterMs >= 0) {
            QTimer::singleShot(exitAfterMs, &app, &QApplication::quit);
        }

        return app.exec();
    }
#endif

    CommandDispatcher dispatcher;
    return dispatcher.dispatch(argc, argv);
}
