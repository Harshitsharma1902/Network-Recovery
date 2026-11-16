#include "FailureSimulationView.h"
#include "api/ApiClient.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QScrollArea>
#include <QTimer>
#include <QShowEvent>

FailureSimulationView::FailureSimulationView(ApiClient *apiClient, QWidget *parent)
    : QWidget(parent)
    , m_apiClient(apiClient)
    , m_routerCount(0)
{
    setupUi();
}

void FailureSimulationView::setupUi() {
    auto *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(16);

    // Left Console Panel (with ScrollArea for responsiveness)
    auto *leftScroll = new QScrollArea(this);
    leftScroll->setFixedWidth(380);
    leftScroll->setWidgetResizable(true);
    leftScroll->setFrameShape(QFrame::NoFrame);
    leftScroll->setStyleSheet("background-color: transparent;");

    auto *leftWidget = new QWidget(leftScroll);
    auto *leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->setContentsMargins(0, 0, 8, 0);
    leftLayout->setSpacing(16);

    // Title
    auto *headerLabel = new QLabel("Failure & Chaos Simulator", leftWidget);
    headerLabel->setStyleSheet("font-size: 18px; font-weight: 800; color: #f8fafc;");
    auto *subLabel = new QLabel("Trigger network topology perturbations and test failover behavior", leftWidget);
    subLabel->setStyleSheet("font-size: 11px; color: #64748b;");
    leftLayout->addWidget(headerLabel);
    leftLayout->addWidget(subLabel);

    // 1. Link Failure Group
    auto *failLinkGroup = new QGroupBox("LINK FAILURE INJECTION", leftWidget);
    auto *flLayout = new QVBoxLayout(failLinkGroup);
    flLayout->setSpacing(10);

    auto *flDesc = new QLabel("Select an operational link to simulate a fiber cut or interface failure:", failLinkGroup);
    flDesc->setWordWrap(true);
    flDesc->setStyleSheet("font-size: 12px; color: #94a3b8;");
    flLayout->addWidget(flDesc);

    m_activeLinksCombo = new QComboBox(failLinkGroup);
    flLayout->addWidget(m_activeLinksCombo);

    m_failLinkBtn = new QPushButton("⚡ Sever Link (POST /api/link/fail)", failLinkGroup);
    m_failLinkBtn->setObjectName("dangerBtn");
    m_failLinkBtn->setCursor(Qt::PointingHandCursor);
    connect(m_failLinkBtn, &QPushButton::clicked, this, &FailureSimulationView::executeLinkFailure);
    flLayout->addWidget(m_failLinkBtn);

    leftLayout->addWidget(failLinkGroup);

    // 2. Link Recovery Group
    auto *recoverLinkGroup = new QGroupBox("LINK RESTORATION", leftWidget);
    auto *rlLayout = new QVBoxLayout(recoverLinkGroup);
    rlLayout->setSpacing(10);

    auto *rlDesc = new QLabel("Restore a severed link back to operational state:", recoverLinkGroup);
    rlDesc->setWordWrap(true);
    rlDesc->setStyleSheet("font-size: 12px; color: #94a3b8;");
    rlLayout->addWidget(rlDesc);

    m_failedLinksCombo = new QComboBox(recoverLinkGroup);
    rlLayout->addWidget(m_failedLinksCombo);

    m_recoverLinkBtn = new QPushButton("✓ Restore Link (POST /api/link/recover)", recoverLinkGroup);
    m_recoverLinkBtn->setObjectName("successBtn");
    m_recoverLinkBtn->setCursor(Qt::PointingHandCursor);
    connect(m_recoverLinkBtn, &QPushButton::clicked, this, &FailureSimulationView::executeLinkRecovery);
    rlLayout->addWidget(m_recoverLinkBtn);

    leftLayout->addWidget(recoverLinkGroup);

    // 3. Router Isolation Group
    auto *failRouterGroup = new QGroupBox("ROUTER FAILURE SIMULATION", leftWidget);
    auto *frLayout = new QVBoxLayout(failRouterGroup);
    frLayout->setSpacing(10);

    auto *frDesc = new QLabel("Simulate hardware power-off or kernel panic isolating a router node:", failRouterGroup);
    frDesc->setWordWrap(true);
    frDesc->setStyleSheet("font-size: 12px; color: #94a3b8;");
    frLayout->addWidget(frDesc);

    m_routersCombo = new QComboBox(failRouterGroup);
    frLayout->addWidget(m_routersCombo);

    m_failRouterBtn = new QPushButton("⚡ Fail Router (Isolate from Mesh)", failRouterGroup);
    m_failRouterBtn->setObjectName("dangerBtn");
    m_failRouterBtn->setCursor(Qt::PointingHandCursor);
    connect(m_failRouterBtn, &QPushButton::clicked, this, &FailureSimulationView::executeRouterFailure);
    frLayout->addWidget(m_failRouterBtn);

    leftLayout->addWidget(failRouterGroup);

    // 4. Undo Stack Operation Group
    auto *undoGroup = new QGroupBox("BACKEND UNDO STACK (DSA)", leftWidget);
    auto *uLayout = new QVBoxLayout(undoGroup);
    uLayout->setSpacing(10);

    auto *uDesc = new QLabel("Pops the most recent failure from the backend C LIFO stack and restores the prior graph state.", undoGroup);
    uDesc->setWordWrap(true);
    uDesc->setStyleSheet("font-size: 12px; color: #94a3b8;");
    uLayout->addWidget(uDesc);

    m_undoBtn = new QPushButton("↶ Pop & Undo Last Failure (POST /api/undo)", undoGroup);
    m_undoBtn->setCursor(Qt::PointingHandCursor);
    m_undoBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #312e81; color: #c7d2fe; border: 1px solid #4338ca;"
        "  font-size: 12px; font-weight: 700; padding: 8px 14px; border-radius: 6px;"
        "}"
        "QPushButton:hover { background-color: #3730a3; }"
    );
    connect(m_undoBtn, &QPushButton::clicked, this, &FailureSimulationView::executeUndo);
    uLayout->addWidget(m_undoBtn);

    m_undoStatusLabel = new QLabel("Ready for undo operations", undoGroup);
    m_undoStatusLabel->setWordWrap(true);
    m_undoStatusLabel->setStyleSheet("font-size: 11px; color: #64748b; font-style: italic;");
    uLayout->addWidget(m_undoStatusLabel);

    leftLayout->addWidget(undoGroup);
    leftLayout->addStretch();

    leftScroll->setWidget(leftWidget);
    mainLayout->addWidget(leftScroll);

    // Right Canvas Panel
    auto *rightCard = new QFrame(this);
    rightCard->setStyleSheet("background-color: #080c14; border: 1px solid #1e293b; border-radius: 8px;");
    auto *rightLayout = new QVBoxLayout(rightCard);
    rightLayout->setContentsMargins(0, 0, 0, 0);

    auto *canvasHeader = new QFrame(rightCard);
    canvasHeader->setStyleSheet("background-color: #0f172a; border-bottom: 1px solid #1e293b; padding: 6px 12px;");
    auto *chLayout = new QHBoxLayout(canvasHeader);
    chLayout->setContentsMargins(12, 6, 12, 6);

    auto *chTitle = new QLabel("LIVE PERTURBATION VISUALIZATION", canvasHeader);
    chTitle->setStyleSheet("font-size: 12px; font-weight: 800; color: #38bdf8;");
    chLayout->addWidget(chTitle);
    chLayout->addStretch();

    auto *autoLayoutBtn = new QPushButton("⟳ Circular Layout", canvasHeader);
    autoLayoutBtn->setCursor(Qt::PointingHandCursor);
    connect(autoLayoutBtn, &QPushButton::clicked, this, [this]() { m_canvas->autoLayoutCircular(); });
    chLayout->addWidget(autoLayoutBtn);

    auto *zoomInBtn = new QPushButton("+ Zoom In", canvasHeader);
    zoomInBtn->setCursor(Qt::PointingHandCursor);
    connect(zoomInBtn, &QPushButton::clicked, this, [this]() { m_canvas->zoomIn(); });
    chLayout->addWidget(zoomInBtn);

    auto *zoomOutBtn = new QPushButton("- Zoom Out", canvasHeader);
    zoomOutBtn->setCursor(Qt::PointingHandCursor);
    connect(zoomOutBtn, &QPushButton::clicked, this, [this]() { m_canvas->zoomOut(); });
    chLayout->addWidget(zoomOutBtn);

    auto *fitBtn = new QPushButton("⤢ Fit to Screen", canvasHeader);
    fitBtn->setCursor(Qt::PointingHandCursor);
    connect(fitBtn, &QPushButton::clicked, this, [this]() { m_canvas->resetZoom(); });
    chLayout->addWidget(fitBtn);

    m_canvasStateLabel = new QLabel("Canvas Synced", canvasHeader);
    m_canvasStateLabel->setStyleSheet("font-size: 11px; color: #10b981; font-weight: 600; padding-left: 6px;");
    chLayout->addWidget(m_canvasStateLabel);

    rightLayout->addWidget(canvasHeader);

    m_canvas = new NetworkCanvas(rightCard);
    rightLayout->addWidget(m_canvas);

    mainLayout->addWidget(rightCard, 1);
}

