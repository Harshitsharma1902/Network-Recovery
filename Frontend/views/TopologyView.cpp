#include "TopologyView.h"
#include "api/ApiClient.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QTimer>
#include <QShowEvent>

TopologyView::TopologyView(ApiClient *apiClient, QWidget *parent)
    : QWidget(parent)
    , m_apiClient(apiClient)
    , m_routerCount(0)
    , m_currentSelectedRouter(-1)
    , m_currentSelectedU(-1)
    , m_currentSelectedV(-1)
{
    setupUi();
}

void TopologyView::setupUi() {
    auto *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(14);

    // Canvas container with overlay toolbar and legend
    auto *canvasContainer = new QFrame(this);
    canvasContainer->setStyleSheet("background-color: #080c14; border: 1px solid #1e293b; border-radius: 8px;");
    auto *canvasLayout = new QVBoxLayout(canvasContainer);
    canvasLayout->setContentsMargins(0, 0, 0, 0);

    // Canvas Toolbar
    auto *toolbar = new QFrame(canvasContainer);
    toolbar->setStyleSheet("background-color: #0f172a; border-bottom: 1px solid #1e293b; padding: 6px 12px;");
    auto *tbLayout = new QHBoxLayout(toolbar);
    tbLayout->setContentsMargins(12, 6, 12, 6);
    tbLayout->setSpacing(10);

    auto *tbTitle = new QLabel("INTERACTIVE TOPOLOGY CANVAS", toolbar);
    tbTitle->setStyleSheet("font-size: 12px; font-weight: 800; color: #38bdf8; letter-spacing: 0.8px;");
    tbLayout->addWidget(tbTitle);

    tbLayout->addStretch();

    auto *autoLayoutBtn = new QPushButton("⟳ Circular Layout", toolbar);
    autoLayoutBtn->setCursor(Qt::PointingHandCursor);
    connect(autoLayoutBtn, &QPushButton::clicked, this, [this]() { m_canvas->autoLayoutCircular(); });
    tbLayout->addWidget(autoLayoutBtn);

    auto *zoomInBtn = new QPushButton("+ Zoom In", toolbar);
    zoomInBtn->setCursor(Qt::PointingHandCursor);
    connect(zoomInBtn, &QPushButton::clicked, this, [this]() { m_canvas->zoomIn(); });
    tbLayout->addWidget(zoomInBtn);

    auto *zoomOutBtn = new QPushButton("- Zoom Out", toolbar);
    zoomOutBtn->setCursor(Qt::PointingHandCursor);
    connect(zoomOutBtn, &QPushButton::clicked, this, [this]() { m_canvas->zoomOut(); });
    tbLayout->addWidget(zoomOutBtn);

    auto *resetZoomBtn = new QPushButton("⤢ Fit View", toolbar);
    resetZoomBtn->setCursor(Qt::PointingHandCursor);
    connect(resetZoomBtn, &QPushButton::clicked, this, [this]() { m_canvas->resetZoom(); });
    tbLayout->addWidget(resetZoomBtn);

    canvasLayout->addWidget(toolbar);

    // The Graphics View Canvas
    m_canvas = new NetworkCanvas(canvasContainer);
    connect(m_canvas, &NetworkCanvas::routerSelected, this, &TopologyView::onRouterSelected);
    connect(m_canvas, &NetworkCanvas::connectionSelected, this, &TopologyView::onConnectionSelected);
    connect(m_canvas, &NetworkCanvas::backgroundClicked, this, &TopologyView::onBackgroundClicked);
    canvasLayout->addWidget(m_canvas);

    // Legend bar at bottom
    auto *legendBar = new QFrame(canvasContainer);
    legendBar->setStyleSheet("background-color: #0b0f19; border-top: 1px solid #1e293b; padding: 6px 14px;");
    auto *legLayout = new QHBoxLayout(legendBar);
    legLayout->setContentsMargins(12, 4, 12, 4);
    legLayout->setSpacing(20);

    auto makeLegendItem = [legendBar](const QString &symbol, const QString &text, const QString &color) {
        auto *lbl = new QLabel(QString("<span style='color:%1; font-weight:bold;'>%2</span> %3").arg(color, symbol, text), legendBar);
        lbl->setStyleSheet("font-size: 11px; color: #94a3b8;");
        return lbl;
    };

    legLayout->addWidget(makeLegendItem("●", "Healthy Router", "#10b981"));
    legLayout->addWidget(makeLegendItem("●", "Warning / Degraded", "#f59e0b"));
    legLayout->addWidget(makeLegendItem("●", "Failed / Isolated", "#ef4444"));
    legLayout->addWidget(makeLegendItem("━", "Operational Link", "#0ea5e9"));
    legLayout->addWidget(makeLegendItem("╌", "Severed / Failed Link", "#ef4444"));
    legLayout->addWidget(makeLegendItem("━", "Shortest Recovery Path", "#a855f7"));
    legLayout->addStretch();

    canvasLayout->addWidget(legendBar);
    mainLayout->addWidget(canvasContainer, 3);

    // Side Details Drawer
    m_detailsDrawer = new QFrame(this);
    m_detailsDrawer->setFixedWidth(300);
    m_detailsDrawer->setStyleSheet("background-color: #0f172a; border: 1px solid #1e293b; border-radius: 8px;");
    auto *drawerLayout = new QVBoxLayout(m_detailsDrawer);
    drawerLayout->setContentsMargins(16, 16, 16, 16);
    drawerLayout->setSpacing(12);

    auto *drawerHeader = new QHBoxLayout();
    m_detailsTitle = new QLabel("ELEMENT DETAILS", m_detailsDrawer);
    m_detailsTitle->setStyleSheet("font-size: 14px; font-weight: 800; color: #f8fafc;");
    drawerHeader->addWidget(m_detailsTitle);
    drawerHeader->addStretch();

    auto *closeBtn = new QPushButton("✕", m_detailsDrawer);
    closeBtn->setFixedSize(24, 24);
    closeBtn->setStyleSheet("background: transparent; border: none; color: #64748b; font-weight: bold;");
    connect(closeBtn, &QPushButton::clicked, this, &TopologyView::hideDetailsDrawer);
    drawerHeader->addWidget(closeBtn);
    drawerLayout->addLayout(drawerHeader);

    m_detailsStatusBadge = new QLabel("STATUS", m_detailsDrawer);
    m_detailsStatusBadge->setStyleSheet("background-color: #064e3b; color: #34d399; font-size: 11px; font-weight: 700; padding: 4px 8px; border-radius: 4px;");
    drawerLayout->addWidget(m_detailsStatusBadge);

    m_detailsSubtitle = new QLabel("Click any router node or link on the canvas to inspect its parameters.", m_detailsDrawer);
    m_detailsSubtitle->setWordWrap(true);
    m_detailsSubtitle->setStyleSheet("font-size: 12px; color: #94a3b8;");
    drawerLayout->addWidget(m_detailsSubtitle);

    auto *tableLabel = new QLabel("INCIDENT CONNECTIONS", m_detailsDrawer);
    tableLabel->setStyleSheet("font-size: 10px; font-weight: 800; color: #64748b; letter-spacing: 0.5px; margin-top: 6px;");
    drawerLayout->addWidget(tableLabel);

    m_neighborsTable = new QTableWidget(0, 3, m_detailsDrawer);
    m_neighborsTable->setHorizontalHeaderLabels({"Target", "Cost", "State"});
    m_neighborsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_neighborsTable->verticalHeader()->setVisible(false);
    m_neighborsTable->setSelectionMode(QAbstractItemView::NoSelection);
    m_neighborsTable->setStyleSheet("background-color: #111827; border: 1px solid #1f2937;");
    drawerLayout->addWidget(m_neighborsTable);

    m_actionBtn1 = new QPushButton("Action 1", m_detailsDrawer);
    m_actionBtn1->setCursor(Qt::PointingHandCursor);
    drawerLayout->addWidget(m_actionBtn1);

    m_actionBtn2 = new QPushButton("Action 2", m_detailsDrawer);
    m_actionBtn2->setCursor(Qt::PointingHandCursor);
    drawerLayout->addWidget(m_actionBtn2);

    drawerLayout->addStretch();
    mainLayout->addWidget(m_detailsDrawer, 1);

    m_detailsDrawer->hide();
}

