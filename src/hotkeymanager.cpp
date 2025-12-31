#include "hotkeymanager.h"

#include <QAction>
#include <QKeySequence>
#include <KGlobalAccel>

HotkeyManager::HotkeyManager(QObject* parent)
    : QObject(parent)
    , m_action(nullptr)
    , m_enabled(true)
    , m_registered(false)
{
}

HotkeyManager::~HotkeyManager()
{
    unregisterHotkey();
}

bool HotkeyManager::registerHotkey(const QString& key, Qt::KeyboardModifiers modifiers)
{
    unregisterHotkey();

    // Create the action
    m_action = new QAction(this);
    m_action->setObjectName(QStringLiteral("clickpaste_trigger"));
    m_action->setText(QStringLiteral("ClickPaste Trigger"));

    connect(m_action, &QAction::triggered, this, &HotkeyManager::onActionTriggered);

    // Build the key sequence
    int keyCode = 0;
    if (!key.isEmpty()) {
        keyCode = QKeySequence(key)[0].key();
    }

    QKeySequence shortcut(modifiers | keyCode);

    // Register with KGlobalAccel
    KGlobalAccel::self()->setGlobalShortcut(m_action, {shortcut});

    // Check if registration succeeded
    QList<QKeySequence> registeredShortcuts = KGlobalAccel::self()->globalShortcut(m_action);
    if (registeredShortcuts.isEmpty() || registeredShortcuts.first().isEmpty()) {
        Q_EMIT registrationFailed(QStringLiteral("Failed to register global hotkey. It may be in use by another application."));
        delete m_action;
        m_action = nullptr;
        m_registered = false;
        return false;
    }

    m_registered = true;
    return true;
}

void HotkeyManager::unregisterHotkey()
{
    if (m_action) {
        KGlobalAccel::self()->removeAllShortcuts(m_action);
        delete m_action;
        m_action = nullptr;
    }
    m_registered = false;
}

bool HotkeyManager::isRegistered() const
{
    return m_registered;
}

void HotkeyManager::setEnabled(bool enabled)
{
    m_enabled = enabled;
}

bool HotkeyManager::isEnabled() const
{
    return m_enabled;
}

void HotkeyManager::onActionTriggered()
{
    if (m_enabled) {
        Q_EMIT hotkeyTriggered();
    }
}
