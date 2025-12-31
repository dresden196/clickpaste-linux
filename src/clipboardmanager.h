#ifndef CLIPBOARDMANAGER_H
#define CLIPBOARDMANAGER_H

#include <QObject>
#include <QString>

class QClipboard;

class ClipboardManager : public QObject
{
    Q_OBJECT

public:
    explicit ClipboardManager(QObject* parent = nullptr);
    ~ClipboardManager() = default;

    QString getText() const;
    bool hasText() const;

private:
    QClipboard* m_clipboard;
};

#endif // CLIPBOARDMANAGER_H
