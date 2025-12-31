#ifndef INPUTEMULATOR_H
#define INPUTEMULATOR_H

#include <QObject>
#include <QString>
#include <atomic>

struct ei;
struct ei_seat;
struct ei_device;

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
    bool connectToEI();
    void disconnectFromEI();
    bool createKeyboard();
    void destroyKeyboard();

    void sendKey(uint32_t keycode, bool press);
    uint32_t charToKeycode(QChar ch, bool& needShift);

    struct ei* m_ei;
    struct ei_seat* m_seat;
    struct ei_device* m_keyboard;
    std::atomic<bool> m_cancelled;
    std::atomic<bool> m_typing;
    bool m_initialized;
};

#endif // INPUTEMULATOR_H