void FailureSimulationView::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);
    if (m_canvas) {
        QTimer::singleShot(40, m_canvas, &NetworkCanvas::resetZoom);
    }
}

void FailureSimulationView::updateState(int routerCount, const QList<Connection> &connections) {
    m_routerCount = routerCount;
    m_connections = connections;
    m_canvas->setNetworkData(routerCount, connections);
    QTimer::singleShot(30, m_canvas, &NetworkCanvas::resetZoom);
    updateDropdowns();
}

void FailureSimulationView::updateDropdowns() {
    // Preserve selection if possible
    int prevActiveIdx = m_activeLinksCombo->currentIndex();
    int prevFailedIdx = m_failedLinksCombo->currentIndex();
    int prevRouterIdx = m_routersCombo->currentIndex();

    m_activeLinksCombo->clear();
    m_failedLinksCombo->clear();
    m_routersCombo->clear();

    for (const auto &conn : m_connections) {
        QString text = QString("Link R%1 <-> R%2  (weight: %3)").arg(conn.u).arg(conn.v).arg(conn.weight);
        QVariant data = QVariant::fromValue(QPoint(conn.u, conn.v));

        if (conn.failed) {
            m_failedLinksCombo->addItem("❌ " + text + " [SEVERED]", data);
        } else {
            m_activeLinksCombo->addItem("✔ " + text + " [UP]", data);
        }
    }

    for (int i = 0; i < m_routerCount; ++i) {
        int activeLinks = 0;
        for (const auto &conn : m_connections) {
            if ((conn.u == i || conn.v == i) && !conn.failed) {
                activeLinks++;
            }
        }
        m_routersCombo->addItem(QString("Router R%1 (%2 active incident links)").arg(i).arg(activeLinks), i);
    }

    m_failLinkBtn->setEnabled(m_activeLinksCombo->count() > 0);
    m_recoverLinkBtn->setEnabled(m_failedLinksCombo->count() > 0);
    m_failRouterBtn->setEnabled(m_routersCombo->count() > 0);

    if (prevActiveIdx >= 0 && prevActiveIdx < m_activeLinksCombo->count()) {
        m_activeLinksCombo->setCurrentIndex(prevActiveIdx);
    }
    if (prevFailedIdx >= 0 && prevFailedIdx < m_failedLinksCombo->count()) {
        m_failedLinksCombo->setCurrentIndex(prevFailedIdx);
    }
    if (prevRouterIdx >= 0 && prevRouterIdx < m_routersCombo->count()) {
        m_routersCombo->setCurrentIndex(prevRouterIdx);
    }
}

