#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QSettings>
#include <QString>
#include <Qt>

class Settings : public QObject
{
    Q_OBJECT

public:
    enum HotkeyMode {
        Target = 0,
        JustGo = 1
    };
    Q_ENUM(HotkeyMode)

    static Settings* instance();

    // Delay settings
    int keyDelayMs() const;
    void setKeyDelayMs(int ms);

    int startDelayMs() const;
    void setStartDelayMs(int ms);

    // Confirmation settings
    bool confirmEnabled() const;
    void setConfirmEnabled(bool enabled);

    int confirmThreshold() const;
    void setConfirmThreshold(int chars);

    // Hotkey settings
    QString hotkey() const;
    void setHotkey(const QString& key);

    Qt::KeyboardModifiers hotkeyModifiers() const;
    void setHotkeyModifiers(Qt::KeyboardModifiers mods);

    HotkeyMode hotkeyMode() const;
    void setHotkeyMode(HotkeyMode mode);

    void sync();

Q_SIGNALS:
    void settingsChanged();
    void hotkeyChanged();

private:
    explicit Settings(QObject* parent = nullptr);
    ~Settings() = default;

    static Settings* s_instance;
    QSettings m_settings;
};

#endif // SETTINGS_H
