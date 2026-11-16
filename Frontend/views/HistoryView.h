#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QList>
#include "models/NetworkEvent.h"

class HistoryView : public QWidget {
    Q_OBJECT

public:
    explicit HistoryView(QWidget *parent = nullptr);

    void addEvent(const NetworkEvent &event);
    void loadInitialEventsFromFile(const QString &filePath);

private slots:
    void applyFilter();
    void clearHistory();

private:
    void setupUi();
    void populateTable();

    QList<NetworkEvent> m_events;

    QLineEdit *m_searchEdit;
    QComboBox *m_categoryFilter;
    QTableWidget *m_table;
    QLabel *m_countLabel;
};
