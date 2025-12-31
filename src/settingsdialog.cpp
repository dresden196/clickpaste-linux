#include "settingsdialog.h"
#include "settings.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QRadioButton>
#include <QPushButton>
#include <QKeyEvent>

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("ClickPaste Settings"));
    setMinimumWidth(400);

    setupUI();
    loadSettings();
}

void SettingsDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    mainLayout->addWidget(createDelaysGroup());
    mainLayout->addWidget(createConfirmationGroup());
    mainLayout->addWidget(createHotkeyGroup());
    mainLayout->addWidget(createModeGroup());

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    QPushButton* doneButton = new QPushButton(QStringLiteral("Done"));
    doneButton->setDefault(true);
    connect(doneButton, &QPushButton::clicked, this, [this]() {
        saveSettings();
        accept();
    });
    buttonLayout->addWidget(doneButton);

    QPushButton* cancelButton = new QPushButton(QStringLiteral("Cancel"));
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonLayout);
}

QGroupBox* SettingsDialog::createDelaysGroup()
{
    QGroupBox* group = new QGroupBox(QStringLiteral("Delays"));
    QGridLayout* layout = new QGridLayout(group);

    layout->addWidget(new QLabel(QStringLiteral("Start Delay (ms):")), 0, 0);
    m_startDelaySpinBox = new QSpinBox();
    m_startDelaySpinBox->setRange(0, 10000);
    m_startDelaySpinBox->setSingleStep(100);
    layout->addWidget(m_startDelaySpinBox, 0, 1);

    layout->addWidget(new QLabel(QStringLiteral("Key Delay (ms):")), 1, 0);
    m_keyDelaySpinBox = new QSpinBox();
    m_keyDelaySpinBox->setRange(0, 1000);
    m_keyDelaySpinBox->setSingleStep(5);
    layout->addWidget(m_keyDelaySpinBox, 1, 1);

    layout->setColumnStretch(1, 1);
    return group;
}

QGroupBox* SettingsDialog::createConfirmationGroup()
{
    QGroupBox* group = new QGroupBox(QStringLiteral("Confirmation"));
    QGridLayout* layout = new QGridLayout(group);

    m_confirmCheckBox = new QCheckBox(QStringLiteral("Confirm before pasting"));
    layout->addWidget(m_confirmCheckBox, 0, 0, 1, 2);

    layout->addWidget(new QLabel(QStringLiteral("When text exceeds:")), 1, 0);
    m_confirmThresholdSpinBox = new QSpinBox();
    m_confirmThresholdSpinBox->setRange(1, 100000);
    m_confirmThresholdSpinBox->setSingleStep(100);
    m_confirmThresholdSpinBox->setSuffix(QStringLiteral(" characters"));
    layout->addWidget(m_confirmThresholdSpinBox, 1, 1);

    connect(m_confirmCheckBox, &QCheckBox::toggled,
            m_confirmThresholdSpinBox, &QSpinBox::setEnabled);

    layout->setColumnStretch(1, 1);
    return group;
}

QGroupBox* SettingsDialog::createHotkeyGroup()
{
    QGroupBox* group = new QGroupBox(QStringLiteral("Hotkey"));
    QGridLayout* layout = new QGridLayout(group);

    layout->addWidget(new QLabel(QStringLiteral("Key:")), 0, 0);
    m_hotkeyEdit = new QLineEdit();
    m_hotkeyEdit->setMaxLength(1);
    m_hotkeyEdit->setPlaceholderText(QStringLiteral("V"));
    m_hotkeyEdit->setMaximumWidth(50);
    layout->addWidget(m_hotkeyEdit, 0, 1);

    layout->addWidget(new QLabel(QStringLiteral("Modifiers:")), 1, 0);

    QHBoxLayout* modLayout = new QHBoxLayout();
    m_altCheckBox = new QCheckBox(QStringLiteral("Alt"));
    m_ctrlCheckBox = new QCheckBox(QStringLiteral("Ctrl"));
    m_shiftCheckBox = new QCheckBox(QStringLiteral("Shift"));
    m_superCheckBox = new QCheckBox(QStringLiteral("Super"));

    modLayout->addWidget(m_altCheckBox);
    modLayout->addWidget(m_ctrlCheckBox);
    modLayout->addWidget(m_shiftCheckBox);
    modLayout->addWidget(m_superCheckBox);
    modLayout->addStretch();

    layout->addLayout(modLayout, 1, 1);

    layout->setColumnStretch(1, 1);
    return group;
}

QGroupBox* SettingsDialog::createModeGroup()
{
    QGroupBox* group = new QGroupBox(QStringLiteral("Hotkey Mode"));
    QVBoxLayout* layout = new QVBoxLayout(group);

    m_targetModeRadio = new QRadioButton(QStringLiteral("Target Mode - Show overlay to select target window"));
    m_justGoModeRadio = new QRadioButton(QStringLiteral("Just Go Mode - Type immediately to focused window"));

    layout->addWidget(m_targetModeRadio);
    layout->addWidget(m_justGoModeRadio);

    return group;
}

void SettingsDialog::loadSettings()
{
    Settings* s = Settings::instance();

    m_startDelaySpinBox->setValue(s->startDelayMs());
    m_keyDelaySpinBox->setValue(s->keyDelayMs());

    m_confirmCheckBox->setChecked(s->confirmEnabled());
    m_confirmThresholdSpinBox->setValue(s->confirmThreshold());
    m_confirmThresholdSpinBox->setEnabled(s->confirmEnabled());

    m_hotkeyEdit->setText(s->hotkey());

    Qt::KeyboardModifiers mods = s->hotkeyModifiers();
    m_altCheckBox->setChecked(mods & Qt::AltModifier);
    m_ctrlCheckBox->setChecked(mods & Qt::ControlModifier);
    m_shiftCheckBox->setChecked(mods & Qt::ShiftModifier);
    m_superCheckBox->setChecked(mods & Qt::MetaModifier);

    if (s->hotkeyMode() == Settings::JustGo) {
        m_justGoModeRadio->setChecked(true);
    } else {
        m_targetModeRadio->setChecked(true);
    }
}

void SettingsDialog::saveSettings()
{
    Settings* s = Settings::instance();

    s->setStartDelayMs(m_startDelaySpinBox->value());
    s->setKeyDelayMs(m_keyDelaySpinBox->value());

    s->setConfirmEnabled(m_confirmCheckBox->isChecked());
    s->setConfirmThreshold(m_confirmThresholdSpinBox->value());

    QString key = m_hotkeyEdit->text().toUpper();
    if (key.isEmpty()) {
        key = QStringLiteral("V");
    }
    s->setHotkey(key);

    Qt::KeyboardModifiers mods = Qt::NoModifier;
    if (m_altCheckBox->isChecked()) mods |= Qt::AltModifier;
    if (m_ctrlCheckBox->isChecked()) mods |= Qt::ControlModifier;
    if (m_shiftCheckBox->isChecked()) mods |= Qt::ShiftModifier;
    if (m_superCheckBox->isChecked()) mods |= Qt::MetaModifier;
    s->setHotkeyModifiers(mods);

    s->setHotkeyMode(m_justGoModeRadio->isChecked() ? Settings::JustGo : Settings::Target);

    s->sync();
}

void SettingsDialog::onHotkeyKeyPress()
{
    // This could be used to capture a key press for the hotkey field
}
