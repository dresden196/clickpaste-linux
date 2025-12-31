#ifndef TARGETOVERLAY_H
#define TARGETOVERLAY_H

#include <QWidget>
#include <QPoint>

namespace LayerShellQt {
class Window;
}

class TargetOverlay : public QWidget
{
    Q_OBJECT

public:
    explicit TargetOverlay(QWidget* parent = nullptr);
    ~TargetOverlay();

    void activate();
    void deactivate();
    bool isActive() const;

Q_SIGNALS:
    void targetSelected(const QPoint& globalPos);
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

    LayerShellQt::Window* m_layerWindow;
    QPoint m_cursorPos;
    bool m_active;
};

#endif // TARGETOVERLAY_H
