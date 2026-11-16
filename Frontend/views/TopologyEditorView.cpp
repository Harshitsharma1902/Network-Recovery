#include "TopologyEditorView.h"
#include "api/ApiClient.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QScrollArea>
#include <QMessageBox>
#include <QTimer>
#include <QShowEvent>

TopologyEditorView::TopologyEditorView(ApiClient *apiClient, QWidget *parent)
    : QWidget(parent)
    , m_apiClient(apiClient)
    , m_routerCount(0)
{
    setupUi();
}

void TopologyEditorView::setupUi() {
    auto *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(16);

    // Left Configuration Scroll Area
    auto *leftScroll = new QScrollArea(this);
    leftScroll->setFixedWidth(420);
    leftScroll->setWidgetResizable(true);
    leftScroll->setFrameShape(QFrame::NoFrame);
    leftScroll->setStyleSheet("background-color: transparent;");

    auto *leftWidget = new QWidget(leftScroll);
    auto *leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->setContentsMargins(0, 0, 8, 0);
    leftLayout->setSpacing(16);

    // Title & Subtitle
    auto *title = new QLabel("Route & Topology Editor", leftWidget);
    title->setStyleSheet("font-size: 18px; font-weight: 800; color: #f8fafc;");
    auto *sub = new QLabel("Add and configure route endpoints (routers), edges, and metric weights", leftWidget);
    sub->setStyleSheet("font-size: 11px; color: #64748b;");
    leftLayout->addWidget(title);
    leftLayout->addWidget(sub);

    // Feedback Banner
    m_feedbackBanner = new QLabel("Ready to configure topology", leftWidget);
    m_feedbackBanner->setWordWrap(true);
    m_feedbackBanner->setStyleSheet("background-color: #1e293b; color: #94a3b8; padding: 8px 12px; border-radius: 6px; font-size: 11px; font-weight: 600;");
    leftLayout->addWidget(m_feedbackBanner);

    // 1. Router Endpoints Provisioning Card
    auto *routerGroup = new QGroupBox("1. PROVISION ROUTE ENDPOINTS (ROUTERS)", leftWidget);
    auto *rgLayout = new QVBoxLayout(routerGroup);
    rgLayout->setSpacing(10);

    m_nextRouterLabel = new QLabel("Next Endpoint: Router R0", routerGroup);
    m_nextRouterLabel->setStyleSheet("font-size: 12px; font-weight: 700; color: #38bdf8;");
    rgLayout->addWidget(m_nextRouterLabel);

    auto *rDesc = new QLabel("Adds a new node to the backend C Adjacency Matrix and saves to persistence storage.", routerGroup);
    rDesc->setWordWrap(true);
    rDesc->setStyleSheet("font-size: 11px; color: #94a3b8;");
    rgLayout->addWidget(rDesc);

    m_addRouterBtn = new QPushButton("+ Provision New Router Node (POST /api/router/add)", routerGroup);
    m_addRouterBtn->setObjectName("primaryBtn");
    m_addRouterBtn->setCursor(Qt::PointingHandCursor);
    connect(m_addRouterBtn, &QPushButton::clicked, this, &TopologyEditorView::onAddRouterClicked);
    rgLayout->addWidget(m_addRouterBtn);

    leftLayout->addWidget(routerGroup);

    // 2. Edge (Link) & Weight Configuration Card
    auto *edgeGroup = new QGroupBox("2. CONFIGURE ROUTE EDGES & METRIC WEIGHTS", leftWidget);
    auto *egLayout = new QVBoxLayout(edgeGroup);
    egLayout->setSpacing(10);

    // Source Endpoint Row
    auto *srcRow = new QHBoxLayout();
    auto *srcLbl = new QLabel("Source Endpoint (u):", edgeGroup);
    srcLbl->setStyleSheet("color: #94a3b8; font-weight: 600; font-size: 12px;");
    m_srcCombo = new QComboBox(edgeGroup);
    srcRow->addWidget(srcLbl, 1);
    srcRow->addWidget(m_srcCombo, 2);
    egLayout->addLayout(srcRow);

    // Destination Endpoint Row
    auto *destRow = new QHBoxLayout();
    auto *destLbl = new QLabel("Destination Endpoint (v):", edgeGroup);
    destLbl->setStyleSheet("color: #94a3b8; font-weight: 600; font-size: 12px;");
    m_destCombo = new QComboBox(edgeGroup);
    destRow->addWidget(destLbl, 1);
    destRow->addWidget(m_destCombo, 2);
    egLayout->addLayout(destRow);

    connect(m_srcCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TopologyEditorView::onEndpointsChanged);
    connect(m_destCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TopologyEditorView::onEndpointsChanged);

    // Weight Metric Row
    auto *weightRow = new QHBoxLayout();
    auto *wLbl = new QLabel("Metric Weight (Cost / Latency ms):", edgeGroup);
    wLbl->setStyleSheet("color: #94a3b8; font-weight: 600; font-size: 12px;");
    m_weightSpin = new QSpinBox(edgeGroup);
    m_weightSpin->setRange(1, 9999);
    m_weightSpin->setValue(5);
    weightRow->addWidget(wLbl, 2);
    weightRow->addWidget(m_weightSpin, 1);
    egLayout->addLayout(weightRow);

    // Edge Status Indicator
    m_edgeStatusNote = new QLabel("Edge Status: Ready", edgeGroup);
    m_edgeStatusNote->setStyleSheet("font-size: 11px; color: #38bdf8; font-style: italic;");
    egLayout->addWidget(m_edgeStatusNote);

    // Edge Action Buttons
    auto *edgeBtnRow = new QHBoxLayout();
    m_saveEdgeBtn = new QPushButton("💾 Save / Update Edge", edgeGroup);
    m_saveEdgeBtn->setObjectName("successBtn");
    m_saveEdgeBtn->setCursor(Qt::PointingHandCursor);
    connect(m_saveEdgeBtn, &QPushButton::clicked, this, &TopologyEditorView::onSaveEdgeClicked);
    edgeBtnRow->addWidget(m_saveEdgeBtn, 2);

    m_deleteEdgeBtn = new QPushButton("🗑 Delete Edge", edgeGroup);
    m_deleteEdgeBtn->setObjectName("dangerBtn");
    m_deleteEdgeBtn->setCursor(Qt::PointingHandCursor);
    connect(m_deleteEdgeBtn, &QPushButton::clicked, this, &TopologyEditorView::onDeleteEdgeClicked);
    edgeBtnRow->addWidget(m_deleteEdgeBtn, 1);
    egLayout->addLayout(edgeBtnRow);

    leftLayout->addWidget(edgeGroup);

    // 3. Current Edges Table
    auto *tableGroup = new QGroupBox("3. LIVE EDGES INVENTORY", leftWidget);
    auto *tgLayout = new QVBoxLayout(tableGroup);
    tgLayout->setSpacing(8);

    m_edgesTable = new QTableWidget(0, 4, tableGroup);
    m_edgesTable->setHorizontalHeaderLabels({"Edge Link", "Weight", "State", "Select"});
    m_edgesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_edgesTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_edgesTable->verticalHeader()->setVisible(false);
    m_edgesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_edgesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_edgesTable->setMinimumHeight(180);
    connect(m_edgesTable, &QTableWidget::itemSelectionChanged, this, &TopologyEditorView::onEdgeTableSelectionChanged);
    tgLayout->addWidget(m_edgesTable);

    leftLayout->addWidget(tableGroup);
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

    auto *chTitle = new QLabel("LIVE TOPOLOGY SYNCHRONIZATION", cHeader);
    chTitle->setStyleSheet("font-size: 12px; font-weight: 800; color: #38bdf8;");
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

    canvasCardLayout->addWidget(cHeader);

    m_canvas = new NetworkCanvas(canvasCard);
    connect(m_canvas, &NetworkCanvas::connectionSelected, this, [this](int u, int v) {
        for (int i = 0; i < m_srcCombo->count(); ++i) {
            if (m_srcCombo->itemData(i).toInt() == u) m_srcCombo->setCurrentIndex(i);
        }
        for (int i = 0; i < m_destCombo->count(); ++i) {
            if (m_destCombo->itemData(i).toInt() == v) m_destCombo->setCurrentIndex(i);
        }
    });
    canvasCardLayout->addWidget(m_canvas);

    mainLayout->addWidget(canvasCard, 1);
}

