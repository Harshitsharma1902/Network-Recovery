#include "RecoveryView.h"
#include "api/ApiClient.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QScrollArea>
#include <QTimer>
#include <QShowEvent>

RecoveryView::RecoveryView(ApiClient *apiClient, QWidget *parent)
    : QWidget(parent)
    , m_apiClient(apiClient)
    , m_routerCount(0)
{
    setupUi();
}

void RecoveryView::setupUi() {
    auto *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(16);

    // Left Control & Telemetry Panel
    auto *leftScroll = new QScrollArea(this);
    leftScroll->setFixedWidth(400);
    leftScroll->setWidgetResizable(true);
    leftScroll->setFrameShape(QFrame::NoFrame);
    leftScroll->setStyleSheet("background-color: transparent;");

    auto *leftWidget = new QWidget(leftScroll);
    auto *leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->setContentsMargins(0, 0, 8, 0);
    leftLayout->setSpacing(16);

    // Title
    auto *title = new QLabel("Recovery Path Analysis", leftWidget);
    title->setStyleSheet("font-size: 18px; font-weight: 800; color: #f8fafc;");
    auto *sub = new QLabel("Demonstrating Dijkstra's Shortest Path & BFS Reachability on C Graph", leftWidget);
    sub->setStyleSheet("font-size: 11px; color: #64748b;");
    leftLayout->addWidget(title);
    leftLayout->addWidget(sub);

    // Selection Group
    auto *inputGroup = new QGroupBox("ROUTER SELECTION", leftWidget);
    auto *inLayout = new QVBoxLayout(inputGroup);
    inLayout->setSpacing(10);

    auto *srcRow = new QHBoxLayout();
    auto *srcLabel = new QLabel("Source Router:", inputGroup);
    srcLabel->setStyleSheet("color: #94a3b8; font-weight: 600; font-size: 12px;");
    m_srcCombo = new QComboBox(inputGroup);
    srcRow->addWidget(srcLabel, 1);
    srcRow->addWidget(m_srcCombo, 2);
    inLayout->addLayout(srcRow);

    auto *destRow = new QHBoxLayout();
    auto *destLabel = new QLabel("Destination Router:", inputGroup);
    destLabel->setStyleSheet("color: #94a3b8; font-weight: 600; font-size: 12px;");
    m_destCombo = new QComboBox(inputGroup);
    destRow->addWidget(destLabel, 1);
    destRow->addWidget(m_destCombo, 2);
    inLayout->addLayout(destRow);

    auto *btnRow = new QHBoxLayout();
    m_computeBtn = new QPushButton("🎯 Run Recovery Route", inputGroup);
    m_computeBtn->setObjectName("primaryBtn");
    m_computeBtn->setCursor(Qt::PointingHandCursor);
    connect(m_computeBtn, &QPushButton::clicked, this, &RecoveryView::computeRoute);
    btnRow->addWidget(m_computeBtn, 2);

    m_clearBtn = new QPushButton("Clear", inputGroup);
    m_clearBtn->setCursor(Qt::PointingHandCursor);
    connect(m_clearBtn, &QPushButton::clicked, this, &RecoveryView::clearRoute);
    btnRow->addWidget(m_clearBtn, 1);
    inLayout->addLayout(btnRow);

    leftLayout->addWidget(inputGroup);

    // Telemetry Group
    auto *telemetryGroup = new QGroupBox("GRAPH ALGORITHM RESULTS", leftWidget);
    auto *telLayout = new QVBoxLayout(telemetryGroup);
    telLayout->setSpacing(10);

    // BFS Reachability Status
    auto *bfsHeader = new QLabel("BFS Reachability Check:", telemetryGroup);
    bfsHeader->setStyleSheet("font-size: 11px; font-weight: 700; color: #94a3b8; letter-spacing: 0.5px;");
    telLayout->addWidget(bfsHeader);

    m_bfsStatusBadge = new QLabel("AWAITING COMPUTATION", telemetryGroup);
    m_bfsStatusBadge->setStyleSheet("background-color: #1e293b; color: #94a3b8; font-weight: 700; font-size: 11px; padding: 6px 12px; border-radius: 6px;");
    telLayout->addWidget(m_bfsStatusBadge);

    // Dijkstra Path Cost
    auto *costHeader = new QLabel("Dijkstra Total Metric Cost:", telemetryGroup);
    costHeader->setStyleSheet("font-size: 11px; font-weight: 700; color: #94a3b8; letter-spacing: 0.5px; margin-top: 6px;");
    telLayout->addWidget(costHeader);

    m_dijkstraCostLabel = new QLabel("--", telemetryGroup);
    m_dijkstraCostLabel->setStyleSheet("font-size: 26px; font-weight: 800; color: #a855f7;");
    telLayout->addWidget(m_dijkstraCostLabel);

    // Route String
    auto *routeHeader = new QLabel("Optimal Recovery Route:", telemetryGroup);
    routeHeader->setStyleSheet("font-size: 11px; font-weight: 700; color: #94a3b8; letter-spacing: 0.5px; margin-top: 6px;");
    telLayout->addWidget(routeHeader);

    m_routePathLabel = new QLabel("Select routers and click 'Run Recovery Route'", telemetryGroup);
    m_routePathLabel->setWordWrap(true);
    m_routePathLabel->setStyleSheet("font-size: 13px; font-weight: 700; color: #f8fafc; background-color: #0b0f19; padding: 8px; border-radius: 6px; border: 1px solid #1e293b;");
    telLayout->addWidget(m_routePathLabel);

    // Unreachable Warning Box
    m_unreachableAlert = new QLabel(telemetryGroup);
    m_unreachableAlert->setWordWrap(true);
    m_unreachableAlert->setStyleSheet("background-color: #450a0a; color: #fca5a5; border: 1px solid #b91c1c; border-radius: 6px; padding: 10px; font-size: 12px;");
    m_unreachableAlert->setText("⚠️ <b>No Connected Route Found!</b><br>BFS traversal confirmed that the target is disconnected from the source. Restore severed links to re-establish connectivity.");
    m_unreachableAlert->hide();
    telLayout->addWidget(m_unreachableAlert);

    // Hops Table
    auto *hopsHeader = new QLabel("HOP-BY-HOP BREAKDOWN", telemetryGroup);
    hopsHeader->setStyleSheet("font-size: 10px; font-weight: 800; color: #64748b; letter-spacing: 0.5px; margin-top: 8px;");
    telLayout->addWidget(hopsHeader);

    m_hopsTable = new QTableWidget(0, 4, telemetryGroup);
    m_hopsTable->setHorizontalHeaderLabels({"Hop #", "From", "To", "Cost"});
    m_hopsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_hopsTable->verticalHeader()->setVisible(false);
    m_hopsTable->setMinimumHeight(140);
    telLayout->addWidget(m_hopsTable);

    leftLayout->addWidget(telemetryGroup);
    leftLayout->addStretch();

    leftScroll->setWidget(leftWidget);
    mainLayout->addWidget(leftScroll);

    // Right Canvas Panel
    auto *canvasCard = new QFrame(this);
    canvasCard->setStyleSheet("background-color: #080c14; border: 1px solid #1e293b; border-radius: 8px;");
    auto *canvasCardLayout = new QVBoxLayout(canvasCard);
    canvasCardLayout->setContentsMargins(0, 0, 0, 0);

    auto *cHeader = new QFrame(canvasCard);
    cHeader->setStyleSheet("background-color: #0f172a; border-bottom: 1px solid #1e293b; padding: 6px 12px;");
    auto *chLayout = new QHBoxLayout(cHeader);
    chLayout->setContentsMargins(12, 6, 12, 6);

    auto *chTitle = new QLabel("RECOVERY PATH VISUALIZATION (ILLUMINATED ROUTE)", cHeader);
    chTitle->setStyleSheet("font-size: 12px; font-weight: 800; color: #a855f7;");
    chLayout->addWidget(chTitle);
    chLayout->addStretch();

    auto *autoLayoutBtn = new QPushButton("⟳ Circular Layout", cHeader);
    autoLayoutBtn->setCursor(Qt::PointingHandCursor);
    connect(autoLayoutBtn, &QPushButton::clicked, this, [this]() { m_canvas->autoLayoutCircular(); });
    chLayout->addWidget(autoLayoutBtn);

    auto *zoomInBtn = new QPushButton("+ Zoom In", cHeader);
    zoomInBtn->setCursor(Qt::PointingHandCursor);
    connect(zoomInBtn, &QPushButton::clicked, this, [this]() { m_canvas->zoomIn(); });
    chLayout->addWidget(zoomInBtn);

    auto *zoomOutBtn = new QPushButton("- Zoom Out", cHeader);
    zoomOutBtn->setCursor(Qt::PointingHandCursor);
    connect(zoomOutBtn, &QPushButton::clicked, this, [this]() { m_canvas->zoomOut(); });
    chLayout->addWidget(zoomOutBtn);

    auto *fitBtn = new QPushButton("⤢ Fit to Screen", cHeader);
    fitBtn->setCursor(Qt::PointingHandCursor);
    connect(fitBtn, &QPushButton::clicked, this, [this]() { m_canvas->resetZoom(); });
    chLayout->addWidget(fitBtn);

    auto *legendNote = new QLabel("● Purple: Dijkstra Shortest Route", cHeader);
    legendNote->setStyleSheet("font-size: 11px; color: #c084fc; font-weight: 600; padding-left: 6px;");
    chLayout->addWidget(legendNote);

    canvasCardLayout->addWidget(cHeader);

    m_canvas = new NetworkCanvas(canvasCard);
    canvasCardLayout->addWidget(m_canvas);

    mainLayout->addWidget(canvasCard, 1);
}

