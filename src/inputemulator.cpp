#include "inputemulator.h"

#include <QThread>
#include <QDebug>
#include <QProcess>

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

    // Check if wtype is available
    QProcess process;
    process.start(QStringLiteral("which"), {QStringLiteral("wtype")});
    if (process.waitForFinished(1000) && process.exitCode() == 0) {
        m_initialized = true;
        return true;
    }

    Q_EMIT errorOccurred(QStringLiteral("wtype not found. Please install wtype: sudo pacman -S wtype"));
    return false;
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

    // Use wtype for typing
    // For character-by-character with delays, we type one char at a time
    if (keyDelayMs > 0) {
        int total = text.length();
        for (int i = 0; i < total && !m_cancelled; ++i) {
            QString ch = text.mid(i, 1);

            QProcess process;
            process.start(QStringLiteral("wtype"), {QStringLiteral("--"), ch});
            process.waitForFinished(5000);

            Q_EMIT typingProgress(i + 1, total);

            if (keyDelayMs > 0 && i < total - 1) {
                QThread::msleep(keyDelayMs);
            }
        }
    } else {
        // Type all at once (faster)
        QProcess process;
        process.start(QStringLiteral("wtype"), {QStringLiteral("--"), text});
        process.waitForFinished(30000);
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