void TopologyView::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);
    if (m_canvas) {
        QTimer::singleShot(40, m_canvas, &NetworkCanvas::resetZoom);
    }
}

void TopologyView::updateState(int routerCount, const QList<Connection> &connections) {
    m_routerCount = routerCount;
    m_connections = connections;
    m_canvas->setNetworkData(routerCount, connections);

    if (m_currentSelectedRouter >= 0) {
        showRouterDetails(m_currentSelectedRouter);
    } else if (m_currentSelectedU >= 0 && m_currentSelectedV >= 0) {
        showConnectionDetails(m_currentSelectedU, m_currentSelectedV);
    }
}

void TopologyView::selectRouter(int routerId) {
    m_canvas->selectRouter(routerId);
    showRouterDetails(routerId);
}

void TopologyView::highlightRoute(const QList<int> &route) {
    m_canvas->highlightRoute(route);
}

void TopologyView::clearRoute() {
    m_canvas->clearRouteHighlight();
}

void TopologyView::onRouterSelected(int routerId) {
    showRouterDetails(routerId);
}

void TopologyView::onConnectionSelected(int u, int v) {
    showConnectionDetails(u, v);
}

void TopologyView::onBackgroundClicked() {
    hideDetailsDrawer();
}

void TopologyView::hideDetailsDrawer() {
    m_currentSelectedRouter = -1;
    m_currentSelectedU = -1;
    m_currentSelectedV = -1;
    m_detailsDrawer->hide();
}

