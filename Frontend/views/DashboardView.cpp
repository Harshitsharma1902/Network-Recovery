#include "DashboardView.h"
#include "api/ApiClient.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QPushButton>
#include <QLabel>
#include <QFrame>

DashboardView::DashboardView(ApiClient *apiClient, QWidget *parent)
    : QWidget(parent)
    , m_apiClient(apiClient)
{
    setupUi();
}

void DashboardView::setupUi() {
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("background-color: transparent;");

    auto *contentWidget = new QWidget(scrollArea);
    auto *layout = new QVBoxLayout(contentWidget);
    layout->setContentsMargins(24, 20, 24, 24);
    layout->setSpacing(20);

    // Header & Quick Actions
    auto *headerLayout = new QHBoxLayout();
    auto *titleLayout = new QVBoxLayout();
    auto *headerLabel = new QLabel("NOC Operational Overview", contentWidget);
    headerLabel->setStyleSheet("font-size: 20px; font-weight: 800; color: #f8fafc;");
    auto *subLabel = new QLabel("Real-time network telemetry, link status, and failure recovery metrics", contentWidget);
    subLabel->setStyleSheet("font-size: 12px; color: #64748b;");
    titleLayout->addWidget(headerLabel);
    titleLayout->addWidget(subLabel);
    headerLayout->addLayout(titleLayout);
    headerLayout->addStretch();

    // Quick Action Buttons
    auto *editBtn = new QPushButton("✏ Edit Topology", contentWidget);
    editBtn->setCursor(Qt::PointingHandCursor);
    connect(editBtn, &QPushButton::clicked, this, [this]() { emit navigateToTab(4); });
    headerLayout->addWidget(editBtn);

    auto *failBtn = new QPushButton("⚡ Simulate Failure", contentWidget);
    failBtn->setObjectName("dangerBtn");
    failBtn->setCursor(Qt::PointingHandCursor);
    connect(failBtn, &QPushButton::clicked, this, [this]() { emit navigateToTab(5); });
    headerLayout->addWidget(failBtn);

    auto *recoverBtn = new QPushButton("🎯 Recovery Path", contentWidget);
    recoverBtn->setObjectName("primaryBtn");
    recoverBtn->setCursor(Qt::PointingHandCursor);
    connect(recoverBtn, &QPushButton::clicked, this, [this]() { emit navigateToTab(6); });
    headerLayout->addWidget(recoverBtn);

    auto *undoBtn = new QPushButton("↶ Undo Failure", contentWidget);
    undoBtn->setCursor(Qt::PointingHandCursor);
    connect(undoBtn, &QPushButton::clicked, this, [this]() { m_apiClient->undoLastFailure(); });
    headerLayout->addWidget(undoBtn);

    layout->addLayout(headerLayout);

    // 6 Statistics Cards in a grid
    auto *statsGrid = new QGridLayout();
    statsGrid->setSpacing(14);

    m_totalRoutersCard = new StatCard("Total Routers", "0", "Indexed nodes in graph", "#38bdf8", contentWidget);
    m_activeRoutersCard = new StatCard("Active Routers", "0", "Online & operational", "#10b981", contentWidget);
    m_failedRoutersCard = new StatCard("Isolated Routers", "0", "No active links", "#ef4444", contentWidget);
    m_totalConnectionsCard = new StatCard("Total Links", "0", "Bidirectional edges", "#818cf8", contentWidget);
    m_activeConnectionsCard = new StatCard("Active Links", "0", "Operational connections", "#06b6d4", contentWidget);
    m_availabilityCard = new StatCard("Network Availability", "100%", "Topology connectivity ratio", "#f59e0b", contentWidget);

    statsGrid->addWidget(m_totalRoutersCard, 0, 0);
    statsGrid->addWidget(m_activeRoutersCard, 0, 1);
    statsGrid->addWidget(m_failedRoutersCard, 0, 2);
    statsGrid->addWidget(m_totalConnectionsCard, 1, 0);
    statsGrid->addWidget(m_activeConnectionsCard, 1, 1);
    statsGrid->addWidget(m_availabilityCard, 1, 2);

    layout->addLayout(statsGrid);

    // Middle Row: Topology Preview & Charts
    auto *middleLayout = new QHBoxLayout();
    middleLayout->setSpacing(16);

    // Left: Mini Topology Preview Card
    auto *canvasCard = new QFrame(contentWidget);
    canvasCard->setStyleSheet("background-color: #0f172a; border: 1px solid #1e293b; border-radius: 8px;");
    auto *canvasCardLayout = new QVBoxLayout(canvasCard);
    canvasCardLayout->setContentsMargins(14, 12, 14, 14);

    auto *canvasHeaderLayout = new QHBoxLayout();
    auto *canvasTitle = new QLabel("LIVE TOPOLOGY PREVIEW", canvasCard);
    canvasTitle->setStyleSheet("font-size: 11px; font-weight: 700; color: #94a3b8; letter-spacing: 0.8px;");
    canvasHeaderLayout->addWidget(canvasTitle);
    canvasHeaderLayout->addStretch();

    auto *viewFullBtn = new QPushButton("Open Full View ➔", canvasCard);
    viewFullBtn->setStyleSheet("background: transparent; border: none; color: #38bdf8; font-weight: 600; font-size: 12px;");
    viewFullBtn->setCursor(Qt::PointingHandCursor);
    connect(viewFullBtn, &QPushButton::clicked, this, [this]() { emit navigateToTab(1); });
    canvasHeaderLayout->addWidget(viewFullBtn);
    canvasCardLayout->addLayout(canvasHeaderLayout);

    m_miniCanvas = new NetworkCanvas(canvasCard);
    m_miniCanvas->setMinimumHeight(280);
    connect(m_miniCanvas, &NetworkCanvas::routerSelected, this, &DashboardView::routerSelected);
    canvasCardLayout->addWidget(m_miniCanvas);

    middleLayout->addWidget(canvasCard, 3);

    // Right: Donut health gauge & Degree distribution
    auto *chartsCol = new QVBoxLayout();
    chartsCol->setSpacing(14);

    m_healthDonutChart = new CustomChart(CustomChart::ChartType::DonutGauge, "Availability Index", contentWidget);
    m_healthDonutChart->setMinimumHeight(170);
    chartsCol->addWidget(m_healthDonutChart);

    m_degreeBarChart = new CustomChart(CustomChart::ChartType::BarChart, "Router Degree Telemetry", contentWidget);
    m_degreeBarChart->setMinimumHeight(170);
    chartsCol->addWidget(m_degreeBarChart);

    middleLayout->addLayout(chartsCol, 2);
    layout->addLayout(middleLayout);

    // Bottom Row: Recent Events Feed Card
    auto *eventsCard = new QFrame(contentWidget);
    eventsCard->setStyleSheet("background-color: #0f172a; border: 1px solid #1e293b; border-radius: 8px;");
    auto *eventsCardLayout = new QVBoxLayout(eventsCard);
    eventsCardLayout->setContentsMargins(16, 14, 16, 14);
    eventsCardLayout->setSpacing(10);

    auto *eventsHeader = new QHBoxLayout();
    auto *eventsTitle = new QLabel("RECENT NETWORK EVENTS", eventsCard);
    eventsTitle->setStyleSheet("font-size: 11px; font-weight: 700; color: #94a3b8; letter-spacing: 0.8px;");
    eventsHeader->addWidget(eventsTitle);
    eventsHeader->addStretch();

    auto *viewAllEventsBtn = new QPushButton("View Event History ➔", eventsCard);
    viewAllEventsBtn->setStyleSheet("background: transparent; border: none; color: #38bdf8; font-weight: 600; font-size: 12px;");
    viewAllEventsBtn->setCursor(Qt::PointingHandCursor);
    connect(viewAllEventsBtn, &QPushButton::clicked, this, [this]() { emit navigateToTab(7); });
    eventsHeader->addWidget(viewAllEventsBtn);
    eventsCardLayout->addLayout(eventsHeader);

    m_recentEventsContainer = new QWidget(eventsCard);
    m_recentEventsLayout = new QVBoxLayout(m_recentEventsContainer);
    m_recentEventsLayout->setContentsMargins(0, 0, 0, 0);
    m_recentEventsLayout->setSpacing(6);
    eventsCardLayout->addWidget(m_recentEventsContainer);

    layout->addWidget(eventsCard);

    scrollArea->setWidget(contentWidget);
    mainLayout->addWidget(scrollArea);
}

