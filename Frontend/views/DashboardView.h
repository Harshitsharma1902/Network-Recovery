#pragma once

#include <QWidget>
#include <QList>
#include "models/Connection.h"
#include "models/NetworkEvent.h"
#include "widgets/StatCard.h"
#include "widgets/CustomChart.h"
#include "widgets/NetworkCanvas.h"

class ApiClient;

class DashboardView : public QWidget {
    Q_OBJECT

public:
    explicit DashboardView(ApiClient *apiClient, QWidget *parent = nullptr);

    void updateState(int routerCount, const QList<Connection> &connections);
    void addEvent(const NetworkEvent &event);

signals:
    void navigateToTab(int tabIndex);
    void routerSelected(int routerId);

private:
    void setupUi();

    ApiClient *m_apiClient;

    // Stat Cards
    StatCard *m_totalRoutersCard;
    StatCard *m_activeRoutersCard;
    StatCard *m_failedRoutersCard;
    StatCard *m_totalConnectionsCard;
    StatCard *m_activeConnectionsCard;
    StatCard *m_availabilityCard;

    // Visuals
    NetworkCanvas *m_miniCanvas;
    CustomChart *m_healthDonutChart;
    CustomChart *m_degreeBarChart;

    // Events list preview
    QWidget *m_recentEventsContainer;
    QVBoxLayout *m_recentEventsLayout;
};