void TopologyView::showRouterDetails(int routerId) {
    m_currentSelectedRouter = routerId;
    m_currentSelectedU = -1;
    m_currentSelectedV = -1;
    m_detailsDrawer->show();

    m_detailsTitle->setText(QString("Router R%1").arg(routerId));
    m_detailsSubtitle->setText(QString("Management IP: 192.168.1.%1\nArchitecture: Adjacency Matrix Node index %2").arg(10 + routerId).arg(routerId));

    // Populate incident connections
    m_neighborsTable->setRowCount(0);
    int totalLinks = 0;
    int failedLinks = 0;

    for (const auto &conn : m_connections) {
        if (conn.u == routerId || conn.v == routerId) {
            int neighbor = (conn.u == routerId) ? conn.v : conn.u;
            totalLinks++;
            if (conn.failed) failedLinks++;

            int row = m_neighborsTable->rowCount();
            m_neighborsTable->insertRow(row);
            m_neighborsTable->setItem(row, 0, new QTableWidgetItem(QString("R%1").arg(neighbor)));
            m_neighborsTable->setItem(row, 1, new QTableWidgetItem(QString::number(conn.weight)));
            auto *statusItem = new QTableWidgetItem(conn.failed ? "DOWN" : "UP");
            statusItem->setForeground(conn.failed ? QColor("#ef4444") : QColor("#10b981"));
            m_neighborsTable->setItem(row, 2, statusItem);
        }
    }

    if (totalLinks > 0 && failedLinks == totalLinks) {
        m_detailsStatusBadge->setText("STATUS: ISOLATED / FAILED");
        m_detailsStatusBadge->setStyleSheet("background-color: #450a0a; color: #f87171; font-weight: 700; padding: 4px 8px; border-radius: 4px;");
    } else if (failedLinks > 0) {
        m_detailsStatusBadge->setText(QString("STATUS: DEGRADED (%1/%2 DOWN)").arg(failedLinks).arg(totalLinks));
        m_detailsStatusBadge->setStyleSheet("background-color: #451a03; color: #fbbf24; font-weight: 700; padding: 4px 8px; border-radius: 4px;");
    } else {
        m_detailsStatusBadge->setText("STATUS: HEALTHY & ACTIVE");
        m_detailsStatusBadge->setStyleSheet("background-color: #064e3b; color: #34d399; font-weight: 700; padding: 4px 8px; border-radius: 4px;");
    }

    // Action button 1: Isolate / Restore all links
    disconnect(m_actionBtn1, nullptr, nullptr, nullptr);
    if (failedLinks < totalLinks) {
        m_actionBtn1->setText("⚡ Isolate Router (Sever Links)");
        m_actionBtn1->setObjectName("dangerBtn");
        connect(m_actionBtn1, &QPushButton::clicked, this, [this, routerId]() {
            for (const auto &conn : m_connections) {
                if ((conn.u == routerId || conn.v == routerId) && !conn.failed) {
                    m_apiClient->failLink(conn.u, conn.v);
                }
            }
        });
    } else {
        m_actionBtn1->setText("✓ Restore Incident Links");
        m_actionBtn1->setObjectName("successBtn");
        connect(m_actionBtn1, &QPushButton::clicked, this, [this, routerId]() {
            for (const auto &conn : m_connections) {
                if ((conn.u == routerId || conn.v == routerId) && conn.failed) {
                    m_apiClient->recoverLink(conn.u, conn.v);
                }
            }
        });
    }

    // Action button 2: Find route
    disconnect(m_actionBtn2, nullptr, nullptr, nullptr);
    m_actionBtn2->setText("🎯 Calculate Recovery Path");
    m_actionBtn2->setObjectName("primaryBtn");
    connect(m_actionBtn2, &QPushButton::clicked, this, [this, routerId]() {
        int target = (routerId + 1) % m_routerCount;
        emit requestRouteAnalysis(routerId, target);
    });
}

