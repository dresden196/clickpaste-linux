#ifndef APPLICATION_H
#define APPLICATION_H

#include <QObject>
#include <memory>

class QAction;
class TrayIcon;
class HotkeyManager;
class InputEmulator;
class TargetOverlay;
class ClipboardManager;
class SettingsDialog;
class QLockFile;

class Application : public QObject
{
    Q_OBJECT

public:
    explicit Application(QObject* parent = nullptr);
    ~Application();

    bool initialize();
    void shutdown();

private Q_SLOTS:
    void onTrayActivated();
    void onSettingsRequested();
    void onExitRequested();

    void onHotkeyTriggered();
    void onTargetSelected(const QPoint& globalPos);
    void onTargetCancelled();

    void onTypingStarted();
    void onTypingFinished();
    void onTypingCancelled();
    void onTypingError(const QString& error);

    void onHotkeyChanged();

private:
    bool checkSingleInstance();
    void startTargeting();
    void startTyping();
    bool showConfirmationDialog(const QString& text);

    void registerHotkey();
    void registerCancelHotkey();
    void unregisterCancelHotkey();

    std::unique_ptr<QLockFile> m_lockFile;
    std::unique_ptr<TrayIcon> m_trayIcon;
    std::unique_ptr<HotkeyManager> m_hotkeyManager;
    std::unique_ptr<InputEmulator> m_inputEmulator;
    std::unique_ptr<TargetOverlay> m_targetOverlay;
    std::unique_ptr<ClipboardManager> m_clipboardManager;
    QAction* m_cancelAction;
};

#endif // APPLICATION_H
