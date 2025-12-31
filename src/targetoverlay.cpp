#include "targetoverlay.h"

#include <QScreen>
#include <QGuiApplication>
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QCursor>

#include <LayerShellQt/Window>
#include <LayerShellQt/Shell>

TargetOverlay::TargetOverlay(QWidget* parent)
    : QWidget(parent, Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
    , m_layerWindow(nullptr)
    , m_active(false)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setMouseTracking(true);

    // Set crosshair cursor
    setCursor(Qt::CrossCursor);
}

TargetOverlay::~TargetOverlay()
{
}

void TargetOverlay::activate()
{
    if (m_active) {
        return;
    }

    m_active = true;

    // Get the primary screen geometry
    QScreen* screen = QGuiApplication::primaryScreen();
    if (screen) {
        setGeometry(screen->geometry());
    }

    setupLayerShell();
    show();
    raise();
    setFocus();
}

void TargetOverlay::deactivate()
{
    if (!m_active) {
        return;
    }

    m_active = false;
    hide();
}

bool TargetOverlay::isActive() const
{
    return m_active;
}

void TargetOverlay::setupLayerShell()
{
    // Initialize Layer Shell for Wayland
    if (!LayerShellQt::Shell::isInitialized()) {
        // Layer Shell not available - we're probably not on Wayland
        // Fall back to regular window behavior
        return;
    }

    m_layerWindow = LayerShellQt::Window::get(windowHandle());
    if (m_layerWindow) {
        // Set the layer to overlay (topmost)
        m_layerWindow->setLayer(LayerShellQt::Window::LayerOverlay);

        // Set anchors to all edges to cover the full screen
        m_layerWindow->setAnchors(LayerShellQt::Window::AnchorTop |
                                   LayerShellQt::Window::AnchorBottom |
                                   LayerShellQt::Window::AnchorLeft |
                                   LayerShellQt::Window::AnchorRight);

        // Request keyboard interactivity so we can receive Escape key
        m_layerWindow->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityOnDemand);

        // Exclusive zone of -1 means we don't reserve any space
        m_layerWindow->setExclusiveZone(-1);
    }
}

void TargetOverlay::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    grabKeyboard();
}

void TargetOverlay::hideEvent(QHideEvent* event)
{
    releaseKeyboard();
    QWidget::hideEvent(event);
}

void TargetOverlay::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    QPainter painter(this);

    // Draw a semi-transparent overlay
    painter.fillRect(rect(), QColor(0, 0, 0, 30));

    // Draw crosshair at cursor position
    if (!m_cursorPos.isNull()) {
        painter.setPen(QPen(QColor(255, 100, 100), 2));

        // Horizontal line
        painter.drawLine(0, m_cursorPos.y(), width(), m_cursorPos.y());

        // Vertical line
        painter.drawLine(m_cursorPos.x(), 0, m_cursorPos.x(), height());

        // Circle at center
        painter.drawEllipse(m_cursorPos, 20, 20);
    }

    // Draw instruction text
    painter.setPen(Qt::white);
    QFont font = painter.font();
    font.setPointSize(14);
    font.setBold(true);
    painter.setFont(font);

    QString text = QStringLiteral("Click on the target window, or press Escape to cancel");
    QRect textRect = painter.fontMetrics().boundingRect(text);
    textRect.moveCenter(QPoint(width() / 2, 50));

    // Draw text background
    QRect bgRect = textRect.adjusted(-10, -5, 10, 5);
    painter.fillRect(bgRect, QColor(0, 0, 0, 180));
    painter.drawText(textRect, Qt::AlignCenter, text);
}

void TargetOverlay::mousePressEvent(QMouseEvent* event)
{
    // We capture the click but wait for release
    m_cursorPos = event->pos();
    update();
}

void TargetOverlay::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        QPoint globalPos = event->globalPosition().toPoint();
        deactivate();

        // Small delay to allow the underlying window to receive focus
        emit targetSelected(globalPos);
    }
}

void TargetOverlay::mouseMoveEvent(QMouseEvent* event)
{
    m_cursorPos = event->pos();
    update();
}

void TargetOverlay::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) {
        deactivate();
        emit cancelled();
    }
}
