#ifndef TRAYICON_H
#define TRAYICON_H

#include <QObject>
#include <QSystemTrayIcon>

class QMenu;
class QAction;

class TrayIcon : public QObject
{
    Q_OBJECT

public:
    enum IconState {
        Normal,
        Typing,
        Targeting
    };

    explicit TrayIcon(QObject* parent = nullptr);
    ~TrayIcon();

    void show();
    void hide();

    void setIconState(IconState state);
    void showMessage(const QString& title, const QString& message,
                     QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::Information);

signals:
    void activated();
    void settingsRequested();
    void exitRequested();

private slots:
    void onActivated(QSystemTrayIcon::ActivationReason reason);

private:
    void createContextMenu();
    void updateIcon();
    bool isDarkTheme() const;

    QSystemTrayIcon* m_trayIcon;
    QMenu* m_contextMenu;
    QAction* m_settingsAction;
    QAction* m_exitAction;
    IconState m_iconState;
};

#endif // TRAYICON_H
