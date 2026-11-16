#include "TopBar.h"
#include <QHBoxLayout>
#include <QDateTime>

TopBar::TopBar(QWidget *parent)
    : QFrame(parent)
    , m_clockTimer(new QTimer(this))
{
    setFixedHeight(64);
    setObjectName("topBar");
    setStyleSheet(
        "QFrame#topBar {"
        "  background-color: #0b0f19;"
        "  border-bottom: 1px solid #1e293b;"
        "  padding: 0 16px;"
        "}"
    );

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(16, 0, 16, 0);
    layout->setSpacing(14);

    // Title / Branding
    auto *brandLayout = new QVBoxLayout();
    brandLayout->setSpacing(2);
    brandLayout->setAlignment(Qt::AlignVCenter);

    m_titleLabel = new QLabel("NETWORK RECOVERY SIMULATOR", this);
    m_titleLabel->setStyleSheet("font-size: 15px; font-weight: 800; color: #f8fafc; letter-spacing: 1px;");

    auto *subLabel = new QLabel("NOC Topology Resilience & Dynamic Routing Engine", this);
    subLabel->setStyleSheet("font-size: 11px; color: #64748b; font-weight: 500;");

    brandLayout->addWidget(m_titleLabel);
    brandLayout->addWidget(subLabel);
    layout->addLayout(brandLayout);

    layout->addStretch();

    // Backend Status Badge
    m_backendStatusPill = new QLabel("CONNECTING...", this);
    m_backendStatusPill->setStyleSheet(
        "background-color: #1e293b; color: #94a3b8; font-weight: 700; "
        "font-size: 11px; padding: 5px 12px; border-radius: 12px; border: 1px solid #334155;"
    );
    layout->addWidget(m_backendStatusPill);

    // Network Health Badge
    m_healthPill = new QLabel("HEALTH: --", this);
    m_healthPill->setStyleSheet(
        "background-color: #064e3b; color: #34d399; font-weight: 700; "
        "font-size: 11px; padding: 5px 12px; border-radius: 12px; border: 1px solid #059669;"
    );
    layout->addWidget(m_healthPill);

    // Live Clock
    m_clockLabel = new QLabel(this);
    m_clockLabel->setStyleSheet("font-size: 12px; color: #94a3b8; font-family: monospace; font-weight: 600;");
    layout->addWidget(m_clockLabel);

    // Undo Button
    m_undoBtn = new QPushButton("↶ Undo Failure", this);
    m_undoBtn->setCursor(Qt::PointingHandCursor);
    m_undoBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #312e81; color: #c7d2fe; border: 1px solid #4338ca;"
        "  font-size: 12px; font-weight: 600; padding: 6px 14px; border-radius: 6px;"
        "}"
        "QPushButton:hover { background-color: #3730a3; }"
        "QPushButton:pressed { background-color: #1e1b4b; }"
    );
    connect(m_undoBtn, &QPushButton::clicked, this, &TopBar::undoRequested);
    layout->addWidget(m_undoBtn);

    // Refresh Button
    m_refreshBtn = new QPushButton("↻ Refresh State", this);
    m_refreshBtn->setCursor(Qt::PointingHandCursor);
    m_refreshBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #0284c7; color: #ffffff; border: 1px solid #0369a1;"
        "  font-size: 12px; font-weight: 600; padding: 6px 14px; border-radius: 6px;"
        "}"
        "QPushButton:hover { background-color: #0369a1; }"
        "QPushButton:pressed { background-color: #075985; }"
    );
    connect(m_refreshBtn, &QPushButton::clicked, this, &TopBar::refreshRequested);
    layout->addWidget(m_refreshBtn);

    connect(m_clockTimer, &QTimer::timeout, this, &TopBar::updateClock);
    m_clockTimer->start(1000);
    updateClock();
}

void TopBar::setBackendStatus(bool connected, const QString &hostInfo) {
    if (connected) {
        m_backendStatusPill->setText(QString("● ONLINE  %1").arg(hostInfo.isEmpty() ? "localhost:8080" : hostInfo));
        m_backendStatusPill->setStyleSheet(
            "background-color: #064e3b; color: #34d399; font-weight: 700; "
            "font-size: 11px; padding: 5px 12px; border-radius: 12px; border: 1px solid #059669;"
        );
    } else {
        m_backendStatusPill->setText(QString("● OFFLINE  %1").arg(hostInfo.isEmpty() ? "localhost:8080" : hostInfo));
        m_backendStatusPill->setStyleSheet(
            "background-color: #450a0a; color: #f87171; font-weight: 700; "
            "font-size: 11px; padding: 5px 12px; border-radius: 12px; border: 1px solid #b91c1c;"
        );
    }
}

void TopBar::setNetworkHealth(double availabilityPercent) {
    QString color = "#34d399";
    QString bgColor = "#064e3b";
    QString border = "#059669";

    if (availabilityPercent < 50.0) {
        color = "#f87171";
        bgColor = "#450a0a";
        border = "#b91c1c";
    } else if (availabilityPercent < 80.0) {
        color = "#fbbf24";
        bgColor = "#451a03";
        border = "#d97706";
    }

    m_healthPill->setText(QString("HEALTH: %1%").arg(QString::number(availabilityPercent, 'f', 1)));
    m_healthPill->setStyleSheet(QString(
        "background-color: %1; color: %2; font-weight: 700; "
        "font-size: 11px; padding: 5px 12px; border-radius: 12px; border: 1px solid %3;"
    ).arg(bgColor, color, border));
}

void TopBar::setUndoEnabled(bool enabled) {
    m_undoBtn->setEnabled(enabled);
}

void TopBar::updateClock() {
    m_clockLabel->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd  hh:mm:ss"));
}