void DashboardView::updateState(int routerCount, const QList<Connection> &connections) {
    int totalEdges = connections.size();
    int activeEdges = 0;
    int failedEdges = 0;

    QMap<int, int> routerDegrees;
    QMap<int, int> routerActiveDegrees;

    for (int i = 0; i < routerCount; ++i) {
        routerDegrees[i] = 0;
        routerActiveDegrees[i] = 0;
    }

    for (const auto &conn : connections) {
        routerDegrees[conn.u]++;
        routerDegrees[conn.v]++;
        if (conn.failed) {
            failedEdges++;
        } else {
            activeEdges++;
            routerActiveDegrees[conn.u]++;
            routerActiveDegrees[conn.v]++;
        }
    }

    int activeRouters = 0;
    int isolatedRouters = 0;
    for (int i = 0; i < routerCount; ++i) {
        if (routerActiveDegrees[i] > 0 || routerDegrees[i] == 0) {
            activeRouters++;
        } else {
            isolatedRouters++;
        }
    }

    double availability = totalEdges > 0 ? (double(activeEdges) / totalEdges * 100.0) : 100.0;

    // Update Stat Cards
    m_totalRoutersCard->setValue(QString::number(routerCount));
    m_activeRoutersCard->setValue(QString::number(activeRouters));
    m_failedRoutersCard->setValue(QString::number(isolatedRouters));
    m_totalConnectionsCard->setValue(QString::number(totalEdges));
    m_activeConnectionsCard->setValue(QString::number(activeEdges));
    m_availabilityCard->setValue(QString("%1%").arg(QString::number(availability, 'f', 1)));

    // Update Mini Canvas
    m_miniCanvas->setNetworkData(routerCount, connections);

    // Update Donut Chart
    m_healthDonutChart->setDonutData(
        availability,
        QString("%1%").arg(QString::number(availability, 'f', 0)),
        activeEdges == totalEdges ? "ALL LINKS UP" : QString("%1 LINKS DOWN").arg(failedEdges),
        availability >= 80.0 ? QColor("#10b981") : (availability >= 50.0 ? QColor("#f59e0b") : QColor("#ef4444"))
    );

    // Update Degree Bar Chart
    QStringList labels;
    QList<double> degrees;
    for (int i = 0; i < routerCount; ++i) {
        labels.append(QString("R%1").arg(i));
        degrees.append(routerActiveDegrees[i]);
    }
    m_degreeBarChart->setBarData(labels, degrees, QColor("#0ea5e9"));
}