void TopologyEditorView::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);
    if (m_canvas) {
        QTimer::singleShot(40, m_canvas, &NetworkCanvas::resetZoom);
    }
}

void TopologyEditorView::updateState(int routerCount, const QList<Connection> &connections) {
    m_routerCount = routerCount;
    m_connections = connections;

    m_nextRouterLabel->setText(QString("Next Endpoint: Router R%1 (Total: %2 nodes)").arg(routerCount).arg(routerCount));
    m_canvas->setNetworkData(routerCount, connections);
    QTimer::singleShot(30, m_canvas, &NetworkCanvas::resetZoom);

    updateDropdowns();
    populateEdgesTable();
    onEndpointsChanged();
}

void TopologyEditorView::updateDropdowns() {
    int prevSrc = m_srcCombo->currentIndex();
    int prevDest = m_destCombo->currentIndex();

    m_srcCombo->blockSignals(true);
    m_destCombo->blockSignals(true);

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
        m_destCombo->setCurrentIndex(1);
    }

    m_srcCombo->blockSignals(false);
    m_destCombo->blockSignals(false);
}

void TopologyEditorView::onEndpointsChanged() {
    if (m_srcCombo->currentIndex() < 0 || m_destCombo->currentIndex() < 0) return;

    int u = m_srcCombo->currentData().toInt();
    int v = m_destCombo->currentData().toInt();

    if (u == v) {
        m_edgeStatusNote->setText("⚠️ Source and Destination must be distinct endpoints.");
        m_edgeStatusNote->setStyleSheet("font-size: 11px; color: #ef4444; font-weight: 600;");
        m_saveEdgeBtn->setEnabled(false);
        m_deleteEdgeBtn->setEnabled(false);
        return;
    }

    m_saveEdgeBtn->setEnabled(true);

    // Check if edge already exists
    const Connection *existing = nullptr;
    for (const auto &conn : m_connections) {
        if ((conn.u == u && conn.v == v) || (conn.u == v && conn.v == u)) {
            existing = &conn;
            break;
        }
    }

    if (existing) {
        m_weightSpin->setValue(existing->weight);
        m_edgeStatusNote->setText(QString("ℹ Existing Link R%1-R%2 (Current metric: %3 ms, %4)")
                                      .arg(u).arg(v).arg(existing->weight)
                                      .arg(existing->failed ? "SEVERED" : "OPERATIONAL"));
        m_edgeStatusNote->setStyleSheet("font-size: 11px; color: #f59e0b; font-weight: 600;");
        m_saveEdgeBtn->setText("💾 Update Edge Metric");
        m_deleteEdgeBtn->setEnabled(true);
    } else {
        m_edgeStatusNote->setText("✨ New Edge (Will be added to Adjacency Matrix & saved to disk)");
        m_edgeStatusNote->setStyleSheet("font-size: 11px; color: #34d399; font-weight: 600;");
        m_saveEdgeBtn->setText("➕ Add New Edge (Link)");
        m_deleteEdgeBtn->setEnabled(false);
    }
}