void RecoveryView::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);
    if (m_canvas) {
        QTimer::singleShot(40, m_canvas, &NetworkCanvas::resetZoom);
    }
}

void RecoveryView::updateState(int routerCount, const QList<Connection> &connections) {
    m_routerCount = routerCount;
    m_connections = connections;
    m_canvas->setNetworkData(routerCount, connections);
    QTimer::singleShot(30, m_canvas, &NetworkCanvas::resetZoom);
    updateDropdowns();
}

void RecoveryView::updateDropdowns() {
    int prevSrc = m_srcCombo->currentIndex();
    int prevDest = m_destCombo->currentIndex();

    m_srcCombo->clear();
    m_destCombo->clear();

    for (int i = 0; i < m_routerCount; ++i) {
        QString text = QString("Router R%1").arg(i);
        m_srcCombo->addItem(text, i);
        m_destCombo->addItem(text, i);
    }

    if (prevSrc >= 0 && prevSrc < m_srcCombo->count()) {
        m_srcCombo->setCurrentIndex(prevSrc);
    } else if (m_srcCombo->count() > 0) {
        m_srcCombo->setCurrentIndex(0);
    }

    if (prevDest >= 0 && prevDest < m_destCombo->count()) {
        m_destCombo->setCurrentIndex(prevDest);
    } else if (m_destCombo->count() > 1) {
        m_destCombo->setCurrentIndex(m_destCombo->count() - 1);
    }
}

