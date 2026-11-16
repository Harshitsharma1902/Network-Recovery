#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include "models/Connection.h"

class ApiClient;

class ConnectionView : public QWidget {
    Q_OBJECT

public:
    explicit ConnectionView(ApiClient *apiClient, QWidget *parent = nullptr);

    void updateState(int routerCount, const QList<Connection> &connections);

signals:
    void inspectConnectionInTopology(int u, int v);
    void navigateToEditor();

private slots:
    void applyFilter();
    void onAddConnectionClicked();

private:
    void setupUi();
    void populateTable();

    ApiClient *m_apiClient;
    int m_routerCount;
    QList<Connection> m_connections;

    QLineEdit *m_searchEdit;
    QComboBox *m_statusFilter;
    QTableWidget *m_table;
    QLabel *m_summaryLabel;
};
