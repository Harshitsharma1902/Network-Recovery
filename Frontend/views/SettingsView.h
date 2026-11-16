#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QCheckBox>
#include <QSlider>
#include <QLabel>
#include <QPushButton>

class ApiClient;

class SettingsView : public QWidget {
    Q_OBJECT

public:
    explicit SettingsView(ApiClient *apiClient, QWidget *parent = nullptr);

private slots:
    void saveSettings();
    void testConnection();

private:
    void setupUi();

    ApiClient *m_apiClient;

    QLineEdit *m_baseUrlEdit;
    QCheckBox *m_autoRefreshCheck;
    QSlider *m_intervalSlider;
    QLabel *m_intervalLabel;
    QPushButton *m_saveBtn;
    QPushButton *m_testBtn;
    QLabel *m_testResultLabel;
};
