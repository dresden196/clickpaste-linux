#include "inputemulator.h"

#include <QThread>
#include <QDebug>
#include <QProcess>
#include <QProcessEnvironment>
#include <QFile>
#include <unistd.h>

InputEmulator::InputEmulator(QObject* parent)
    : QObject(parent)
    , m_cancelled(false)
    , m_typing(false)
    , m_initialized(false)
{
}

InputEmulator::~InputEmulator()
{
}

bool InputEmulator::initialize()
{
    if (m_initialized) {
        return true;
    }

    // Check if ydotool is available
    QProcess which;
    which.start(QStringLiteral("which"), {QStringLiteral("ydotool")});
    if (!which.waitForFinished(1000) || which.exitCode() != 0) {
        Q_EMIT errorOccurred(QStringLiteral("ydotool not found. Please install: sudo pacman -S ydotool"));
        return false;
    }

    // Check if ydotoold system socket exists (systemd service running)
    m_socketPath = QStringLiteral("/run/ydotool/socket");

    if (!QFile::exists(m_socketPath)) {
        Q_EMIT errorOccurred(QStringLiteral("ydotoold service not running. Please enable it:\n"
                                            "  sudo systemctl enable --now ydotoold.service"));
        return false;
    }

    m_initialized = true;
    return true;
}

bool InputEmulator::isInitialized() const
{
    return m_initialized;
}

void InputEmulator::typeText(const QString& text, int keyDelayMs, int startDelayMs)
{
    if (!m_initialized) {
        Q_EMIT errorOccurred(QStringLiteral("Input emulator not initialized"));
        return;
    }

    if (text.isEmpty()) {
        Q_EMIT errorOccurred(QStringLiteral("No text to type"));
        return;
    }

    m_cancelled = false;
    m_typing = true;
    Q_EMIT typingStarted();

    // Start delay
    if (startDelayMs > 0) {
        QThread::msleep(startDelayMs);
    }

    if (m_cancelled) {
        m_typing = false;
        Q_EMIT typingCancelled();
        return;
    }

    // Use ydotool for typing
    // ydotool type --key-delay <ms> "text"
    QProcess process;

    // Set the socket path environment variable
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert(QStringLiteral("YDOTOOL_SOCKET"), m_socketPath);
    process.setProcessEnvironment(env);

    QStringList args;
    args << QStringLiteral("type");

    if (keyDelayMs > 0) {
        args << QStringLiteral("--key-delay") << QString::number(keyDelayMs);
    }

    args << QStringLiteral("--") << text;

    process.start(QStringLiteral("ydotool"), args);

    if (!process.waitForFinished(60000)) {
        Q_EMIT errorOccurred(QStringLiteral("ydotool timed out"));
        m_typing = false;
        return;
    }

    if (process.exitCode() != 0) {
        QString error = QString::fromUtf8(process.readAllStandardError());
        if (error.isEmpty()) {
            error = QStringLiteral("ydotool failed. Is ydotoold running? Try: sudo systemctl start ydotoold");
        }
        Q_EMIT errorOccurred(error);
        m_typing = false;
        return;
    }

    m_typing = false;

    if (m_cancelled) {
        Q_EMIT typingCancelled();
    } else {
        Q_EMIT typingFinished();
    }
}

void InputEmulator::cancel()
{
    m_cancelled = true;
}

bool InputEmulator::isTyping() const
{
    return m_typing;
}
