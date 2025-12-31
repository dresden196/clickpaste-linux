#include "clipboardmanager.h"

#include <QApplication>
#include <QClipboard>
#include <QProcess>

ClipboardManager::ClipboardManager(QObject* parent)
    : QObject(parent)
    , m_clipboard(QApplication::clipboard())
{
}

QString ClipboardManager::getText() const
{
    // Try wl-paste first (more reliable on Wayland)
    QProcess process;
    process.start(QStringLiteral("wl-paste"), {QStringLiteral("--no-newline")});
    if (process.waitForFinished(1000)) {
        QString text = QString::fromUtf8(process.readAllStandardOutput());
        if (!text.isEmpty()) {
            // Normalize line endings: \r\n -> \n (Linux standard)
            text.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
            text.replace(QStringLiteral("\r"), QStringLiteral("\n"));
            return text;
        }
    }

    // Fallback to Qt clipboard
    QString text = m_clipboard->text();
    text.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    text.replace(QStringLiteral("\r"), QStringLiteral("\n"));
    return text;
}

bool ClipboardManager::hasText() const
{
    // Try wl-paste first
    QProcess process;
    process.start(QStringLiteral("wl-paste"), {QStringLiteral("--no-newline")});
    if (process.waitForFinished(1000)) {
        return !process.readAllStandardOutput().isEmpty();
    }

    // Fallback to Qt clipboard
    return !m_clipboard->text().isEmpty();
}
