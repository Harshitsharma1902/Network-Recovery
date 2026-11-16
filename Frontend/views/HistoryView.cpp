#include "HistoryView.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QFile>
#include <QTextStream>

HistoryView::HistoryView(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void HistoryView::setupUi() {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 20, 24, 24);
    layout->setSpacing(16);

    // Header & Actions
    auto *headerLayout = new QHBoxLayout();
    auto *titleCol = new QVBoxLayout();
    auto *title = new QLabel("NOC Event Audit Log", this);
    title->setStyleSheet("font-size: 18px; font-weight: 800; color: #f8fafc;");
    auto *sub = new QLabel("Historical event tracking linked to backend EventNode linked-list and persistence", this);
    sub->setStyleSheet("font-size: 12px; color: #64748b;");
    titleCol->addWidget(title);
    titleCol->addWidget(sub);
    headerLayout->addLayout(titleCol);
    headerLayout->addStretch();

    auto *clearBtn = new QPushButton("Clear Session Log", this);
    clearBtn->setCursor(Qt::PointingHandCursor);
    connect(clearBtn, &QPushButton::clicked, this, &HistoryView::clearHistory);
    headerLayout->addWidget(clearBtn);

    layout->addLayout(headerLayout);

    // Filters Bar
    auto *filterFrame = new QFrame(this);
    filterFrame->setStyleSheet("background-color: #0f172a; border: 1px solid #1e293b; border-radius: 8px; padding: 6px;");
    auto *filterLayout = new QHBoxLayout(filterFrame);
    filterLayout->setContentsMargins(12, 6, 12, 6);
    filterLayout->setSpacing(14);

    m_searchEdit = new QLineEdit(filterFrame);
    m_searchEdit->setPlaceholderText("🔍 Search event description...");
    m_searchEdit->setFixedWidth(280);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &HistoryView::applyFilter);
    filterLayout->addWidget(m_searchEdit);

    auto *filterLabel = new QLabel("Category:", filterFrame);
    filterLabel->setStyleSheet("color: #94a3b8; font-size: 12px; font-weight: 600;");
    filterLayout->addWidget(filterLabel);

    m_categoryFilter = new QComboBox(filterFrame);
    m_categoryFilter->addItems({"All Categories", "FAILURES", "RECOVERIES", "UNDO OPERATIONS", "ROUTING", "INFO"});
    connect(m_categoryFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &HistoryView::applyFilter);
    filterLayout->addWidget(m_categoryFilter);

    filterLayout->addStretch();

    m_countLabel = new QLabel("0 Events Recorded", filterFrame);
    m_countLabel->setStyleSheet("color: #38bdf8; font-weight: 700; font-size: 12px;");
    filterLayout->addWidget(m_countLabel);

    layout->addWidget(filterFrame);

    // Events Table
    m_table = new QTableWidget(0, 5, this);
    m_table->setHorizontalHeaderLabels({"Event ID", "Timestamp", "Category", "Description", "Target"});
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    m_table->verticalHeader()->setVisible(false);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    layout->addWidget(m_table);
}

void HistoryView::addEvent(const NetworkEvent &event) {
    m_events.prepend(event);
    populateTable();
}

void HistoryView::clearHistory() {
    m_events.clear();
    populateTable();
}

void HistoryView::loadInitialEventsFromFile(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#')) continue;

        QStringList parts = line.split('\t');
        if (parts.size() >= 2) {
            NetworkEvent ev;
            ev.id = parts[0].toInt();
            ev.description = parts[1];
            ev.timestamp = QDateTime::currentDateTime();

            if (ev.description.contains("failed", Qt::CaseInsensitive)) {
                ev.category = EventCategory::Failure;
            } else if (ev.description.contains("recovered", Qt::CaseInsensitive)) {
                ev.category = EventCategory::Recovery;
            } else if (ev.description.contains("undone", Qt::CaseInsensitive)) {
                ev.category = EventCategory::Undo;
            } else {
                ev.category = EventCategory::Info;
            }

            m_events.append(ev);
        }
    }
    file.close();
    populateTable();
}

void HistoryView::populateTable() {
    m_table->setRowCount(0);

    QString search = m_searchEdit->text().trimmed().toLower();
    int catIdx = m_categoryFilter->currentIndex();

    int visible = 0;

    for (const auto &ev : m_events) {
        if (!search.isEmpty() && !ev.description.toLower().contains(search)) {
            continue;
        }

        if (catIdx == 1 && ev.category != EventCategory::Failure) continue;
        if (catIdx == 2 && ev.category != EventCategory::Recovery) continue;
        if (catIdx == 3 && ev.category != EventCategory::Undo) continue;
        if (catIdx == 4 && ev.category != EventCategory::RouteCalculation) continue;
        if (catIdx == 5 && ev.category != EventCategory::Info) continue;

        int row = m_table->rowCount();
        m_table->insertRow(row);
        visible++;

        // ID
        auto *idItem = new QTableWidgetItem(QString("#%1").arg(ev.id));
        idItem->setTextAlignment(Qt::AlignCenter);
        idItem->setFont(QFont("monospace", 10, QFont::Bold));
        m_table->setItem(row, 0, idItem);

        // Timestamp
        auto *timeItem = new QTableWidgetItem(ev.timestamp.toString("yyyy-MM-dd hh:mm:ss"));
        timeItem->setFont(QFont("monospace", 10));
        m_table->setItem(row, 1, timeItem);

        // Category Badge
        auto *badge = new QLabel(ev.categoryName());
        badge->setAlignment(Qt::AlignCenter);
        badge->setStyleSheet(QString(
            "background-color: %1; color: #ffffff; font-weight: 700; "
            "font-size: 10px; padding: 3px 6px; border-radius: 4px;"
        ).arg(ev.categoryColor()));

        auto *badgeWidget = new QWidget();
        auto *badgeLayout = new QHBoxLayout(badgeWidget);
        badgeLayout->setContentsMargins(4, 4, 4, 4);
        badgeLayout->addWidget(badge);
        m_table->setCellWidget(row, 2, badgeWidget);

        // Description
        m_table->setItem(row, 3, new QTableWidgetItem(ev.description));

        // Target
        QString target = "--";
        if (ev.routerId >= 0) target = QString("Router R%1").arg(ev.routerId);
        else if (ev.u >= 0 && ev.v >= 0) target = QString("Link R%1-R%2").arg(ev.u).arg(ev.v);
        m_table->setItem(row, 4, new QTableWidgetItem(target));
    }

    m_countLabel->setText(QString("Showing %1 of %2 Events").arg(visible).arg(m_events.size()));
}

void HistoryView::applyFilter() {
    populateTable();
}