void FailureSimulationView::executeLinkFailure() {
    if (m_activeLinksCombo->currentIndex() < 0) return;

    QPoint pt = m_activeLinksCombo->currentData().toPoint();
    int u = pt.x();
    int v = pt.y();

    auto reply = QMessageBox::question(
        this, "Confirm Link Severance",
        QString("Are you sure you want to sever the link between Router R%1 and Router R%2?\n\n"
                "This will mark the edge as failed in the C Adjacency Matrix and push the failure to the backend undo stack.")
            .arg(u).arg(v),
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        m_apiClient->failLink(u, v);
    }
}

void FailureSimulationView::executeLinkRecovery() {
    if (m_failedLinksCombo->currentIndex() < 0) return;

    QPoint pt = m_failedLinksCombo->currentData().toPoint();
    int u = pt.x();
    int v = pt.y();

    m_apiClient->recoverLink(u, v);
}

void FailureSimulationView::executeRouterFailure() {
    if (m_routersCombo->currentIndex() < 0) return;

    int routerId = m_routersCombo->currentData().toInt();

    auto reply = QMessageBox::question(
        this, "Confirm Router Failure",
        QString("Are you sure you want to simulate failure for Router R%1?\n\n"
                "This will sever all active incident connections connected to R%1, isolating it from the network graph.")
            .arg(routerId),
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        for (const auto &conn : m_connections) {
            if ((conn.u == routerId || conn.v == routerId) && !conn.failed) {
                m_apiClient->failLink(conn.u, conn.v);
            }
        }
    }
}

void FailureSimulationView::executeUndo() {
    m_undoStatusLabel->setText("Sending undo request to backend...");
    m_apiClient->undoLastFailure();
}

void FailureSimulationView::setLastUndoResult(bool success, const QString &message) {
    if (success) {
        m_undoStatusLabel->setText("✓ " + message);
        m_undoStatusLabel->setStyleSheet("font-size: 11px; color: #34d399; font-weight: 600;");
    } else {
        m_undoStatusLabel->setText("✕ " + message);
        m_undoStatusLabel->setStyleSheet("font-size: 11px; color: #f87171; font-weight: 600;");
    }
}
