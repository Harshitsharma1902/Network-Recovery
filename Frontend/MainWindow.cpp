#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFile>
#include <QDir>
#include <QCoreApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_apiClient(new ApiClient(this))
    , m_eventCounter(1)
{
    setWindowTitle("Network Failure Recovery Simulator — Enterprise NOC Dashboard");
    resize(1280, 840);
    setMinimumSize(1024, 700);

    setupUi();
    loadStyleSheet();
    setupConnections();

    // Load initial events from backend data file if exists
    QString eventsFilePath = QDir(QCoreApplication::applicationDirPath()).filePath("../../Backend/data/events.txt");
    if (!QFile::exists(eventsFilePath)) {
        eventsFilePath = "Backend/data/events.txt";
    }
    m_historyView->loadInitialEventsFromFile(eventsFilePath);

    // Initial state fetch & auto-refresh startup
    m_apiClient->startAutoRefresh(2500);
}

void MainWindow::setupUi() {
    auto *centralWidget = new QWidget(this);
    centralWidget->setObjectName("centralWidget");
    setCentralWidget(centralWidget);

    auto *rootLayout = new QVBoxLayout(centralWidget);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // Top Navigation Bar
    m_topBar = new TopBar(centralWidget);
    rootLayout->addWidget(m_topBar);

    // Body: Sidebar + Stacked Content Area
    auto *bodyLayout = new QHBoxLayout();
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->setSpacing(0);

    m_sidebar = new Sidebar(centralWidget);
    bodyLayout->addWidget(m_sidebar);

    m_stackedWidget = new QStackedWidget(centralWidget);
    m_stackedWidget->setStyleSheet("background-color: #0b0f19;");

    // Initialize 10 Views
    m_dashboardView = new DashboardView(m_apiClient, m_stackedWidget);
    m_topologyView = new TopologyView(m_apiClient, m_stackedWidget);
    m_routerView = new RouterView(m_apiClient, m_stackedWidget);
    m_connectionView = new ConnectionView(m_apiClient, m_stackedWidget);
    m_editorView = new TopologyEditorView(m_apiClient, m_stackedWidget);
    m_failureView = new FailureSimulationView(m_apiClient, m_stackedWidget);
    m_recoveryView = new RecoveryView(m_apiClient, m_stackedWidget);
    m_historyView = new HistoryView(m_stackedWidget);
    m_analyticsView = new AnalyticsView(m_stackedWidget);
    m_settingsView = new SettingsView(m_apiClient, m_stackedWidget);

    m_stackedWidget->addWidget(m_dashboardView);    // 0
    m_stackedWidget->addWidget(m_topologyView);     // 1
    m_stackedWidget->addWidget(m_routerView);       // 2
    m_stackedWidget->addWidget(m_connectionView);   // 3
    m_stackedWidget->addWidget(m_editorView);       // 4
    m_stackedWidget->addWidget(m_failureView);      // 5
    m_stackedWidget->addWidget(m_recoveryView);     // 6
    m_stackedWidget->addWidget(m_historyView);      // 7
    m_stackedWidget->addWidget(m_analyticsView);    // 8
    m_stackedWidget->addWidget(m_settingsView);     // 9

    bodyLayout->addWidget(m_stackedWidget, 1);
    rootLayout->addLayout(bodyLayout, 1);

    // Status Bar
    m_statusLabel = new QLabel("System Initialized | NOC Monitoring Ready", this);
    m_statusLabel->setStyleSheet("color: #64748b; font-size: 11px; padding-left: 8px;");
    statusBar()->addWidget(m_statusLabel, 1);
    statusBar()->setStyleSheet("QStatusBar { background-color: #0b0f19; border-top: 1px solid #1e293b; color: #94a3b8; }");
}

void MainWindow::loadStyleSheet() {
    QString stylePath = ":/styles/dark_theme.qss";
    QFile file(stylePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // Fallback to relative file
        file.setFileName("Frontend/resources/styles/dark_theme.qss");
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return;
        }
    }

    if (file.isOpen()) {
        setStyleSheet(file.readAll());
        file.close();
    }
}

