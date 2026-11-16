#pragma once

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QTableWidget>
#include <QList>
#include "models/Connection.h"
#include "widgets/NetworkCanvas.h"

class ApiClient;

class RecoveryView : public QWidget {
    Q_OBJECT

public:
    explicit RecoveryView(ApiClient *apiClient, QWidget *parent = nullptr);

    void updateState(int routerCount, const QList<Connection> &connections);
    void setRouteResult(int src, int dest, bool reachable, int cost, const QList<int> &route);
    void setSourceAndDest(int src, int dest);

private slots:
    void computeRoute();
    void clearRoute();

protected:
    void showEvent(QShowEvent *event) override;

private:
    void setupUi();
    void updateDropdowns();

    ApiClient *m_apiClient;
    int m_routerCount;
    QList<Connection> m_connections;

    // Controls
    QComboBox *m_srcCombo;
    QComboBox *m_destCombo;
    QPushButton *m_computeBtn;
    QPushButton *m_clearBtn;

    // Telemetry displays
    QLabel *m_bfsStatusBadge;
    QLabel *m_dijkstraCostLabel;
    QLabel *m_routePathLabel;
    QTableWidget *m_hopsTable;
    QLabel *m_unreachableAlert;

    // Canvas
    NetworkCanvas *m_canvas;
};
