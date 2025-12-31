#include "application.h"
#include "trayicon.h"
#include "hotkeymanager.h"
#include "inputemulator.h"
#include "targetoverlay.h"
#include "clipboardmanager.h"
#include "settingsdialog.h"
#include "settings.h"

#include <QApplication>
#include <QLockFile>
#include <QStandardPaths>
#include <QDir>
#include <QMessageBox>
#include <QTimer>
#include <QDebug>
#include <QAction>
#include <KGlobalAccel>

Application::Application(QObject* parent)
    : QObject(parent)
    , m_cancelAction(nullptr)
{
}

Application::~Application()
{
    shutdown();
}

bool Application::initialize()
{
    // Check single instance
    if (!checkSingleInstance()) {
        QMessageBox::warning(nullptr, QStringLiteral("ClickPaste"),
                            QStringLiteral("ClickPaste is already running."));
        return false;
    }

    // Create components
    m_trayIcon = std::make_unique<TrayIcon>();
    m_hotkeyManager = std::make_unique<HotkeyManager>();
    m_inputEmulator = std::make_unique<InputEmulator>();
    m_targetOverlay = std::make_unique<TargetOverlay>();
    m_clipboardManager = std::make_unique<ClipboardManager>();

    // Connect tray icon signals
    connect(m_trayIcon.get(), &TrayIcon::activated,
            this, &Application::onTrayActivated);
    connect(m_trayIcon.get(), &TrayIcon::settingsRequested,
            this, &Application::onSettingsRequested);
    connect(m_trayIcon.get(), &TrayIcon::exitRequested,
            this, &Application::onExitRequested);

    // Connect hotkey manager signals
    connect(m_hotkeyManager.get(), &HotkeyManager::hotkeyTriggered,
            this, &Application::onHotkeyTriggered);
    connect(m_hotkeyManager.get(), &HotkeyManager::registrationFailed,
            this, [this](const QString& reason) {
                m_trayIcon->showMessage(QStringLiteral("ClickPaste"),
                                        reason,
                                        QSystemTrayIcon::Warning);
            });

    // Connect target overlay signals
    connect(m_targetOverlay.get(), &TargetOverlay::targetSelected,
            this, &Application::onTargetSelected);
    connect(m_targetOverlay.get(), &TargetOverlay::cancelled,
            this, &Application::onTargetCancelled);

    // Connect input emulator signals
    connect(m_inputEmulator.get(), &InputEmulator::typingStarted,
            this, &Application::onTypingStarted);
    connect(m_inputEmulator.get(), &InputEmulator::typingFinished,
            this, &Application::onTypingFinished);
    connect(m_inputEmulator.get(), &InputEmulator::typingCancelled,
            this, &Application::onTypingCancelled);
    connect(m_inputEmulator.get(), &InputEmulator::errorOccurred,
            this, &Application::onTypingError);

    // Connect settings changes
    connect(Settings::instance(), &Settings::hotkeyChanged,
            this, &Application::onHotkeyChanged);

    // Initialize input emulator
    if (!m_inputEmulator->initialize()) {
        qWarning() << "Failed to initialize input emulator - typing may not work";
        m_trayIcon->showMessage(QStringLiteral("ClickPaste"),
                                QStringLiteral("Failed to initialize input emulation. "
                                              "Ensure your compositor supports libei."),
                                QSystemTrayIcon::Warning);
    }

    // Register hotkey
    registerHotkey();

    // Show tray icon
    m_trayIcon->show();

    return true;
}

void Application::shutdown()
{
    if (m_hotkeyManager) {
        m_hotkeyManager->unregisterHotkey();
    }

    if (m_targetOverlay && m_targetOverlay->isActive()) {
        m_targetOverlay->deactivate();
    }

    if (m_lockFile) {
        m_lockFile->unlock();
    }
}

bool Application::checkSingleInstance()
{
    QString lockPath = QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation);
    if (lockPath.isEmpty()) {
        lockPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    }

    QDir dir(lockPath);
    if (!dir.exists()) {
        dir.mkpath(QStringLiteral("."));
    }

    m_lockFile = std::make_unique<QLockFile>(lockPath + QStringLiteral("/clickpaste.lock"));
    m_lockFile->setStaleLockTime(0);

    if (!m_lockFile->tryLock(100)) {
        return false;
    }

    return true;
}

void Application::onTrayActivated()
{
    // Tray icon clicked - start targeting
    startTargeting();
}

void Application::onSettingsRequested()
{
    // Disable hotkey while settings dialog is open
    m_hotkeyManager->setEnabled(false);

    SettingsDialog dialog;
    dialog.exec();

    // Re-enable hotkey
    m_hotkeyManager->setEnabled(true);
}

