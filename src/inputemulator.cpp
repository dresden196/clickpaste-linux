#include "inputemulator.h"

#include <QThread>
#include <QDebug>
#include <QProcess>
#include <QFile>
#include <QStandardPaths>
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

    // Check if ydotoold socket exists (daemon running)
    QString socketPath = QStringLiteral("/run/user/%1/.ydotool_socket").arg(getuid());

    if (!QFile::exists(socketPath)) {
        // Try to start ydotoold
        qDebug() << "Starting ydotoold daemon...";

        QProcess daemon;
        daemon.setProgram(QStringLiteral("ydotoold"));
        daemon.setArguments({QStringLiteral("--socket-path"), socketPath, QStringLiteral("--socket-perm"), QStringLiteral("0600")});
        daemon.startDetached();

        // Wait a moment for it to start
        QThread::msleep(500);

        if (!QFile::exists(socketPath)) {
            Q_EMIT errorOccurred(QStringLiteral("Could not start ydotoold. Make sure you're in the 'input' group:\n"
                                                "  sudo usermod -aG input $USER\n"
                                                "Then log out and back in."));
            return false;
        }
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
            error = QStringLiteral("ydotool failed. Is ydotoold running? Try: sudo systemctl start ydotool");
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
