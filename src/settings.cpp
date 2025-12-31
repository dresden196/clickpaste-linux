#include "settings.h"

Settings* Settings::s_instance = nullptr;

Settings* Settings::instance()
{
    if (!s_instance) {
        s_instance = new Settings();
    }
    return s_instance;
}

Settings::Settings(QObject* parent)
    : QObject(parent)
    , m_settings(QStringLiteral("ClickPaste"), QStringLiteral("ClickPaste"))
{
}

int Settings::keyDelayMs() const
{
    return m_settings.value(QStringLiteral("keyDelayMs"), 15).toInt();
}

void Settings::setKeyDelayMs(int ms)
{
    if (keyDelayMs() != ms) {
        m_settings.setValue(QStringLiteral("keyDelayMs"), ms);
        Q_EMIT settingsChanged();
    }
}

int Settings::startDelayMs() const
{
    return m_settings.value(QStringLiteral("startDelayMs"), 0).toInt();
}

void Settings::setStartDelayMs(int ms)
{
    if (startDelayMs() != ms) {
        m_settings.setValue(QStringLiteral("startDelayMs"), ms);
        Q_EMIT settingsChanged();
    }
}

bool Settings::confirmEnabled() const
{
    return m_settings.value(QStringLiteral("confirmEnabled"), false).toBool();
}

void Settings::setConfirmEnabled(bool enabled)
{
    if (confirmEnabled() != enabled) {
        m_settings.setValue(QStringLiteral("confirmEnabled"), enabled);
        Q_EMIT settingsChanged();
    }
}

int Settings::confirmThreshold() const
{
    return m_settings.value(QStringLiteral("confirmThreshold"), 100).toInt();
}

void Settings::setConfirmThreshold(int chars)
{
    if (confirmThreshold() != chars) {
        m_settings.setValue(QStringLiteral("confirmThreshold"), chars);
        Q_EMIT settingsChanged();
    }
}

QString Settings::hotkey() const
{
    return m_settings.value(QStringLiteral("hotkey"), QStringLiteral("V")).toString();
}

void Settings::setHotkey(const QString& key)
{
    if (hotkey() != key) {
        m_settings.setValue(QStringLiteral("hotkey"), key);
        Q_EMIT hotkeyChanged();
    }
}

Qt::KeyboardModifiers Settings::hotkeyModifiers() const
{
    int mods = m_settings.value(QStringLiteral("hotkeyModifiers"),
                                 static_cast<int>(Qt::ControlModifier | Qt::AltModifier)).toInt();
    return static_cast<Qt::KeyboardModifiers>(mods);
}

void Settings::setHotkeyModifiers(Qt::KeyboardModifiers mods)
{
    if (hotkeyModifiers() != mods) {
        m_settings.setValue(QStringLiteral("hotkeyModifiers"), static_cast<int>(mods));
        Q_EMIT hotkeyChanged();
    }
}

Settings::HotkeyMode Settings::hotkeyMode() const
{
    return static_cast<HotkeyMode>(m_settings.value(QStringLiteral("hotkeyMode"), 0).toInt());
}

void Settings::setHotkeyMode(HotkeyMode mode)
{
    if (hotkeyMode() != mode) {
        m_settings.setValue(QStringLiteral("hotkeyMode"), static_cast<int>(mode));
        Q_EMIT settingsChanged();
    }
}

void Settings::sync()
{
    m_settings.sync();
}
