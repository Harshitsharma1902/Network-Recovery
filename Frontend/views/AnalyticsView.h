#pragma once

#include <QWidget>
#include <QList>
#include "models/Connection.h"
#include "models/NetworkEvent.h"
#include "widgets/CustomChart.h"
#include "widgets/StatCard.h"

class AnalyticsView : public QWidget {
    Q_OBJECT

public:
    explicit AnalyticsView(QWidget *parent = nullptr);

    void updateState(int routerCount, const QList<Connection> &connections);
    void updateEventMetrics(const QList<NetworkEvent> &events);

private:
    void setupUi();

    // Stats
    StatCard *m_mttrCard;
    StatCard *m_avgWeightCard;
    StatCard *m_redundancyCard;
    StatCard *m_eventFrequencyCard;

    // Charts
    CustomChart *m_linkAvailabilityDonut;
    CustomChart *m_nodeDegreeBar;
    CustomChart *m_eventCategoryBar;
};
