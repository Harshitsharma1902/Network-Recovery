#pragma once

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QList>
#include "models/Connection.h"
#include "widgets/NetworkCanvas.h"

class ApiClient;

class FailureSimulationView : public QWidget {
    Q_OBJECT

public:
    explicit FailureSimulationView(ApiClient *apiClient, QWidget *parent = nullptr);

    void updateState(int routerCount, const QList<Connection> &connections);
    void setLastUndoResult(bool success, const QString &message);

private slots:
    void executeLinkFailure();
    void executeLinkRecovery();
    void executeRouterFailure();
    void executeUndo();

protected:
    void showEvent(QShowEvent *event) override;

private:
    void setupUi();
    void updateDropdowns();

    ApiClient *m_apiClient;
    int m_routerCount;
    QList<Connection> m_connections;

    // Controls
    QComboBox *m_activeLinksCombo;
    QPushButton *m_failLinkBtn;

    QComboBox *m_failedLinksCombo;
    QPushButton *m_recoverLinkBtn;

    QComboBox *m_routersCombo;
    QPushButton *m_failRouterBtn;

    QPushButton *m_undoBtn;
    QLabel *m_undoStatusLabel;

    // Canvas
    NetworkCanvas *m_canvas;
    QLabel *m_canvasStateLabel;
};
