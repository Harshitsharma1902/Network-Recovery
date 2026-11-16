#pragma once

#include <QObject>
#include <QString>
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include "models/Connection.h"

class ApiClient : public QObject {
    Q_OBJECT

public:
    explicit ApiClient(QObject *parent = nullptr);
    ~ApiClient() override = default;

    void setBaseUrl(const QString &url);
    QString baseUrl() const { return m_baseUrl; }

    void startAutoRefresh(int intervalMs = 2500);
    void stopAutoRefresh();
    bool isAutoRefreshActive() const;

    void fetchState();
    void failLink(int u, int v);
    void recoverLink(int u, int v);
    void findRoute(int src, int dest);
    void undoLastFailure();
    void addRouter();
    void saveEdge(int u, int v, int weight);
    void deleteEdge(int u, int v);

    bool isConnected() const { return m_connected; }

signals:
    void stateUpdated(int routerCount, const QList<Connection> &connections);
    void routeReceived(int src, int dest, bool reachable, int cost, const QList<int> &route);
    void linkFailedResult(int u, int v, bool success, const QString &message);
    void linkRecoveredResult(int u, int v, bool success, const QString &message);
    void undoResult(bool success, const QString &message);
    void routerAddedResult(bool success, int routerId, int count, const QString &message);
    void edgeSavedResult(bool success, int u, int v, int weight, const QString &message);
    void edgeDeletedResult(bool success, int u, int v, const QString &message);
    void backendStatusChanged(bool connected, const QString &message);
    void requestStatus(bool active, const QString &description);

private:
    void setConnectionStatus(bool connected, const QString &message);
    void handleStateReply(const QByteArray &data);
    void handleFailReply(const QByteArray &data, int u, int v);
    void handleRecoverReply(const QByteArray &data, int u, int v);
    void handleRouteReply(const QByteArray &data, int src, int dest);
    void handleUndoReply(const QByteArray &data);
    void handleAddRouterReply(const QByteArray &data);
    void handleSaveEdgeReply(const QByteArray &data, int u, int v, int weight);
    void handleDeleteEdgeReply(const QByteArray &data, int u, int v);

    QString m_baseUrl;
    QNetworkAccessManager *m_networkManager;
    QTimer *m_pollTimer;
    bool m_connected;
    int m_consecutiveFailures;
};