void DashboardView::addEvent(const NetworkEvent &event) {
    // Keep max 5 recent events in preview
    if (m_recentEventsLayout->count() >= 5) {
        auto *item = m_recentEventsLayout->takeAt(m_recentEventsLayout->count() - 1);
        if (item->widget()) delete item->widget();
        delete item;
    }

    auto *row = new QFrame(m_recentEventsContainer);
    row->setStyleSheet("background-color: #111827; border: 1px solid #1f2937; border-radius: 6px; padding: 6px;");
    auto *rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(10, 6, 10, 6);
    rowLayout->setSpacing(12);

    auto *badge = new QLabel(event.categoryName(), row);
    badge->setStyleSheet(QString(
        "background-color: %1; color: #ffffff; font-weight: 700; "
        "font-size: 10px; padding: 3px 8px; border-radius: 4px;"
    ).arg(event.categoryColor()));
    rowLayout->addWidget(badge);

    auto *descLabel = new QLabel(event.description, row);
    descLabel->setStyleSheet("color: #e2e8f0; font-size: 12px; font-weight: 500;");
    rowLayout->addWidget(descLabel);

    rowLayout->addStretch();

    auto *timeLabel = new QLabel(event.timestamp.toString("hh:mm:ss"), row);
    timeLabel->setStyleSheet("color: #64748b; font-size: 11px; font-family: monospace;");
    rowLayout->addWidget(timeLabel);

    m_recentEventsLayout->insertWidget(0, row);
}
