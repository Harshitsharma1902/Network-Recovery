#pragma once

#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QTimer>

class TopBar : public QFrame {
    Q_OBJECT

public:
    explicit TopBar(QWidget *parent = nullptr);

    void setBackendStatus(bool connected, const QString &hostInfo);
    void setNetworkHealth(double availabilityPercent);
    void setUndoEnabled(bool enabled);

signals:
    void refreshRequested();
    void undoRequested();

private slots:
    void updateClock();

private:
    QLabel *m_titleLabel;
    QLabel *m_backendStatusPill;
    QLabel *m_healthPill;
    QLabel *m_clockLabel;
    QPushButton *m_refreshBtn;
    QPushButton *m_undoBtn;
    QTimer *m_clockTimer;
};
