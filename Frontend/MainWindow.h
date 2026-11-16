#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include <QStatusBar>
#include <QLabel>
#include "api/ApiClient.h"
#include "widgets/TopBar.h"
#include "widgets/Sidebar.h"
#include "views/DashboardView.h"
#include "views/TopologyView.h"
#include "views/RouterView.h"
#include "views/ConnectionView.h"
#include "views/FailureSimulationView.h"
#include "views/TopologyEditorView.h"
#include "views/RecoveryView.h"
#include "views/HistoryView.h"
#include "views/AnalyticsView.h"
#include "views/SettingsView.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

private slots:
    void onNavSelected(int index);
    void onStateUpdated(int routerCount, const QList<Connection> &connections);
    void onRouteReceived(int src, int dest, bool reachable, int cost, const QList<int> &route);
    void onLinkFailedResult(int u, int v, bool success, const QString &message);
    void onLinkRecoveredResult(int u, int v, bool success, const QString &message);
    void onUndoResult(bool success, const QString &message);
    void onRouterAddedResult(bool success, int routerId, int count, const QString &message);
    void onEdgeSavedResult(bool success, int u, int v, int weight, const QString &message);
    void onEdgeDeletedResult(bool success, int u, int v, const QString &message);
    void onBackendStatusChanged(bool connected, const QString &message);
    void onRequestStatus(bool active, const QString &description);

private:
    void setupUi();
    void setupConnections();
    void loadStyleSheet();
    void recordEvent(const QString &description, EventCategory category, int u = -1, int v = -1, int router = -1);

    ApiClient *m_apiClient;

    TopBar *m_topBar;
    Sidebar *m_sidebar;
    QStackedWidget *m_stackedWidget;

    DashboardView *m_dashboardView;
    TopologyView *m_topologyView;
    RouterView *m_routerView;
    ConnectionView *m_connectionView;
    TopologyEditorView *m_editorView;
    FailureSimulationView *m_failureView;
    RecoveryView *m_recoveryView;
    HistoryView *m_historyView;
    AnalyticsView *m_analyticsView;
    SettingsView *m_settingsView;

    QLabel *m_statusLabel;
    int m_eventCounter;
};
