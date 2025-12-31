#include "clipboardmanager.h"

#include <QApplication>
#include <QClipboard>

ClipboardManager::ClipboardManager(QObject* parent)
    : QObject(parent)
    , m_clipboard(QApplication::clipboard())
{
}

QString ClipboardManager::getText() const
{
    QString text = m_clipboard->text();
    // Normalize line endings: \r\n -> \n (Linux standard)
    text.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    text.replace(QStringLiteral("\r"), QStringLiteral("\n"));
    return text;
}

bool ClipboardManager::hasText() const
{
    return !m_clipboard->text().isEmpty();
}
