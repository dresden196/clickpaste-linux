#ifndef INPUTEMULATOR_H
#define INPUTEMULATOR_H

#include <QObject>
#include <QString>
#include <atomic>

class InputEmulator : public QObject
{
    Q_OBJECT

public:
    explicit InputEmulator(QObject* parent = nullptr);
    ~InputEmulator();

    bool initialize();
    bool isInitialized() const;

    void typeText(const QString& text, int keyDelayMs, int startDelayMs = 0);
    void cancel();
    bool isTyping() const;

Q_SIGNALS:
    void typingStarted();
    void typingProgress(int current, int total);
    void typingFinished();
    void typingCancelled();
    void errorOccurred(const QString& error);

private:
    std::atomic<bool> m_cancelled;
    std::atomic<bool> m_typing;
    bool m_initialized;
    QString m_socketPath;
};

#endif // INPUTEMULATOR_H