void TopologyEditorView::populateEdgesTable() {
    m_edgesTable->setRowCount(0);

    for (int i = 0; i < m_connections.size(); ++i) {
        const auto &conn = m_connections[i];
        int row = m_edgesTable->rowCount();
        m_edgesTable->insertRow(row);

        auto *pairItem = new QTableWidgetItem(QString("R%1 <-> R%2").arg(conn.u).arg(conn.v));
        pairItem->setTextAlignment(Qt::AlignCenter);
        pairItem->setFont(QFont("monospace", 10, QFont::Bold));
        m_edgesTable->setItem(row, 0, pairItem);

        auto *wItem = new QTableWidgetItem(QString::number(conn.weight));
        wItem->setTextAlignment(Qt::AlignCenter);
        m_edgesTable->setItem(row, 1, wItem);

        auto *stateItem = new QTableWidgetItem(conn.failed ? "DOWN" : "UP");
        stateItem->setTextAlignment(Qt::AlignCenter);
        stateItem->setForeground(conn.failed ? QColor("#ef4444") : QColor("#10b981"));
        stateItem->setFont(QFont(font().family(), 10, QFont::Bold));
        m_edgesTable->setItem(row, 2, stateItem);

        auto *selectBtn = new QPushButton("Edit", m_edgesTable);
        selectBtn->setCursor(Qt::PointingHandCursor);
        selectBtn->setStyleSheet("padding: 2px 8px; font-size: 11px;");

        int u = conn.u;
        int v = conn.v;
        int w = conn.weight;

        connect(selectBtn, &QPushButton::clicked, this, [this, u, v, w]() {
            for (int j = 0; j < m_srcCombo->count(); ++j) {
                if (m_srcCombo->itemData(j).toInt() == u) m_srcCombo->setCurrentIndex(j);
            }
            for (int j = 0; j < m_destCombo->count(); ++j) {
                if (m_destCombo->itemData(j).toInt() == v) m_destCombo->setCurrentIndex(j);
            }
            m_weightSpin->setValue(w);
            m_canvas->selectConnection(u, v);
        });

        m_edgesTable->setCellWidget(row, 3, selectBtn);
    }
}

