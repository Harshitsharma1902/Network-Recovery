#include "AnalyticsView.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QLabel>
#include <QMap>

AnalyticsView::AnalyticsView(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void AnalyticsView::setupUi() {
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet("background-color: transparent;");

    auto *content = new QWidget(scroll);
    auto *layout = new QVBoxLayout(content);
    layout->setContentsMargins(24, 20, 24, 24);
    layout->setSpacing(20);

    // Title
    auto *title = new QLabel("NOC Network Resilience & Graph Analytics", content);
    title->setStyleSheet("font-size: 18px; font-weight: 800; color: #f8fafc;");
    auto *sub = new QLabel("Statistical distribution of mesh connectivity, failure rates, and graph densities", content);
    sub->setStyleSheet("font-size: 12px; color: #64748b;");
    layout->addWidget(title);
    layout->addWidget(sub);

    // Stat Cards Grid
    auto *statsGrid = new QGridLayout();
    statsGrid->setSpacing(14);

    m_avgWeightCard = new StatCard("Average Link Weight", "0 ms", "Mean path latency cost", "#38bdf8", content);
    m_redundancyCard = new StatCard("Mesh Redundancy", "0.0", "Edges to Nodes ratio (E/V)", "#10b981", content);
    m_mttrCard = new StatCard("Dijkstra Efficiency", "O(V²)", "Shortest path complexity", "#a855f7", content);
    m_eventFrequencyCard = new StatCard("Total Recorded Events", "0", "State transition count", "#f59e0b", content);

    statsGrid->addWidget(m_avgWeightCard, 0, 0);
    statsGrid->addWidget(m_redundancyCard, 0, 1);
    statsGrid->addWidget(m_mttrCard, 0, 2);
    statsGrid->addWidget(m_eventFrequencyCard, 0, 3);
    layout->addLayout(statsGrid);

    // Charts Row 1
    auto *chartsRow1 = new QHBoxLayout();
    chartsRow1->setSpacing(16);

    m_linkAvailabilityDonut = new CustomChart(CustomChart::ChartType::DonutGauge, "Link Availability Health Ratio", content);
    m_linkAvailabilityDonut->setMinimumHeight(240);
    chartsRow1->addWidget(m_linkAvailabilityDonut, 1);

    m_nodeDegreeBar = new CustomChart(CustomChart::ChartType::BarChart, "Router Degree Distribution (Incident Edges)", content);
    m_nodeDegreeBar->setMinimumHeight(240);
    chartsRow1->addWidget(m_nodeDegreeBar, 2);

    layout->addLayout(chartsRow1);

    // Charts Row 2
    auto *chartsRow2 = new QHBoxLayout();
    chartsRow2->setSpacing(16);

    m_eventCategoryBar = new CustomChart(CustomChart::ChartType::BarChart, "Event Frequency by Category", content);
    m_eventCategoryBar->setMinimumHeight(220);
    chartsRow2->addWidget(m_eventCategoryBar);

    layout->addLayout(chartsRow2);

    scroll->setWidget(content);
    mainLayout->addWidget(scroll);
}

void AnalyticsView::updateState(int routerCount, const QList<Connection> &connections) {
    int totalEdges = connections.size();
    int activeEdges = 0;
    int sumWeight = 0;

    QMap<int, int> degrees;
    for (int i = 0; i < routerCount; ++i) degrees[i] = 0;

    for (const auto &conn : connections) {
        sumWeight += conn.weight;
        degrees[conn.u]++;
        degrees[conn.v]++;
        if (!conn.failed) {
            activeEdges++;
        }
    }

    double avgWeight = totalEdges > 0 ? (double(sumWeight) / totalEdges) : 0.0;
    double redundancy = routerCount > 0 ? (double(totalEdges) / routerCount) : 0.0;
    double availability = totalEdges > 0 ? (double(activeEdges) / totalEdges * 100.0) : 100.0;

    m_avgWeightCard->setValue(QString("%1").arg(QString::number(avgWeight, 'f', 1)));
    m_redundancyCard->setValue(QString::number(redundancy, 'f', 2));

    m_linkAvailabilityDonut->setDonutData(
        availability,
        QString("%1%").arg(QString::number(availability, 'f', 0)),
        QString("%1 / %2 UP").arg(activeEdges).arg(totalEdges),
        availability >= 80.0 ? QColor("#10b981") : (availability >= 50.0 ? QColor("#f59e0b") : QColor("#ef4444"))
    );

    QStringList labels;
    QList<double> degVals;
    for (int i = 0; i < routerCount; ++i) {
        labels.append(QString("R%1").arg(i));
        degVals.append(degrees[i]);
    }
    m_nodeDegreeBar->setBarData(labels, degVals, QColor("#38bdf8"));
}

void AnalyticsView::updateEventMetrics(const QList<NetworkEvent> &events) {
    m_eventFrequencyCard->setValue(QString::number(events.size()));

    int failCount = 0;
    int recoverCount = 0;
    int undoCount = 0;
    int routeCount = 0;

    for (const auto &ev : events) {
        switch (ev.category) {
            case EventCategory::Failure: failCount++; break;
            case EventCategory::Recovery: recoverCount++; break;
            case EventCategory::Undo: undoCount++; break;
            case EventCategory::RouteCalculation: routeCount++; break;
            default: break;
        }
    }

    QStringList labels = {"Failures", "Recoveries", "Undos", "Routes"};
    QList<double> counts = {double(failCount), double(recoverCount), double(undoCount), double(routeCount)};
    m_eventCategoryBar->setBarData(labels, counts, QColor("#a855f7"));
}
