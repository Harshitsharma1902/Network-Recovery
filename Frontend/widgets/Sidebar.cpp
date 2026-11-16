#include "Sidebar.h"
#include <QVBoxLayout>
#include <QLabel>

Sidebar::Sidebar(QWidget *parent)
    : QFrame(parent)
    , m_buttonGroup(new QButtonGroup(this))
    , m_currentIndex(0)
{
    setFixedWidth(240);
    setObjectName("sidebar");
    setStyleSheet(
        "QFrame#sidebar {"
        "  background-color: #0b0f19;"
        "  border-right: 1px solid #1e293b;"
        "}"
    );

    setupUi();
}

void Sidebar::setupUi() {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 16, 12, 16);
    layout->setSpacing(6);

    auto *sectionLabel = new QLabel("NAVIGATION", this);
    sectionLabel->setStyleSheet("color: #475569; font-size: 10px; font-weight: 800; padding: 4px 10px; letter-spacing: 1px;");
    layout->addWidget(sectionLabel);

    struct NavItem {
        QString icon;
        QString label;
    };

    QList<NavItem> items = {
        {"⛯", "Overview"},
        {"🌐", "Network Topology"},
        {"🖧", "Routers"},
        {"🔗", "Connections"},
        {"✏", "Route & Edge Editor"},
        {"⚡", "Failure Simulation"},
        {"🎯", "Recovery Analysis"},
        {"📜", "Event History"},
        {"📊", "Analytics"},
        {"⚙", "Settings"}
    };

    for (int i = 0; i < items.size(); ++i) {
        QPushButton *btn = createNavButton(i, items[i].icon, items[i].label);
        layout->addWidget(btn);
        m_navButtons.append(btn);
        m_buttonGroup->addButton(btn, i);
    }

    layout->addStretch();

    // Footer info
    auto *footerFrame = new QFrame(this);
    footerFrame->setStyleSheet("background-color: #0f172a; border: 1px solid #1e293b; border-radius: 6px; padding: 10px;");
    auto *footerLayout = new QVBoxLayout(footerFrame);
    footerLayout->setContentsMargins(8, 8, 8, 8);
    footerLayout->setSpacing(3);

    auto *algoLabel = new QLabel("CORE ALGORITHMS", footerFrame);
    algoLabel->setStyleSheet("color: #38bdf8; font-size: 10px; font-weight: 800;");
    auto *dijkstraLabel = new QLabel("• Dijkstra Shortest Path", footerFrame);
    dijkstraLabel->setStyleSheet("color: #94a3b8; font-size: 11px;");
    auto *bfsLabel = new QLabel("• BFS Reachability", footerFrame);
    bfsLabel->setStyleSheet("color: #94a3b8; font-size: 11px;");
    auto *stackLabel = new QLabel("• Undo Stack & Matrix", footerFrame);
    stackLabel->setStyleSheet("color: #94a3b8; font-size: 11px;");

    footerLayout->addWidget(algoLabel);
    footerLayout->addWidget(dijkstraLabel);
    footerLayout->addWidget(bfsLabel);
    footerLayout->addWidget(stackLabel);
    layout->addWidget(footerFrame);

    connect(m_buttonGroup, &QButtonGroup::idClicked, this, [this](int id) {
        setCurrentIndex(id);
        emit navSelected(id);
    });

    updateStyles();
}

QPushButton* Sidebar::createNavButton(int index, const QString &icon, const QString &text) {
    auto *btn = new QPushButton(QString("%1  %2").arg(icon, text), this);
    btn->setCheckable(true);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setFixedHeight(42);
    btn->setChecked(index == m_currentIndex);
    return btn;
}

void Sidebar::setCurrentIndex(int index) {
    if (index >= 0 && index < m_navButtons.size()) {
        m_currentIndex = index;
        for (int i = 0; i < m_navButtons.size(); ++i) {
            m_navButtons[i]->setChecked(i == index);
        }
        updateStyles();
    }
}

void Sidebar::updateStyles() {
    for (int i = 0; i < m_navButtons.size(); ++i) {
        QPushButton *btn = m_navButtons[i];
        if (i == m_currentIndex) {
            btn->setStyleSheet(
                "QPushButton {"
                "  background-color: #0284c7;"
                "  border: 1px solid #0ea5e9;"
                "  color: #ffffff;"
                "  text-align: left;"
                "  padding-left: 14px;"
                "  font-weight: 700;"
                "  font-size: 13px;"
                "  border-radius: 6px;"
                "}"
            );
        } else {
            btn->setStyleSheet(
                "QPushButton {"
                "  background-color: transparent;"
                "  border: 1px solid transparent;"
                "  color: #94a3b8;"
                "  text-align: left;"
                "  padding-left: 14px;"
                "  font-weight: 500;"
                "  font-size: 13px;"
                "  border-radius: 6px;"
                "}"
                "QPushButton:hover {"
                "  background-color: #1e293b;"
                "  color: #f1f5f9;"
                "}"
            );
        }
    }
}
