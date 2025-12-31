#include "inputemulator.h"

#include <QThread>
#include <QDebug>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusUnixFileDescriptor>

#include <libei.h>
#include <linux/input-event-codes.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

InputEmulator::InputEmulator(QObject* parent)
    : QObject(parent)
    , m_ei(nullptr)
    , m_seat(nullptr)
    , m_keyboard(nullptr)
    , m_cancelled(false)
    , m_typing(false)
    , m_initialized(false)
{
}

InputEmulator::~InputEmulator()
{
    disconnectFromEI();
}

bool InputEmulator::initialize()
{
    if (m_initialized) {
        return true;
    }

    m_initialized = connectToEI();
    return m_initialized;
}

bool InputEmulator::isInitialized() const
{
    return m_initialized;
}

bool InputEmulator::connectToEI()
{
    // Connect to the XDG RemoteDesktop portal to get an EI file descriptor
    QDBusInterface portal(
        QStringLiteral("org.freedesktop.portal.Desktop"),
        QStringLiteral("/org/freedesktop/portal/desktop"),
        QStringLiteral("org.freedesktop.portal.RemoteDesktop"),
        QDBusConnection::sessionBus()
    );

    if (!portal.isValid()) {
        Q_EMIT errorOccurred(QStringLiteral("Cannot connect to XDG Desktop Portal"));
        return false;
    }

    // For now, we'll use a simpler approach - direct EI connection
    // In a full implementation, we would go through the portal

    m_ei = ei_new(nullptr);
    if (!m_ei) {
        Q_EMIT errorOccurred(QStringLiteral("Failed to create libei context"));
        return false;
    }

    // Try to connect to the EI socket
    // The compositor should provide this via the portal
    int ret = ei_setup_backend_socket(m_ei, nullptr);
    if (ret != 0) {
        Q_EMIT errorOccurred(QStringLiteral("Failed to connect to EI backend. Ensure your compositor supports libei."));
        ei_unref(m_ei);
        m_ei = nullptr;
        return false;
    }

    // Process events to get the seat
    int fd = ei_get_fd(m_ei);
    struct pollfd pfd = { fd, POLLIN, 0 };

    // Wait for connection events (with timeout)
    int timeout_ms = 5000;
    while (timeout_ms > 0) {
        int ret = poll(&pfd, 1, 100);
        if (ret > 0) {
            ei_dispatch(m_ei);

            struct ei_event* event;
            while ((event = ei_get_event(m_ei)) != nullptr) {
                enum ei_event_type type = ei_event_get_type(event);

                if (type == EI_EVENT_SEAT_ADDED) {
                    m_seat = ei_event_get_seat(event);
                    ei_seat_ref(m_seat);

                    // Bind the seat to get keyboard capability
                    ei_seat_bind_capabilities(m_seat, EI_DEVICE_CAP_KEYBOARD, nullptr);
                }
                else if (type == EI_EVENT_DEVICE_ADDED) {
                    struct ei_device* device = ei_event_get_device(event);
                    if (ei_device_has_capability(device, EI_DEVICE_CAP_KEYBOARD)) {
                        m_keyboard = device;
                        ei_device_ref(m_keyboard);
                    }
                }
                else if (type == EI_EVENT_DISCONNECT) {
                    Q_EMIT errorOccurred(QStringLiteral("Disconnected from EI server"));
                    ei_event_unref(event);
                    return false;
                }

                ei_event_unref(event);

                if (m_keyboard) {
                    return true;
                }
            }
        }
        timeout_ms -= 100;
    }

    if (!m_keyboard) {
        Q_EMIT errorOccurred(QStringLiteral("Timeout waiting for keyboard device from EI"));
        return false;
    }

    return true;
}

void InputEmulator::disconnectFromEI()
{
    destroyKeyboard();

    if (m_seat) {
        ei_seat_unref(m_seat);
        m_seat = nullptr;
    }

    if (m_ei) {
        ei_unref(m_ei);
        m_ei = nullptr;
    }

    m_initialized = false;
}

bool InputEmulator::createKeyboard()
{
    // Keyboard should be created during connection
    return m_keyboard != nullptr;
}