void Application::onExitRequested()
{
    shutdown();
    QApplication::quit();
}

void Application::onHotkeyTriggered()
{
    Settings* s = Settings::instance();

    if (s->hotkeyMode() == Settings::JustGo) {
        // Just Go mode - type immediately to focused window
        startTyping();
    } else {
        // Target mode - show overlay to select window
        startTargeting();
    }
}

void Application::startTargeting()
{
    if (m_inputEmulator->isTyping()) {
        return;
    }

    m_trayIcon->setIconState(TrayIcon::Targeting);
    m_targetOverlay->activate();
}

void Application::onTargetSelected(const QPoint& globalPos)
{
    Q_UNUSED(globalPos)

    m_trayIcon->setIconState(TrayIcon::Normal);

    // Small delay to allow the underlying window to receive focus
    QTimer::singleShot(150, this, &Application::startTyping);
}

void Application::onTargetCancelled()
{
    m_trayIcon->setIconState(TrayIcon::Normal);
}

void Application::startTyping()
{
    // Check clipboard
    if (!m_clipboardManager->hasText()) {
        QApplication::beep();
        m_trayIcon->showMessage(QStringLiteral("ClickPaste"),
                                QStringLiteral("Clipboard is empty"),
                                QSystemTrayIcon::Information);
        return;
    }

    QString text = m_clipboardManager->getText();

    // Check confirmation
    Settings* s = Settings::instance();
    if (s->confirmEnabled() && text.length() > s->confirmThreshold()) {
        if (!showConfirmationDialog(text)) {
            return;
        }
    }

    // Start typing
    m_inputEmulator->typeText(text, s->keyDelayMs(), s->startDelayMs());
}

bool Application::showConfirmationDialog(const QString& text)
{
    QApplication::beep();

    QString preview = text.left(100);
    if (text.length() > 100) {
        preview += QStringLiteral("...");
    }

    QMessageBox::StandardButton result = QMessageBox::question(
        nullptr,
        QStringLiteral("ClickPaste - Confirm"),
        QStringLiteral("About to type %1 characters:\n\n\"%2\"\n\nContinue?")
            .arg(text.length())
            .arg(preview),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::Yes
    );

    return result == QMessageBox::Yes;
}

void Application::onTypingStarted()
{
    m_trayIcon->setIconState(TrayIcon::Typing);
    m_hotkeyManager->setEnabled(false);
    registerCancelHotkey();
}

void Application::onTypingFinished()
{
    unregisterCancelHotkey();
    m_trayIcon->setIconState(TrayIcon::Normal);
    m_hotkeyManager->setEnabled(true);
}

void Application::onTypingCancelled()
{
    unregisterCancelHotkey();
    m_trayIcon->setIconState(TrayIcon::Normal);
    m_hotkeyManager->setEnabled(true);
    m_trayIcon->showMessage(QStringLiteral("ClickPaste"),
                            QStringLiteral("Typing cancelled"),
                            QSystemTrayIcon::Information);
}

void Application::onTypingError(const QString& error)
{
    unregisterCancelHotkey();
    m_trayIcon->setIconState(TrayIcon::Normal);
    m_hotkeyManager->setEnabled(true);
    m_trayIcon->showMessage(QStringLiteral("ClickPaste Error"),
                            error,
                            QSystemTrayIcon::Critical);
}

void Application::onHotkeyChanged()
{
    registerHotkey();
}

void Application::registerHotkey()
{
    Settings* s = Settings::instance();
    m_hotkeyManager->registerHotkey(s->hotkey(), s->hotkeyModifiers());
}

void Application::registerCancelHotkey()
{
    if (m_cancelAction) {
        return; // Already registered
    }

    m_cancelAction = new QAction(this);
    m_cancelAction->setObjectName(QStringLiteral("clickpaste_cancel"));
    m_cancelAction->setText(QStringLiteral("Cancel ClickPaste"));

    connect(m_cancelAction, &QAction::triggered, this, [this]() {
        if (m_inputEmulator && m_inputEmulator->isTyping()) {
            m_inputEmulator->cancel();
        }
    });

    KGlobalAccel::setGlobalShortcut(m_cancelAction,
                                     QList<QKeySequence>() << QKeySequence(Qt::Key_Escape));
}

void Application::unregisterCancelHotkey()
{
    if (!m_cancelAction) {
        return;
    }

    KGlobalAccel::removeAllShortcuts(m_cancelAction);
    delete m_cancelAction;
    m_cancelAction = nullptr;
}