void RecoveryView::setSourceAndDest(int src, int dest) {
    for (int i = 0; i < m_srcCombo->count(); ++i) {
        if (m_srcCombo->itemData(i).toInt() == src) {
            m_srcCombo->setCurrentIndex(i);
            break;
        }
    }
    for (int i = 0; i < m_destCombo->count(); ++i) {
        if (m_destCombo->itemData(i).toInt() == dest) {
            m_destCombo->setCurrentIndex(i);
            break;
        }
    }
    computeRoute();
}

void RecoveryView::computeRoute() {
    if (m_srcCombo->currentIndex() < 0 || m_destCombo->currentIndex() < 0) return;

    int src = m_srcCombo->currentData().toInt();
    int dest = m_destCombo->currentData().toInt();

    m_apiClient->findRoute(src, dest);
}

void RecoveryView::clearRoute() {
    m_canvas->clearRouteHighlight();
    m_bfsStatusBadge->setText("AWAITING COMPUTATION");
    m_bfsStatusBadge->setStyleSheet("background-color: #1e293b; color: #94a3b8; font-weight: 700; font-size: 11px; padding: 6px 12px; border-radius: 6px;");
    m_dijkstraCostLabel->setText("--");
    m_routePathLabel->setText("Select routers and click 'Run Recovery Route'");
    m_hopsTable->setRowCount(0);
    m_unreachableAlert->hide();
}

void RecoveryView::setRouteResult(int src, int dest, bool reachable, int cost, const QList<int> &route) {
    if (!reachable || route.isEmpty()) {
        m_canvas->clearRouteHighlight();
        m_bfsStatusBadge->setText("✕ UNREACHABLE (BFS: Network Partitioned)");
        m_bfsStatusBadge->setStyleSheet("background-color: #450a0a; color: #f87171; font-weight: 700; font-size: 11px; padding: 6px 12px; border-radius: 6px;");
        m_dijkstraCostLabel->setText("∞ (INF)");
        m_dijkstraCostLabel->setStyleSheet("font-size: 26px; font-weight: 800; color: #ef4444;");
        m_routePathLabel->setText(QString("No path exists between R%1 and R%2 due to severed links.").arg(src).arg(dest));
        m_hopsTable->setRowCount(0);
        m_unreachableAlert->show();
        return;
    }

    m_unreachableAlert->hide();
    m_bfsStatusBadge->setText("✓ REACHABLE (BFS Verified Path Exists)");
    m_bfsStatusBadge->setStyleSheet("background-color: #064e3b; color: #34d399; font-weight: 700; font-size: 11px; padding: 6px 12px; border-radius: 6px;");

    m_dijkstraCostLabel->setText(QString("%1 ms").arg(cost));
    m_dijkstraCostLabel->setStyleSheet("font-size: 26px; font-weight: 800; color: #a855f7;");

    // Format path string
    QStringList pathStrings;
    for (int node : route) {
        pathStrings.append(QString("R%1").arg(node));
    }
    m_routePathLabel->setText(pathStrings.join(" ➔ "));

    // Populate hops table
    m_hopsTable->setRowCount(0);
    for (int i = 0; i < route.size() - 1; ++i) {
        int u = route[i];
        int v = route[i + 1];

        int weight = 1;
        for (const auto &conn : m_connections) {
            if ((conn.u == u && conn.v == v) || (conn.u == v && conn.v == u)) {
                weight = conn.weight;
                break;
            }
        }

        int row = m_hopsTable->rowCount();
        m_hopsTable->insertRow(row);
        m_hopsTable->setItem(row, 0, new QTableWidgetItem(QString("Hop %1").arg(i + 1)));
        m_hopsTable->setItem(row, 1, new QTableWidgetItem(QString("R%1").arg(u)));
        m_hopsTable->setItem(row, 2, new QTableWidgetItem(QString("R%1").arg(v)));
        m_hopsTable->setItem(row, 3, new QTableWidgetItem(QString::number(weight)));
    }

    // Highlight on canvas
    m_canvas->highlightRoute(route);
}
