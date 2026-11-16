#pragma once

#include <QWidget>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QTableWidget>
#include <QList>
#include "models/Connection.h"
#include "widgets/NetworkCanvas.h"

class ApiClient;

class TopologyEditorView : public QWidget {
    Q_OBJECT

public:
    explicit TopologyEditorView(ApiClient *apiClient, QWidget *parent = nullptr);

    void updateState(int routerCount, const QList<Connection> &connections);
    void setFeedbackMessage(bool success, const QString &message);

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void onAddRouterClicked();
    void onSaveEdgeClicked();
    void onDeleteEdgeClicked();
    void onEndpointsChanged();
    void onEdgeTableSelectionChanged();

private:
    void setupUi();
    void updateDropdowns();
    void populateEdgesTable();

    ApiClient *m_apiClient;
    int m_routerCount;
    QList<Connection> m_connections;

    // Feedback banner
    QLabel *m_feedbackBanner;

    // Router provisioning widgets
    QLabel *m_nextRouterLabel;
    QPushButton *m_addRouterBtn;

    // Edge editor widgets
    QComboBox *m_srcCombo;
    QComboBox *m_destCombo;
    QSpinBox *m_weightSpin;
    QLabel *m_edgeStatusNote;
    QPushButton *m_saveEdgeBtn;
    QPushButton *m_deleteEdgeBtn;

    // Edges table
    QTableWidget *m_edgesTable;

    // Canvas
    NetworkCanvas *m_canvas;
};
