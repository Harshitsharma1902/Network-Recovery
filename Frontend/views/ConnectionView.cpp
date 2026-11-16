#include "ConnectionView.h"
#include "api/ApiClient.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QMessageBox>

ConnectionView::ConnectionView(ApiClient *apiClient, QWidget *parent)
    : QWidget(parent)
    , m_apiClient(apiClient)
    , m_routerCount(0)
{
    setupUi();
}

void ConnectionView::setupUi() {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 20, 24, 24);
    layout->setSpacing(16);

    // Header & Actions
    auto *headerLayout = new QHBoxLayout();
    auto *titleCol = new QVBoxLayout();
    auto *title = new QLabel("Network Links & Topology Mesh", this);
    title->setStyleSheet("font-size: 18px; font-weight: 800; color: #f8fafc;");
    auto *sub = new QLabel("Inspect edge weights, operational latency metrics, and control physical links", this);
    sub->setStyleSheet("font-size: 12px; color: #64748b;");
    titleCol->addWidget(title);
    titleCol->addWidget(sub);
    headerLayout->addLayout(titleCol);
    headerLayout->addStretch();

    auto *addBtn = new QPushButton("+ Provision New Link", this);
    addBtn->setObjectName("primaryBtn");
    addBtn->setCursor(Qt::PointingHandCursor);
    connect(addBtn, &QPushButton::clicked, this, &ConnectionView::onAddConnectionClicked);
    headerLayout->addWidget(addBtn);

    layout->addLayout(headerLayout);

    // Filters Bar
    auto *filterFrame = new QFrame(this);
    filterFrame->setStyleSheet("background-color: #0f172a; border: 1px solid #1e293b; border-radius: 8px; padding: 6px;");
    auto *filterLayout = new QHBoxLayout(filterFrame);
    filterLayout->setContentsMargins(12, 6, 12, 6);
    filterLayout->setSpacing(14);

    m_searchEdit = new QLineEdit(filterFrame);
    m_searchEdit->setPlaceholderText("🔍 Filter by Router ID (e.g. 0, 3)...");
    m_searchEdit->setFixedWidth(240);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &ConnectionView::applyFilter);
    filterLayout->addWidget(m_searchEdit);

    auto *filterLabel = new QLabel("Link State:", filterFrame);
    filterLabel->setStyleSheet("color: #94a3b8; font-size: 12px; font-weight: 600;");
    filterLayout->addWidget(filterLabel);

    m_statusFilter = new QComboBox(filterFrame);
    m_statusFilter->addItems({"All Links", "Operational (UP)", "Severed / Failed (DOWN)"});
    connect(m_statusFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ConnectionView::applyFilter);
    filterLayout->addWidget(m_statusFilter);

    filterLayout->addStretch();

    m_summaryLabel = new QLabel("0 Total Links", filterFrame);
    m_summaryLabel->setStyleSheet("color: #38bdf8; font-weight: 700; font-size: 12px;");
    filterLayout->addWidget(m_summaryLabel);

    layout->addWidget(filterFrame);

    // Connections Table
    m_table = new QTableWidget(0, 6, this);
    m_table->setHorizontalHeaderLabels({"Link Pair", "Node A", "Node B", "Metric / Cost", "Operational State", "Action"});
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->verticalHeader()->setVisible(false);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    layout->addWidget(m_table);
}

void ConnectionView::updateState(int routerCount, const QList<Connection> &connections) {
    m_routerCount = routerCount;
    m_connections = connections;
    populateTable();
}

void ConnectionView::populateTable() {
    m_table->setRowCount(0);

    QString search = m_searchEdit->text().trimmed();
    int filterIdx = m_statusFilter->currentIndex();

    int visibleCount = 0;

    for (const auto &conn : m_connections) {
        if (!search.isEmpty()) {
            QString uStr = QString::number(conn.u);
            QString vStr = QString::number(conn.v);
            if (!uStr.contains(search) && !vStr.contains(search) &&
                !QString("r%1").arg(conn.u).contains(search.toLower()) &&
                !QString("r%1").arg(conn.v).contains(search.toLower())) {
                continue;
            }
        }

        if (filterIdx == 1 && conn.failed) continue;
        if (filterIdx == 2 && !conn.failed) continue;

        int row = m_table->rowCount();
        m_table->insertRow(row);
        visibleCount++;

        // Pair
        auto *pairItem = new QTableWidgetItem(QString("R%1 <==> R%2").arg(conn.u).arg(conn.v));
        pairItem->setFont(QFont("monospace", 11, QFont::Bold));
        pairItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(row, 0, pairItem);

        // Node A
        auto *uItem = new QTableWidgetItem(QString("Router R%1").arg(conn.u));
        uItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(row, 1, uItem);

        // Node B
        auto *vItem = new QTableWidgetItem(QString("Router R%1").arg(conn.v));
        vItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(row, 2, vItem);

        // Weight
        auto *weightItem = new QTableWidgetItem(QString::number(conn.weight));
        weightItem->setFont(QFont("monospace", 11, QFont::Bold));
        weightItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(row, 3, weightItem);

        // State
        auto *stateItem = new QTableWidgetItem(conn.failed ? "SEVERED (DOWN)" : "OPERATIONAL (UP)");
        stateItem->setForeground(conn.failed ? QColor("#ef4444") : QColor("#10b981"));
        stateItem->setFont(QFont(font().family(), 10, QFont::Bold));
        stateItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(row, 4, stateItem);

        // Action Button
        auto *btn = new QPushButton(conn.failed ? "Restore Link" : "Sever Link");
        btn->setObjectName(conn.failed ? "successBtn" : "dangerBtn");
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet("padding: 4px 10px; font-size: 11px;");

        int u = conn.u;
        int v = conn.v;
        bool isFailed = conn.failed;

        connect(btn, &QPushButton::clicked, this, [this, u, v, isFailed]() {
            if (isFailed) {
                m_apiClient->recoverLink(u, v);
            } else {
                m_apiClient->failLink(u, v);
            }
        });

        m_table->setCellWidget(row, 5, btn);
    }

    m_summaryLabel->setText(QString("Showing %1 of %2 Connections").arg(visibleCount).arg(m_connections.size()));
}

void ConnectionView::applyFilter() {
    populateTable();
}

void ConnectionView::onAddConnectionClicked() {
    emit navigateToEditor();
}
