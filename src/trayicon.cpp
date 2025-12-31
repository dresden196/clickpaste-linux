#include "trayicon.h"

#include <QMenu>
#include <QAction>
#include <QIcon>
#include <QFile>
#include <QSettings>

TrayIcon::TrayIcon(QObject* parent)
    : QObject(parent)
    , m_trayIcon(new QSystemTrayIcon(this))
    , m_contextMenu(nullptr)
    , m_settingsAction(nullptr)
    , m_exitAction(nullptr)
    , m_iconState(Normal)
{
    createContextMenu();
    updateIcon();

    m_trayIcon->setToolTip(QStringLiteral("ClickPaste: Click to choose a target"));

    connect(m_trayIcon, &QSystemTrayIcon::activated,
            this, &TrayIcon::onActivated);
}

TrayIcon::~TrayIcon()
{
    delete m_contextMenu;
}

void TrayIcon::show()
{
    m_trayIcon->show();
}

void TrayIcon::hide()
{
    m_trayIcon->hide();
}

void TrayIcon::setIconState(IconState state)
{
    if (m_iconState != state) {
        m_iconState = state;
        updateIcon();
    }
}

void TrayIcon::showMessage(const QString& title, const QString& message,
                           QSystemTrayIcon::MessageIcon icon)
{
    m_trayIcon->showMessage(title, message, icon);
}

void TrayIcon::onActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger) {
        emit activated();
    }
}

void TrayIcon::createContextMenu()
{
    m_contextMenu = new QMenu();

    m_settingsAction = m_contextMenu->addAction(QStringLiteral("Settings..."));
    connect(m_settingsAction, &QAction::triggered, this, &TrayIcon::settingsRequested);

    m_contextMenu->addSeparator();

    m_exitAction = m_contextMenu->addAction(QStringLiteral("Exit"));
    connect(m_exitAction, &QAction::triggered, this, &TrayIcon::exitRequested);

    m_trayIcon->setContextMenu(m_contextMenu);
}

void TrayIcon::updateIcon()
{
    QString iconName;
    bool dark = isDarkTheme();

    switch (m_iconState) {
    case Normal:
        iconName = dark ? QStringLiteral(":/icons/clickpaste-dark.svg")
                        : QStringLiteral(":/icons/clickpaste.svg");
        break;
    case Typing:
        iconName = QStringLiteral(":/icons/clickpaste-typing.svg");
        break;
    case Targeting:
        iconName = dark ? QStringLiteral(":/icons/clickpaste-dark.svg")
                        : QStringLiteral(":/icons/clickpaste.svg");
        break;
    }

    // Try to load from resources first, then fall back to theme
    if (QFile::exists(iconName)) {
        m_trayIcon->setIcon(QIcon(iconName));
    } else {
        // Fallback to a generic icon
        m_trayIcon->setIcon(QIcon::fromTheme(QStringLiteral("edit-paste")));
    }
}

bool TrayIcon::isDarkTheme() const
{
    // Try to detect KDE Plasma color scheme
    QSettings settings(QStringLiteral("kdeglobals"), QSettings::NativeFormat);
    QString colorScheme = settings.value(QStringLiteral("General/ColorScheme")).toString();

    // Check if it's a dark theme by name or check the window background
    if (colorScheme.contains(QStringLiteral("Dark"), Qt::CaseInsensitive) ||
        colorScheme.contains(QStringLiteral("Breeze Dark"), Qt::CaseInsensitive)) {
        return true;
    }

    // Alternative: check background color luminance
    // For now, default to light theme if we can't determine
    return false;
}
