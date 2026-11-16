#include "RouterView.h"
#include "api/ApiClient.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QDialog>
#include <QMessageBox>

RouterView::RouterView(ApiClient *apiClient, QWidget *parent)
    : QWidget(parent)
    , m_apiClient(apiClient)
    , m_routerCount(0)
{
    setupUi();
}

void RouterView::setupUi() {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 20, 24, 24);
    layout->setSpacing(16);

    // Header & Actions
    auto *headerLayout = new QHBoxLayout();
    auto *titleCol = new QVBoxLayout();
    auto *title = new QLabel("Router Inventory & State Management", this);
    title->setStyleSheet("font-size: 18px; font-weight: 800; color: #f8fafc;");
    auto *sub = new QLabel("Manage network nodes, analyze local degrees, and trigger router-level failover", this);
    sub->setStyleSheet("font-size: 12px; color: #64748b;");
    titleCol->addWidget(title);
    titleCol->addWidget(sub);
    headerLayout->addLayout(titleCol);
    headerLayout->addStretch();

    auto *addBtn = new QPushButton("+ Add Router Node", this);
    addBtn->setObjectName("primaryBtn");
    addBtn->setCursor(Qt::PointingHandCursor);
    connect(addBtn, &QPushButton::clicked, this, &RouterView::onAddRouterClicked);
    headerLayout->addWidget(addBtn);

    layout->addLayout(headerLayout);

    // Filters Bar
    auto *filterFrame = new QFrame(this);
    filterFrame->setStyleSheet("background-color: #0f172a; border: 1px solid #1e293b; border-radius: 8px; padding: 6px;");
    auto *filterLayout = new QHBoxLayout(filterFrame);
    filterLayout->setContentsMargins(12, 6, 12, 6);
    filterLayout->setSpacing(14);

    m_searchEdit = new QLineEdit(filterFrame);
    m_searchEdit->setPlaceholderText("🔍 Search router ID or name (e.g. R0, R2)...");
    m_searchEdit->setFixedWidth(280);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &RouterView::applyFilter);
    filterLayout->addWidget(m_searchEdit);

    auto *filterLabel = new QLabel("Status Filter:", filterFrame);
    filterLabel->setStyleSheet("color: #94a3b8; font-size: 12px; font-weight: 600;");
    filterLayout->addWidget(filterLabel);

    m_statusFilter = new QComboBox(filterFrame);
    m_statusFilter->addItems({"All Statuses", "Healthy / Active", "Degraded (Partial Links Down)", "Isolated (All Links Down)"});
    connect(m_statusFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &RouterView::applyFilter);
    filterLayout->addWidget(m_statusFilter);

    filterLayout->addStretch();

    m_summaryLabel = new QLabel("0 Routers Managed", filterFrame);
    m_summaryLabel->setStyleSheet("color: #38bdf8; font-weight: 700; font-size: 12px;");
    filterLayout->addWidget(m_summaryLabel);

    layout->addWidget(filterFrame);

    // Routers Table
    m_table = new QTableWidget(0, 7, this);
    m_table->setHorizontalHeaderLabels({"ID", "Router Name", "Management IP", "Status", "Degree", "Connected Neighbors", "Actions"});
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);
    m_table->verticalHeader()->setVisible(false);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(false);

    layout->addWidget(m_table);
}

void RouterView::updateState(int routerCount, const QList<Connection> &connections) {
    m_routerCount = routerCount;
    m_connections = connections;
    populateTable();
}

