#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QFrame>
#include "models/Connection.h"
#include "widgets/NetworkCanvas.h"

class ApiClient;

class TopologyView : public QWidget {
    Q_OBJECT

public:
    explicit TopologyView(ApiClient *apiClient, QWidget *parent = nullptr);

    void updateState(int routerCount, const QList<Connection> &connections);
    void selectRouter(int routerId);
    void highlightRoute(const QList<int> &route);
    void clearRoute();

signals:
    void requestRouteAnalysis(int src, int dest);

private slots:
    void onRouterSelected(int routerId);
    void onConnectionSelected(int u, int v);
    void onBackgroundClicked();

protected:
    void showEvent(QShowEvent *event) override;

private:
    void setupUi();
    void showRouterDetails(int routerId);
    void showConnectionDetails(int u, int v);
    void hideDetailsDrawer();

    ApiClient *m_apiClient;
    NetworkCanvas *m_canvas;

    int m_routerCount;
    QList<Connection> m_connections;

    // Side details drawer
    QFrame *m_detailsDrawer;
    QLabel *m_detailsTitle;
    QLabel *m_detailsStatusBadge;
    QLabel *m_detailsSubtitle;
    QTableWidget *m_neighborsTable;
    QPushButton *m_actionBtn1;
    QPushButton *m_actionBtn2;

    int m_currentSelectedRouter;
    int m_currentSelectedU;
    int m_currentSelectedV;
};
