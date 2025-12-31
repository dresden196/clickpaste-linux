#ifndef TARGETOVERLAY_H
#define TARGETOVERLAY_H

#include <QWidget>
#include <QPoint>
#include <QList>

namespace LayerShellQt {
class Window;
}

class QScreen;
class ScreenOverlay;

class TargetOverlay : public QObject
{
    Q_OBJECT

public:
    explicit TargetOverlay(QObject* parent = nullptr);
    ~TargetOverlay();

    void activate();
    void deactivate();
    bool isActive() const;

Q_SIGNALS:
    void targetSelected(const QPoint& globalPos);
    void cancelled();

private Q_SLOTS:
    void onScreenAdded(QScreen* screen);
    void onScreenRemoved(QScreen* screen);
    void onOverlayClicked(const QPoint& globalPos);
    void onOverlayCancelled();

private:
    void createOverlayForScreen(QScreen* screen);
    void removeOverlayForScreen(QScreen* screen);

    QList<ScreenOverlay*> m_overlays;
    bool m_active;
};

// Per-screen overlay widget
class ScreenOverlay : public QWidget
{
    Q_OBJECT

public:
    explicit ScreenOverlay(QScreen* screen, QWidget* parent = nullptr);
    ~ScreenOverlay();

    QScreen* screen() const { return m_screen; }
    void activate();
    void deactivate();

Q_SIGNALS:
    void clicked(const QPoint& globalPos);
    void cancelled();

protected:
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    void setupLayerShell();

    QScreen* m_screen;
    LayerShellQt::Window* m_layerWindow;
    QPoint m_cursorPos;
};

#endif // TARGETOVERLAY_H
