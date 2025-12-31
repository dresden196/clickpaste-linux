#ifndef HOTKEYMANAGER_H
#define HOTKEYMANAGER_H

#include <QObject>
#include <QString>
#include <Qt>

class QAction;

class HotkeyManager : public QObject
{
    Q_OBJECT

public:
    explicit HotkeyManager(QObject* parent = nullptr);
    ~HotkeyManager();

    bool registerHotkey(const QString& key, Qt::KeyboardModifiers modifiers);
    void unregisterHotkey();
    bool isRegistered() const;

    void setEnabled(bool enabled);
    bool isEnabled() const;

signals:
    void hotkeyTriggered();
    void registrationFailed(const QString& reason);

private slots:
    void onActionTriggered();

private:
    QAction* m_action;
    bool m_enabled;
    bool m_registered;
};

#endif // HOTKEYMANAGER_H