void RouterView::populateTable() {
    m_table->setRowCount(0);

    QString search = m_searchEdit->text().trimmed().toLower();
    int filterIdx = m_statusFilter->currentIndex();

    int visibleCount = 0;

    for (int i = 0; i < m_routerCount; ++i) {
        QString name = QString("Router R%1").arg(i);
        QString ip = QString("192.168.1.%1").arg(10 + i);

        // Calculate incident connections and neighbors
        int totalLinks = 0;
        int activeLinks = 0;
        int failedLinks = 0;
        QStringList neighbors;

        for (const auto &conn : m_connections) {
            if (conn.u == i || conn.v == i) {
                totalLinks++;
                int neighbor = (conn.u == i) ? conn.v : conn.u;
                if (conn.failed) {
                    failedLinks++;
                    neighbors.append(QString("R%1 [DOWN]").arg(neighbor));
                } else {
                    activeLinks++;
                    neighbors.append(QString("R%1 (w:%2)").arg(neighbor).arg(conn.weight));
                }
            }
        }

        QString statusStr;
        QColor statusColor;
        int statusCode = 0; // 0=healthy, 1=degraded, 2=isolated

        if (totalLinks > 0 && failedLinks == totalLinks) {
            statusStr = "ISOLATED";
            statusColor = QColor("#ef4444");
            statusCode = 2;
        } else if (failedLinks > 0) {
            statusStr = QString("DEGRADED (%1/%2)").arg(failedLinks).arg(totalLinks);
            statusColor = QColor("#f59e0b");
            statusCode = 1;
        } else {
            statusStr = "ACTIVE";
            statusColor = QColor("#10b981");
            statusCode = 0;
        }

        // Apply search filter
        if (!search.isEmpty()) {
            if (!QString("r%1").arg(i).contains(search) &&
                !name.toLower().contains(search) &&
                !ip.contains(search)) {
                continue;
            }
        }

        // Apply status filter
        if (filterIdx == 1 && statusCode != 0) continue;
        if (filterIdx == 2 && statusCode != 1) continue;
        if (filterIdx == 3 && statusCode != 2) continue;

        int row = m_table->rowCount();
        m_table->insertRow(row);
        visibleCount++;

        // ID
        auto *idItem = new QTableWidgetItem(QString("R%1").arg(i));
        idItem->setFont(QFont("monospace", 11, QFont::Bold));
        idItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(row, 0, idItem);

        // Name
        m_table->setItem(row, 1, new QTableWidgetItem(name));

        // IP
        auto *ipItem = new QTableWidgetItem(ip);
        ipItem->setFont(QFont("monospace", 10));
        m_table->setItem(row, 2, ipItem);

        // Status
        auto *statusItem = new QTableWidgetItem(statusStr);
        statusItem->setForeground(statusColor);
        statusItem->setFont(QFont(font().family(), 10, QFont::Bold));
        m_table->setItem(row, 3, statusItem);

        // Degree
        auto *degreeItem = new QTableWidgetItem(QString("%1 links (%2 active)").arg(totalLinks).arg(activeLinks));
        m_table->setItem(row, 4, degreeItem);

        // Neighbors
        m_table->setItem(row, 5, new QTableWidgetItem(neighbors.join(", ")));

        // Actions cell
        auto *actionWidget = new QWidget();
        auto *actLayout = new QHBoxLayout(actionWidget);
        actLayout->setContentsMargins(4, 2, 4, 2);
        actLayout->setSpacing(6);

        auto *inspectBtn = new QPushButton("Canvas", actionWidget);
        inspectBtn->setCursor(Qt::PointingHandCursor);
        inspectBtn->setStyleSheet("padding: 4px 8px; font-size: 11px;");
        connect(inspectBtn, &QPushButton::clicked, this, [this, i]() {
            emit inspectRouterInTopology(i);
        });
        actLayout->addWidget(inspectBtn);

        if (activeLinks > 0) {
            auto *failBtn = new QPushButton("Isolate", actionWidget);
            failBtn->setObjectName("dangerBtn");
            failBtn->setCursor(Qt::PointingHandCursor);
            failBtn->setStyleSheet("padding: 4px 8px; font-size: 11px;");
            connect(failBtn, &QPushButton::clicked, this, [this, i]() {
                for (const auto &conn : m_connections) {
                    if ((conn.u == i || conn.v == i) && !conn.failed) {
                        m_apiClient->failLink(conn.u, conn.v);
                    }
                }
            });
            actLayout->addWidget(failBtn);
        } else if (failedLinks > 0) {
            auto *recoverBtn = new QPushButton("Restore", actionWidget);
            recoverBtn->setObjectName("successBtn");
            recoverBtn->setCursor(Qt::PointingHandCursor);
            recoverBtn->setStyleSheet("padding: 4px 8px; font-size: 11px;");
            connect(recoverBtn, &QPushButton::clicked, this, [this, i]() {
                for (const auto &conn : m_connections) {
                    if ((conn.u == i || conn.v == i) && conn.failed) {
                        m_apiClient->recoverLink(conn.u, conn.v);
                    }
                }
            });
            actLayout->addWidget(recoverBtn);
        }

        m_table->setCellWidget(row, 6, actionWidget);
    }

    m_summaryLabel->setText(QString("Showing %1 of %2 Routers").arg(visibleCount).arg(m_routerCount));
}

void RouterView::applyFilter() {
    populateTable();
}

void RouterView::onAddRouterClicked() {
    auto reply = QMessageBox::question(
        this, "Provision New Router",
        QString("Do you want to provision a new router endpoint (Router R%1) in the backend C matrix?")
            .arg(m_routerCount),
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        m_apiClient->addRouter();
    }
}