void MainWindow::setupConnections() {
    // Navigation
    connect(m_sidebar, &Sidebar::navSelected, this, &MainWindow::onNavSelected);

    connect(m_dashboardView, &DashboardView::navigateToTab, this, [this](int idx) {
        m_sidebar->setCurrentIndex(idx);
        m_stackedWidget->setCurrentIndex(idx);
    });

    connect(m_dashboardView, &DashboardView::routerSelected, this, [this](int id) {
        m_sidebar->setCurrentIndex(1);
        m_stackedWidget->setCurrentIndex(1);
        m_topologyView->selectRouter(id);
    });

    connect(m_routerView, &RouterView::inspectRouterInTopology, this, [this](int id) {
        m_sidebar->setCurrentIndex(1);
        m_stackedWidget->setCurrentIndex(1);
        m_topologyView->selectRouter(id);
    });

    connect(m_connectionView, &ConnectionView::navigateToEditor, this, [this]() {
        m_sidebar->setCurrentIndex(4);
        m_stackedWidget->setCurrentIndex(4);
    });

    connect(m_topologyView, &TopologyView::requestRouteAnalysis, this, [this](int src, int dest) {
        m_sidebar->setCurrentIndex(6);
        m_stackedWidget->setCurrentIndex(6);
        m_recoveryView->setSourceAndDest(src, dest);
    });

    // TopBar
    connect(m_topBar, &TopBar::refreshRequested, m_apiClient, &ApiClient::fetchState);
    connect(m_topBar, &TopBar::undoRequested, m_apiClient, &ApiClient::undoLastFailure);

    // ApiClient
    connect(m_apiClient, &ApiClient::stateUpdated, this, &MainWindow::onStateUpdated);
    connect(m_apiClient, &ApiClient::routeReceived, this, &MainWindow::onRouteReceived);
    connect(m_apiClient, &ApiClient::linkFailedResult, this, &MainWindow::onLinkFailedResult);
    connect(m_apiClient, &ApiClient::linkRecoveredResult, this, &MainWindow::onLinkRecoveredResult);
    connect(m_apiClient, &ApiClient::undoResult, this, &MainWindow::onUndoResult);
    connect(m_apiClient, &ApiClient::routerAddedResult, this, &MainWindow::onRouterAddedResult);
    connect(m_apiClient, &ApiClient::edgeSavedResult, this, &MainWindow::onEdgeSavedResult);
    connect(m_apiClient, &ApiClient::edgeDeletedResult, this, &MainWindow::onEdgeDeletedResult);
    connect(m_apiClient, &ApiClient::backendStatusChanged, this, &MainWindow::onBackendStatusChanged);
    connect(m_apiClient, &ApiClient::requestStatus, this, &MainWindow::onRequestStatus);
}

void MainWindow::onNavSelected(int index) {
    m_stackedWidget->setCurrentIndex(index);
}

void MainWindow::onStateUpdated(int routerCount, const QList<Connection> &connections) {
    m_dashboardView->updateState(routerCount, connections);
    m_topologyView->updateState(routerCount, connections);
    m_routerView->updateState(routerCount, connections);
    m_connectionView->updateState(routerCount, connections);
    m_editorView->updateState(routerCount, connections);
    m_failureView->updateState(routerCount, connections);
    m_recoveryView->updateState(routerCount, connections);
    m_analyticsView->updateState(routerCount, connections);

    // Compute health percentage
    int total = connections.size();
    int active = 0;
    for (const auto &c : connections) {
        if (!c.failed) active++;
    }
    double availability = total > 0 ? (double(active) / total * 100.0) : 100.0;
    m_topBar->setNetworkHealth(availability);
}

void MainWindow::onRouteReceived(int src, int dest, bool reachable, int cost, const QList<int> &route) {
    m_recoveryView->setRouteResult(src, dest, reachable, cost, route);

    if (reachable) {
        QStringList pathStrs;
        for (int node : route) pathStrs.append(QString("R%1").arg(node));
        recordEvent(QString("Dijkstra Route R%1 -> R%2 computed (Cost: %3 ms, Path: %4)")
                        .arg(src).arg(dest).arg(cost).arg(pathStrs.join(" -> ")),
                    EventCategory::RouteCalculation, src, dest);
    } else {
        recordEvent(QString("Route R%1 -> R%2 unreachable: BFS found no active connected path")
                        .arg(src).arg(dest),
                    EventCategory::Warning, src, dest);
    }
}

void MainWindow::onLinkFailedResult(int u, int v, bool success, const QString &message) {
    if (success) {
        recordEvent(QString("Link R%1-R%2 severed (Failed in Adjacency Matrix)").arg(u).arg(v),
                    EventCategory::Failure, u, v);
        m_statusLabel->setText(QString("Failure Injected: Link R%1-R%2 severed").arg(u).arg(v));
    } else {
        recordEvent(QString("Failed to sever link R%1-R%2: %3").arg(u).arg(v).arg(message),
                    EventCategory::Warning, u, v);
        m_statusLabel->setText(QString("Error failing link R%1-R%2: %3").arg(u).arg(v).arg(message));
    }
}

