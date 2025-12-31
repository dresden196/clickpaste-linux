#include "application.h"

#include <QApplication>
#include <QDebug>

int main(int argc, char* argv[])
{
    // Set application metadata
    QApplication::setApplicationName(QStringLiteral("ClickPaste"));
    QApplication::setApplicationVersion(QStringLiteral("1.0.0"));
    QApplication::setOrganizationName(QStringLiteral("ClickPaste"));
    QApplication::setOrganizationDomain(QStringLiteral("clickpaste.app"));

    // Prefer Wayland but fall back to X11 if needed
    // Note: On pure Wayland, this is ignored
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QApplication app(argc, argv);

    // Don't quit when last window closes (we're a tray app)
    app.setQuitOnLastWindowClosed(false);

    // Create and initialize the application
    Application clickPaste;
    if (!clickPaste.initialize()) {
        qCritical() << "Failed to initialize ClickPaste";
        return 1;
    }

    return app.exec();
}
