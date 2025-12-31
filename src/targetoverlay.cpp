#include "targetoverlay.h"

#include <QScreen>
#include <QGuiApplication>
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QCursor>

#include <LayerShellQt/Window>

// ============================================================================
// TargetOverlay - manages overlays across all screens
// ============================================================================

TargetOverlay::TargetOverlay(QObject* parent)
    : QObject(parent)
    , m_active(false)
{
    // Create overlays for existing screens
    const auto screens = QGuiApplication::screens();
    for (QScreen* screen : screens) {
        createOverlayForScreen(screen);
    }

    // Handle dynamic screen changes
    connect(qApp, &QGuiApplication::screenAdded, this, &TargetOverlay::onScreenAdded);
    connect(qApp, &QGuiApplication::screenRemoved, this, &TargetOverlay::onScreenRemoved);
}

TargetOverlay::~TargetOverlay()
{
    qDeleteAll(m_overlays);
    m_overlays.clear();
}

void TargetOverlay::activate()
{
    if (m_active) {
        return;
    }

    m_active = true;

    for (ScreenOverlay* overlay : m_overlays) {
        overlay->activate();
    }
}

void TargetOverlay::deactivate()
{
    if (!m_active) {
        return;
    }

    m_active = false;

    for (ScreenOverlay* overlay : m_overlays) {
        overlay->deactivate();
    }
}

bool TargetOverlay::isActive() const
{
    return m_active;
}

void TargetOverlay::onScreenAdded(QScreen* screen)
{
    createOverlayForScreen(screen);
    if (m_active) {
        m_overlays.last()->activate();
    }
}

void TargetOverlay::onScreenRemoved(QScreen* screen)
{
    removeOverlayForScreen(screen);
}

void TargetOverlay::onOverlayClicked(const QPoint& globalPos)
{
    deactivate();
    Q_EMIT targetSelected(globalPos);
}

void TargetOverlay::onOverlayCancelled()
{
    deactivate();
    Q_EMIT cancelled();
}

void TargetOverlay::createOverlayForScreen(QScreen* screen)
{
    auto* overlay = new ScreenOverlay(screen);
    connect(overlay, &ScreenOverlay::clicked, this, &TargetOverlay::onOverlayClicked);
    connect(overlay, &ScreenOverlay::cancelled, this, &TargetOverlay::onOverlayCancelled);
    m_overlays.append(overlay);
}

void TargetOverlay::removeOverlayForScreen(QScreen* screen)
{
    for (int i = 0; i < m_overlays.size(); ++i) {
        if (m_overlays[i]->screen() == screen) {
            delete m_overlays.takeAt(i);
            return;
        }
    }
}

// ============================================================================
// ScreenOverlay - per-screen overlay widget
// ============================================================================

ScreenOverlay::ScreenOverlay(QScreen* screen, QWidget* parent)
    : QWidget(parent, Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
    , m_screen(screen)
    , m_layerWindow(nullptr)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setMouseTracking(true);
    setCursor(Qt::CrossCursor);

    // Set geometry to match screen
    setGeometry(screen->geometry());
}

ScreenOverlay::~ScreenOverlay()
{
}

void ScreenOverlay::activate()
{
    // Update geometry in case screen changed
    setGeometry(m_screen->geometry());
    setupLayerShell();
    show();
    raise();
    setFocus();
}

void ScreenOverlay::deactivate()
{
    hide();
}

void ScreenOverlay::setupLayerShell()
{
    if (!windowHandle()) {
        create();
    }

    m_layerWindow = LayerShellQt::Window::get(windowHandle());
    if (m_layerWindow) {
        m_layerWindow->setLayer(LayerShellQt::Window::LayerOverlay);

        // Anchor to all edges of this screen
        LayerShellQt::Window::Anchors anchors;
        anchors.setFlag(LayerShellQt::Window::AnchorTop);
        anchors.setFlag(LayerShellQt::Window::AnchorBottom);
        anchors.setFlag(LayerShellQt::Window::AnchorLeft);
        anchors.setFlag(LayerShellQt::Window::AnchorRight);
        m_layerWindow->setAnchors(anchors);

        m_layerWindow->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityOnDemand);
        m_layerWindow->setExclusiveZone(-1);

        // Bind to this specific screen
        m_layerWindow->setDesiredOutput(m_screen);
    }
}

void ScreenOverlay::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    grabKeyboard();
}

void ScreenOverlay::hideEvent(QHideEvent* event)
{
    releaseKeyboard();
    QWidget::hideEvent(event);
}

void ScreenOverlay::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    QPainter painter(this);

    // Semi-transparent overlay
    painter.fillRect(rect(), QColor(0, 0, 0, 30));

    // Crosshair at cursor position
    if (!m_cursorPos.isNull()) {
        painter.setPen(QPen(QColor(255, 100, 100), 2));
        painter.drawLine(0, m_cursorPos.y(), width(), m_cursorPos.y());
        painter.drawLine(m_cursorPos.x(), 0, m_cursorPos.x(), height());
        painter.drawEllipse(m_cursorPos, 20, 20);
    }

    // Instruction text
    painter.setPen(Qt::white);
    QFont font = painter.font();
    font.setPointSize(14);
    font.setBold(true);
    painter.setFont(font);

    QString text = QStringLiteral("Click on the target window, or press Escape to cancel");
    QRect textRect = painter.fontMetrics().boundingRect(text);
    textRect.moveCenter(QPoint(width() / 2, 50));

    QRect bgRect = textRect.adjusted(-10, -5, 10, 5);
    painter.fillRect(bgRect, QColor(0, 0, 0, 180));
    painter.drawText(textRect, Qt::AlignCenter, text);
}

void ScreenOverlay::mousePressEvent(QMouseEvent* event)
{
    m_cursorPos = event->pos();
    update();
}

void ScreenOverlay::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        Q_EMIT clicked(event->globalPosition().toPoint());
    }
}

void ScreenOverlay::mouseMoveEvent(QMouseEvent* event)
{
    m_cursorPos = event->pos();
    update();
}

void ScreenOverlay::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) {
        Q_EMIT cancelled();
    }
}
