#include "SettingsView.h"
#include "api/ApiClient.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QScrollArea>
#include <QTextBrowser>

SettingsView::SettingsView(ApiClient *apiClient, QWidget *parent)
    : QWidget(parent)
    , m_apiClient(apiClient)
{
    setupUi();
}

void SettingsView::setupUi() {
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet("background-color: transparent;");

    auto *content = new QWidget(scroll);
    auto *layout = new QVBoxLayout(content);
    layout->setContentsMargins(24, 20, 24, 24);
    layout->setSpacing(20);

    // Header
    auto *title = new QLabel("System Configuration & Architecture", content);
    title->setStyleSheet("font-size: 18px; font-weight: 800; color: #f8fafc;");
    auto *sub = new QLabel("Configure backend connectivity parameters and inspect DSA implementation details", content);
    sub->setStyleSheet("font-size: 12px; color: #64748b;");
    layout->addWidget(title);
    layout->addWidget(sub);

    // Backend Connection Group
    auto *connGroup = new QGroupBox("BACKEND API COMMUNICATION (QT NETWORK)", content);
    auto *cgLayout = new QVBoxLayout(connGroup);
    cgLayout->setSpacing(14);

    auto *urlRow = new QHBoxLayout();
    auto *urlLabel = new QLabel("Mongoose HTTP Base URL:", connGroup);
    urlLabel->setStyleSheet("color: #94a3b8; font-weight: 600; font-size: 12px;");
    m_baseUrlEdit = new QLineEdit(m_apiClient->baseUrl(), connGroup);
    m_baseUrlEdit->setFixedWidth(300);
    urlRow->addWidget(urlLabel);
    urlRow->addWidget(m_baseUrlEdit);
    urlRow->addStretch();
    cgLayout->addLayout(urlRow);

    // Auto Refresh
    auto *refreshRow = new QHBoxLayout();
    m_autoRefreshCheck = new QCheckBox("Enable Live Background Polling", connGroup);
    m_autoRefreshCheck->setChecked(m_apiClient->isAutoRefreshActive());
    refreshRow->addWidget(m_autoRefreshCheck);

    m_intervalSlider = new QSlider(Qt::Horizontal, connGroup);
    m_intervalSlider->setRange(1, 10);
    m_intervalSlider->setValue(2);
    m_intervalSlider->setFixedWidth(160);
    refreshRow->addWidget(m_intervalSlider);

    m_intervalLabel = new QLabel("Polling Interval: 2.0s", connGroup);
    m_intervalLabel->setStyleSheet("color: #38bdf8; font-weight: 600; font-size: 12px;");
    refreshRow->addWidget(m_intervalLabel);
    refreshRow->addStretch();
    cgLayout->addLayout(refreshRow);

    connect(m_intervalSlider, &QSlider::valueChanged, this, [this](int val) {
        m_intervalLabel->setText(QString("Polling Interval: %1.0s").arg(val));
    });

    // Buttons
    auto *btnRow = new QHBoxLayout();
    m_testBtn = new QPushButton("⚡ Test Connection", connGroup);
    m_testBtn->setCursor(Qt::PointingHandCursor);
    connect(m_testBtn, &QPushButton::clicked, this, &SettingsView::testConnection);
    btnRow->addWidget(m_testBtn);

    m_saveBtn = new QPushButton("Save Settings", connGroup);
    m_saveBtn->setObjectName("primaryBtn");
    m_saveBtn->setCursor(Qt::PointingHandCursor);
    connect(m_saveBtn, &QPushButton::clicked, this, &SettingsView::saveSettings);
    btnRow->addWidget(m_saveBtn);

    m_testResultLabel = new QLabel(connGroup);
    m_testResultLabel->setStyleSheet("font-size: 12px; font-weight: 600;");
    btnRow->addWidget(m_testResultLabel);
    btnRow->addStretch();
    cgLayout->addLayout(btnRow);

    layout->addWidget(connGroup);

    // DSA Architecture Documentation Card
    auto *archGroup = new QGroupBox("C BACKEND ARCHITECTURE & DATA STRUCTURES (DSA SPECIFICATION)", content);
    auto *agLayout = new QVBoxLayout(archGroup);
    agLayout->setSpacing(10);

    auto *docBrowser = new QTextBrowser(archGroup);
    docBrowser->setMinimumHeight(320);
    docBrowser->setStyleSheet(
        "QTextBrowser {"
        "  background-color: #0b0f19;"
        "  border: 1px solid #1e293b;"
        "  color: #cbd5e1;"
        "  padding: 12px;"
        "  font-size: 12px;"
        "  line-height: 1.6;"
        "}"
    );

    docBrowser->setHtml(
        "<h3 style='color:#38bdf8;'>1. Graph Representation (Adjacency Matrix)</h3>"
        "<p>The network topology is modeled in <code>Backend/include/graph.h</code> using a 50x50 Adjacency Matrix: "
        "<code>int weight[50][50]</code> and <code>int failed[50][50]</code>. An edge is operational when <code>weight != INF</code> "
        "and <code>failed == 0</code>.</p>"

        "<h3 style='color:#10b981;'>2. Shortest Recovery Route (Dijkstra's Algorithm)</h3>"
        "<p>When an endpoint request is sent to <code>POST /api/route</code>, the C backend executes Dijkstra's algorithm. "
        "It maintains <code>dist[]</code>, <code>visited[]</code>, and <code>previous[]</code> arrays to compute the path with minimum latency cost.</p>"

        "<h3 style='color:#f59e0b;'>3. Reachability Verification (Breadth-First Search - BFS)</h3>"
        "<p>Before running Dijkstra, BFS verifies connectivity between source and destination using an array-based FIFO queue. "
        "If a sequence of operational links does not connect the two nodes, it immediately reports <code>reachable: false</code>.</p>"

        "<h3 style='color:#ec4899;'>4. Rollback & Recovery Stack (LIFO Undo)</h3>"
        "<p>Every failure operation is pushed onto a stack (<code>Stack undoStack</code>) containing <code>Failure {type, u, v}</code>. "
        "When <code>POST /api/undo</code> is called, the stack pops the last failure and restores the previous graph state.</p>"

        "<h3 style='color:#818cf8;'>5. Persistent Storage & Mongoose HTTP Server</h3>"
        "<p>State is saved to <code>Backend/data/connections.txt</code> and event history is logged to <code>Backend/data/events.txt</code>. "
        "The Mongoose HTTP server runs on port 8080 handling JSON serialization.</p>"
    );

    agLayout->addWidget(docBrowser);
    layout->addWidget(archGroup);

    scroll->setWidget(content);
    mainLayout->addWidget(scroll);
}

void SettingsView::saveSettings() {
    m_apiClient->setBaseUrl(m_baseUrlEdit->text().trimmed());
    if (m_autoRefreshCheck->isChecked()) {
        m_apiClient->startAutoRefresh(m_intervalSlider->value() * 1000);
    } else {
        m_apiClient->stopAutoRefresh();
    }
    m_testResultLabel->setText("✓ Settings saved successfully");
    m_testResultLabel->setStyleSheet("color: #10b981;");
}

void SettingsView::testConnection() {
    m_testResultLabel->setText("Testing connection...");
    m_testResultLabel->setStyleSheet("color: #38bdf8;");
    m_apiClient->fetchState();
}
