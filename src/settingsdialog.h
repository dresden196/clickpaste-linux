#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

class QSpinBox;
class QCheckBox;
class QLineEdit;
class QRadioButton;
class QGroupBox;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);
    ~SettingsDialog() = default;

private Q_SLOTS:
    void loadSettings();
    void saveSettings();
    void onHotkeyKeyPress();

private:
    void setupUI();
    QGroupBox* createDelaysGroup();
    QGroupBox* createConfirmationGroup();
    QGroupBox* createHotkeyGroup();
    QGroupBox* createModeGroup();

    // Delay controls
    QSpinBox* m_startDelaySpinBox;
    QSpinBox* m_keyDelaySpinBox;

    // Confirmation controls
    QCheckBox* m_confirmCheckBox;
    QSpinBox* m_confirmThresholdSpinBox;

    // Hotkey controls
    QLineEdit* m_hotkeyEdit;
    QCheckBox* m_altCheckBox;
    QCheckBox* m_ctrlCheckBox;
    QCheckBox* m_shiftCheckBox;
    QCheckBox* m_superCheckBox;

    // Mode controls
    QRadioButton* m_targetModeRadio;
    QRadioButton* m_justGoModeRadio;
};

#endif // SETTINGSDIALOG_H