void InputEmulator::destroyKeyboard()
{
    if (m_keyboard) {
        ei_device_unref(m_keyboard);
        m_keyboard = nullptr;
    }
}

void InputEmulator::typeText(const QString& text, int keyDelayMs, int startDelayMs)
{
    if (!m_initialized || !m_keyboard) {
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

    // Start a frame for input events
    ei_device_start_emulating(m_keyboard, 0);

    int total = text.length();
    for (int i = 0; i < total && !m_cancelled; ++i) {
        QChar ch = text.at(i);

        bool needShift = false;
        uint32_t keycode = charToKeycode(ch, needShift);

        if (keycode != 0) {
            // Press shift if needed
            if (needShift) {
                sendKey(KEY_LEFTSHIFT, true);
            }

            // Press and release the key
            sendKey(keycode, true);
            sendKey(keycode, false);

            // Release shift if needed
            if (needShift) {
                sendKey(KEY_LEFTSHIFT, false);
            }

            // Commit the frame
            ei_device_frame(m_keyboard, 0);

            // Process any pending events
            ei_dispatch(m_ei);
        }

        Q_EMIT typingProgress(i + 1, total);

        // Delay between keys
        if (keyDelayMs > 0 && i < total - 1) {
            QThread::msleep(keyDelayMs);
        }
    }

    ei_device_stop_emulating(m_keyboard);

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

void InputEmulator::sendKey(uint32_t keycode, bool press)
{
    if (!m_keyboard) return;

    ei_device_keyboard_key(m_keyboard, keycode, press ? 1 : 0);
}

uint32_t InputEmulator::charToKeycode(QChar ch, bool& needShift)
{
    needShift = false;

    // Handle common ASCII characters
    char c = ch.toLatin1();

    // Letters
    if (c >= 'a' && c <= 'z') {
        return KEY_A + (c - 'a');
    }
    if (c >= 'A' && c <= 'Z') {
        needShift = true;
        return KEY_A + (c - 'A');
    }

    // Numbers
    if (c >= '0' && c <= '9') {
        if (c == '0') return KEY_0;
        return KEY_1 + (c - '1');
    }

    // Shifted number row symbols
    switch (c) {
    case '!': needShift = true; return KEY_1;
    case '@': needShift = true; return KEY_2;
    case '#': needShift = true; return KEY_3;
    case '$': needShift = true; return KEY_4;
    case '%': needShift = true; return KEY_5;
    case '^': needShift = true; return KEY_6;
    case '&': needShift = true; return KEY_7;
    case '*': needShift = true; return KEY_8;
    case '(': needShift = true; return KEY_9;
    case ')': needShift = true; return KEY_0;
    }

    // Punctuation and special characters
    switch (c) {
    case ' ': return KEY_SPACE;
    case '\n': return KEY_ENTER;
    case '\t': return KEY_TAB;
    case '\b': return KEY_BACKSPACE;

    case '-': return KEY_MINUS;
    case '_': needShift = true; return KEY_MINUS;
    case '=': return KEY_EQUAL;
    case '+': needShift = true; return KEY_EQUAL;

    case '[': return KEY_LEFTBRACE;
    case '{': needShift = true; return KEY_LEFTBRACE;
    case ']': return KEY_RIGHTBRACE;
    case '}': needShift = true; return KEY_RIGHTBRACE;

    case '\\': return KEY_BACKSLASH;
    case '|': needShift = true; return KEY_BACKSLASH;

    case ';': return KEY_SEMICOLON;
    case ':': needShift = true; return KEY_SEMICOLON;

    case '\'': return KEY_APOSTROPHE;
    case '"': needShift = true; return KEY_APOSTROPHE;

    case '`': return KEY_GRAVE;
    case '~': needShift = true; return KEY_GRAVE;

    case ',': return KEY_COMMA;
    case '<': needShift = true; return KEY_COMMA;

    case '.': return KEY_DOT;
    case '>': needShift = true; return KEY_DOT;

    case '/': return KEY_SLASH;
    case '?': needShift = true; return KEY_SLASH;
    }

    // Unknown character - skip it
    qDebug() << "Unknown character:" << ch;
    return 0;
}