void TopologyEditorView::onEdgeTableSelectionChanged() {
    int row = m_edgesTable->currentRow();
    if (row >= 0 && row < m_connections.size()) {
        const auto &conn = m_connections[row];
        for (int j = 0; j < m_srcCombo->count(); ++j) {
            if (m_srcCombo->itemData(j).toInt() == conn.u) m_srcCombo->setCurrentIndex(j);
        }
        for (int j = 0; j < m_destCombo->count(); ++j) {
            if (m_destCombo->itemData(j).toInt() == conn.v) m_destCombo->setCurrentIndex(j);
        }
        m_weightSpin->setValue(conn.weight);
        m_canvas->selectConnection(conn.u, conn.v);
    }
}

void TopologyEditorView::onAddRouterClicked() {
    auto reply = QMessageBox::question(
        this, "Confirm Router Provisioning",
        QString("Are you sure you want to provision a new router node (Router R%1)?\n\n"
                "This will increment the graph vertex count in the backend C matrix and save the updated configuration.")
            .arg(m_routerCount),
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        m_apiClient->addRouter();
    }
}

void TopologyEditorView::onSaveEdgeClicked() {
    if (m_srcCombo->currentIndex() < 0 || m_destCombo->currentIndex() < 0) return;

    int u = m_srcCombo->currentData().toInt();
    int v = m_destCombo->currentData().toInt();
    int weight = m_weightSpin->value();

    if (u == v || weight <= 0) return;

    m_apiClient->saveEdge(u, v, weight);
}

void TopologyEditorView::onDeleteEdgeClicked() {
    if (m_srcCombo->currentIndex() < 0 || m_destCombo->currentIndex() < 0) return;

    int u = m_srcCombo->currentData().toInt();
    int v = m_destCombo->currentData().toInt();

    auto reply = QMessageBox::question(
        this, "Confirm Edge Deletion",
        QString("Are you sure you want to delete the edge link between Router R%1 and Router R%2?\n\n"
                "This will remove the connection from the backend C Adjacency Matrix and update connections.txt.")
            .arg(u).arg(v),
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        m_apiClient->deleteEdge(u, v);
    }
}

void TopologyEditorView::setFeedbackMessage(bool success, const QString &message) {
    m_feedbackBanner->setText(QString("%1 %2").arg(success ? "✓" : "✕", message));
    m_feedbackBanner->setStyleSheet(QString(
        "background-color: %1; color: %2; padding: 8px 12px; border-radius: 6px; font-size: 11px; font-weight: 700; border: 1px solid %3;"
    ).arg(success ? "#064e3b" : "#450a0a", success ? "#34d399" : "#f87171", success ? "#059669" : "#b91c1c"));
}