void TopologyView::showConnectionDetails(int u, int v) {
    m_currentSelectedRouter = -1;
    m_currentSelectedU = u;
    m_currentSelectedV = v;
    m_detailsDrawer->show();

    // Find connection
    const Connection *found = nullptr;
    for (const auto &conn : m_connections) {
        if ((conn.u == u && conn.v == v) || (conn.u == v && conn.v == u)) {
            found = &conn;
            break;
        }
    }

    if (!found) return;

    m_detailsTitle->setText(QString("Link R%1 <-> R%2").arg(u).arg(v));
    m_detailsSubtitle->setText(QString("Metric Weight: %1\nTopology Layer: Adjacency Edge [%2][%3]").arg(found->weight).arg(u).arg(v));

    m_neighborsTable->setRowCount(0);
    m_neighborsTable->insertRow(0);
    m_neighborsTable->setItem(0, 0, new QTableWidgetItem(QString("R%1").arg(u)));
    m_neighborsTable->setItem(0, 1, new QTableWidgetItem(QString::number(found->weight)));
    m_neighborsTable->setItem(0, 2, new QTableWidgetItem(found->failed ? "FAIL" : "OK"));

    m_neighborsTable->insertRow(1);
    m_neighborsTable->setItem(1, 0, new QTableWidgetItem(QString("R%1").arg(v)));
    m_neighborsTable->setItem(1, 1, new QTableWidgetItem(QString::number(found->weight)));
    m_neighborsTable->setItem(1, 2, new QTableWidgetItem(found->failed ? "FAIL" : "OK"));

    if (found->failed) {
        m_detailsStatusBadge->setText("STATE: LINK SEVERED (DOWN)");
        m_detailsStatusBadge->setStyleSheet("background-color: #450a0a; color: #f87171; font-weight: 700; padding: 4px 8px; border-radius: 4px;");

        disconnect(m_actionBtn1, nullptr, nullptr, nullptr);
        m_actionBtn1->setText("✓ Restore Operational Link");
        m_actionBtn1->setObjectName("successBtn");
        connect(m_actionBtn1, &QPushButton::clicked, this, [this, u, v]() {
            m_apiClient->recoverLink(u, v);
        });
    } else {
        m_detailsStatusBadge->setText("STATE: OPERATIONAL (UP)");
        m_detailsStatusBadge->setStyleSheet("background-color: #064e3b; color: #34d399; font-weight: 700; padding: 4px 8px; border-radius: 4px;");

        disconnect(m_actionBtn1, nullptr, nullptr, nullptr);
        m_actionBtn1->setText("⚡ Sever Link (Fail Link)");
        m_actionBtn1->setObjectName("dangerBtn");
        connect(m_actionBtn1, &QPushButton::clicked, this, [this, u, v]() {
            m_apiClient->failLink(u, v);
        });
    }

    disconnect(m_actionBtn2, nullptr, nullptr, nullptr);
    m_actionBtn2->setText("🎯 Shortest Route R%1 -> R%2");
    m_actionBtn2->setObjectName("primaryBtn");
    connect(m_actionBtn2, &QPushButton::clicked, this, [this, u, v]() {
        emit requestRouteAnalysis(u, v);
    });
}
