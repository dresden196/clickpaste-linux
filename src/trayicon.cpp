#include "trayicon.h"

#include <QMenu>
#include <QAction>
#include <QIcon>
#include <QFile>
#include <QSettings>
#include <QPalette>
#include <QApplication>

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
        Q_EMIT activated();
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
    case Targeting:
        // Use light icon on dark themes, dark icon on light themes
        iconName = dark ? QStringLiteral(":/icons/clickpaste-dark.svg")
                        : QStringLiteral(":/icons/clickpaste.svg");
        break;
    case Typing:
        iconName = QStringLiteral(":/icons/clickpaste-typing.svg");
        break;
    }

    QIcon icon(iconName);
    if (!icon.isNull()) {
        m_trayIcon->setIcon(icon);
    } else {
        // Fallback to a generic icon
        m_trayIcon->setIcon(QIcon::fromTheme(QStringLiteral("edit-paste")));
    }
}

bool TrayIcon::isDarkTheme() const
{
    // Check the window background color luminance
    // This is the most reliable method across different desktops
    QPalette palette = QApplication::palette();
    QColor bg = palette.color(QPalette::Window);

    // Calculate relative luminance
    // Dark theme if background luminance is low
    int luminance = (bg.red() * 299 + bg.green() * 587 + bg.blue() * 114) / 1000;
    return luminance < 128;
}