void MainWindow::onLinkRecoveredResult(int u, int v, bool success, const QString &message) {
    if (success) {
        recordEvent(QString("Link R%1-R%2 restored (Operational in Adjacency Matrix)").arg(u).arg(v),
                    EventCategory::Recovery, u, v);
        m_statusLabel->setText(QString("Link Restored: Link R%1-R%2 is UP").arg(u).arg(v));
    } else {
        recordEvent(QString("Failed to recover link R%1-R%2: %3").arg(u).arg(v).arg(message),
                    EventCategory::Warning, u, v);
    }
}

void MainWindow::onUndoResult(bool success, const QString &message) {
    m_failureView->setLastUndoResult(success, message);
    if (success) {
        recordEvent("Popped and reverted last failure from Backend Undo Stack", EventCategory::Undo);
        m_statusLabel->setText("Undo executed: Reverted prior failure state from C stack");
    } else {
        recordEvent(QString("Undo failed: %1").arg(message), EventCategory::Warning);
        m_statusLabel->setText(QString("Undo Notice: %1").arg(message));
    }
}

void MainWindow::onRouterAddedResult(bool success, int routerId, int count, const QString &message) {
    Q_UNUSED(count);
    m_editorView->setFeedbackMessage(success, message);
    if (success) {
        recordEvent(QString("Router R%1 provisioned into C Adjacency Matrix").arg(routerId),
                    EventCategory::Info, -1, -1, routerId);
        m_statusLabel->setText(QString("Router R%1 provisioned successfully").arg(routerId));
    } else {
        recordEvent(QString("Failed to provision router: %1").arg(message), EventCategory::Warning);
        m_statusLabel->setText(QString("Router provisioning failed: %1").arg(message));
    }
}

void MainWindow::onEdgeSavedResult(bool success, int u, int v, int weight, const QString &message) {
    m_editorView->setFeedbackMessage(success, message);
    if (success) {
        recordEvent(QString("Edge Link R%1-R%2 set in matrix (weight: %3)").arg(u).arg(v).arg(weight),
                    EventCategory::Info, u, v);
        m_statusLabel->setText(QString("Edge R%1-R%2 saved with weight %3").arg(u).arg(v).arg(weight));
    } else {
        recordEvent(QString("Failed to save edge R%1-R%2: %3").arg(u).arg(v).arg(message), EventCategory::Warning, u, v);
        m_statusLabel->setText(QString("Edge save error: %1").arg(message));
    }
}

void MainWindow::onEdgeDeletedResult(bool success, int u, int v, const QString &message) {
    m_editorView->setFeedbackMessage(success, message);
    if (success) {
        recordEvent(QString("Edge Link R%1-R%2 deleted from matrix and connections.txt").arg(u).arg(v),
                    EventCategory::Info, u, v);
        m_statusLabel->setText(QString("Edge R%1-R%2 deleted").arg(u).arg(v));
    } else {
        recordEvent(QString("Failed to delete edge R%1-R%2: %3").arg(u).arg(v).arg(message), EventCategory::Warning, u, v);
        m_statusLabel->setText(QString("Edge delete error: %1").arg(message));
    }
}

void MainWindow::onBackendStatusChanged(bool connected, const QString &message) {
    m_topBar->setBackendStatus(connected, m_apiClient->baseUrl().section("//", 1));
    m_statusLabel->setText(QString("Backend %1: %2").arg(connected ? "Online" : "Disconnected", message));
}

void MainWindow::onRequestStatus(bool active, const QString &description) {
    if (active) {
        m_statusLabel->setText(description);
    } else {
        m_statusLabel->setText("Ready | System Synced");
    }
}

void MainWindow::recordEvent(const QString &description, EventCategory category, int u, int v, int router) {
    NetworkEvent ev;
    ev.id = m_eventCounter++;
    ev.timestamp = QDateTime::currentDateTime();
    ev.description = description;
    ev.category = category;
    ev.u = u;
    ev.v = v;
    ev.routerId = router;

    m_dashboardView->addEvent(ev);
    m_historyView->addEvent(ev);
    m_analyticsView->updateEventMetrics({ev});
}
