#include "ApiClient.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QNetworkRequest>
#include <QDebug>

ApiClient::ApiClient(QObject *parent)
    : QObject(parent)
    , m_baseUrl("http://127.0.0.1:8080")
    , m_networkManager(new QNetworkAccessManager(this))
    , m_pollTimer(new QTimer(this))
    , m_connected(false)
    , m_consecutiveFailures(0)
{
    connect(m_pollTimer, &QTimer::timeout, this, &ApiClient::fetchState);
}

void ApiClient::setBaseUrl(const QString &url) {
    m_baseUrl = url.trimmed();
    if (m_baseUrl.endsWith('/')) {
        m_baseUrl.chop(1);
    }
}

void ApiClient::startAutoRefresh(int intervalMs) {
    m_pollTimer->start(intervalMs);
    fetchState();
}

void ApiClient::stopAutoRefresh() {
    m_pollTimer->stop();
}

bool ApiClient::isAutoRefreshActive() const {
    return m_pollTimer->isActive();
}

void ApiClient::setConnectionStatus(bool connected, const QString &message) {
    if (m_connected != connected) {
        m_connected = connected;
        emit backendStatusChanged(connected, message);
    }
}

void ApiClient::fetchState() {
    emit requestStatus(true, "Fetching network state...");
    QUrl url(m_baseUrl + "/api/state");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setTransferTimeout(3000);

    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        emit requestStatus(false, "");
        if (reply->error() != QNetworkReply::NoError) {
            m_consecutiveFailures++;
            if (m_consecutiveFailures >= 2) {
                setConnectionStatus(false, reply->errorString());
            }
            reply->deleteLater();
            return;
        }

        m_consecutiveFailures = 0;
        setConnectionStatus(true, "Connected to backend");
        QByteArray data = reply->readAll();
        handleStateReply(data);
        reply->deleteLater();
    });
}

void ApiClient::handleStateReply(const QByteArray &data) {
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        qWarning() << "Failed to parse /api/state response:" << parseError.errorString();
        return;
    }

    QJsonObject root = doc.object();
    int count = root.value("count").toInt(0);
    QJsonArray edgesArray = root.value("edges").toArray();

    QList<Connection> connections;
    for (const QJsonValue &val : edgesArray) {
        if (!val.isObject()) continue;
        QJsonObject edgeObj = val.toObject();
        Connection conn;
        conn.u = edgeObj.value("u").toInt(-1);
        conn.v = edgeObj.value("v").toInt(-1);
        conn.weight = edgeObj.value("weight").toInt(1);
        conn.failed = edgeObj.value("failed").toBool(false);
        if (conn.u >= 0 && conn.v >= 0) {
            connections.append(conn);
        }
    }

    emit stateUpdated(count, connections);
}

void ApiClient::failLink(int u, int v) {
    emit requestStatus(true, QString("Failing link R%1 - R%2...").arg(u).arg(v));
    QUrl url(m_baseUrl + "/api/link/fail");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setTransferTimeout(4000);

    QJsonObject payload;
    payload["u"] = u;
    payload["v"] = v;
    QByteArray body = QJsonDocument(payload).toJson(QJsonDocument::Compact);

    QNetworkReply *reply = m_networkManager->post(request, body);
    connect(reply, &QNetworkReply::finished, this, [this, reply, u, v]() {
        emit requestStatus(false, "");
        if (reply->error() != QNetworkReply::NoError) {
            emit linkFailedResult(u, v, false, reply->errorString());
            reply->deleteLater();
            return;
        }

        QByteArray data = reply->readAll();
        handleFailReply(data, u, v);
        reply->deleteLater();
    });
}

void ApiClient::handleFailReply(const QByteArray &data, int u, int v) {
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject root = doc.object();
    if (root.contains("error")) {
        emit linkFailedResult(u, v, false, root.value("error").toString());
    } else if (root.value("success").toBool(false)) {
        emit linkFailedResult(u, v, true, QString("Link R%1-R%2 severed successfully").arg(u).arg(v));
        fetchState();
    } else {
        emit linkFailedResult(u, v, false, "Unknown failure response");
    }
}

void ApiClient::recoverLink(int u, int v) {
    emit requestStatus(true, QString("Recovering link R%1 - R%2...").arg(u).arg(v));
    QUrl url(m_baseUrl + "/api/link/recover");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setTransferTimeout(4000);

    QJsonObject payload;
    payload["u"] = u;
    payload["v"] = v;
    QByteArray body = QJsonDocument(payload).toJson(QJsonDocument::Compact);

    QNetworkReply *reply = m_networkManager->post(request, body);
    connect(reply, &QNetworkReply::finished, this, [this, reply, u, v]() {
        emit requestStatus(false, "");
        if (reply->error() != QNetworkReply::NoError) {
            emit linkRecoveredResult(u, v, false, reply->errorString());
            reply->deleteLater();
            return;
        }

        QByteArray data = reply->readAll();
        handleRecoverReply(data, u, v);
        reply->deleteLater();
    });
}

void ApiClient::handleRecoverReply(const QByteArray &data, int u, int v) {
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject root = doc.object();
    if (root.contains("error")) {
        emit linkRecoveredResult(u, v, false, root.value("error").toString());
    } else if (root.value("success").toBool(false)) {
        emit linkRecoveredResult(u, v, true, QString("Link R%1-R%2 restored successfully").arg(u).arg(v));
        fetchState();
    } else {
        emit linkRecoveredResult(u, v, false, "Unknown recovery response");
    }
}

void ApiClient::findRoute(int src, int dest) {
    emit requestStatus(true, QString("Computing shortest recovery route R%1 -> R%2...").arg(src).arg(dest));
    QUrl url(m_baseUrl + "/api/route");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setTransferTimeout(4000);

    QJsonObject payload;
    payload["src"] = src;
    payload["dest"] = dest;
    QByteArray body = QJsonDocument(payload).toJson(QJsonDocument::Compact);

    QNetworkReply *reply = m_networkManager->post(request, body);
    connect(reply, &QNetworkReply::finished, this, [this, reply, src, dest]() {
        emit requestStatus(false, "");
        if (reply->error() != QNetworkReply::NoError) {
            emit routeReceived(src, dest, false, -1, {});
            reply->deleteLater();
            return;
        }

        QByteArray data = reply->readAll();
        handleRouteReply(data, src, dest);
        reply->deleteLater();
    });
}

void ApiClient::handleRouteReply(const QByteArray &data, int src, int dest) {
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject root = doc.object();

    if (root.contains("error")) {
        qWarning() << "Route error:" << root.value("error").toString();
        emit routeReceived(src, dest, false, -1, {});
        return;
    }

    bool reachable = root.value("reachable").toBool(false);
    if (!reachable) {
        emit routeReceived(src, dest, false, -1, {});
        return;
    }

    int cost = root.value("cost").toInt(0);
    QJsonArray routeArr = root.value("route").toArray();
    QList<int> route;
    for (const QJsonValue &nodeVal : routeArr) {
        route.append(nodeVal.toInt());
    }

    emit routeReceived(src, dest, true, cost, route);
}

void ApiClient::undoLastFailure() {
    emit requestStatus(true, "Requesting undo of last failure...");
    QUrl url(m_baseUrl + "/api/undo");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setTransferTimeout(4000);

    QByteArray body = "{}";

    QNetworkReply *reply = m_networkManager->post(request, body);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        emit requestStatus(false, "");
        if (reply->error() != QNetworkReply::NoError) {
            emit undoResult(false, reply->errorString());
            reply->deleteLater();
            return;
        }

        QByteArray data = reply->readAll();
        handleUndoReply(data);
        reply->deleteLater();
    });
}

void ApiClient::handleUndoReply(const QByteArray &data) {
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject root = doc.object();
    if (root.contains("error")) {
        emit undoResult(false, root.value("error").toString());
    } else if (root.value("success").toBool(false)) {
        emit undoResult(true, "Last failure operation was undone successfully.");
        fetchState();
    } else {
        emit undoResult(false, "Unexpected undo response from server.");
    }
}

void ApiClient::addRouter() {
    emit requestStatus(true, "Provisioning new router node in backend graph...");
    QUrl url(m_baseUrl + "/api/router/add");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setTransferTimeout(4000);

    QNetworkReply *reply = m_networkManager->post(request, QByteArray("{}"));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        emit requestStatus(false, "");
        if (reply->error() != QNetworkReply::NoError) {
            emit routerAddedResult(false, -1, -1, reply->errorString());
            reply->deleteLater();
            return;
        }

        QByteArray data = reply->readAll();
        handleAddRouterReply(data);
        reply->deleteLater();
    });
}

void ApiClient::handleAddRouterReply(const QByteArray &data) {
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject root = doc.object();
    if (root.contains("error")) {
        emit routerAddedResult(false, -1, -1, root.value("error").toString());
    } else if (root.value("success").toBool(false)) {
        int id = root.value("id").toInt(-1);
        int count = root.value("count").toInt(-1);
        emit routerAddedResult(true, id, count, QString("Router R%1 provisioned successfully.").arg(id));
        fetchState();
    } else {
        emit routerAddedResult(false, -1, -1, "Unknown router provisioning response.");
    }
}

void ApiClient::saveEdge(int u, int v, int weight) {
    emit requestStatus(true, QString("Saving edge R%1 <-> R%2 (weight %3)...").arg(u).arg(v).arg(weight));
    QUrl url(m_baseUrl + "/api/edge/save");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setTransferTimeout(4000);

    QJsonObject payload;
    payload["u"] = u;
    payload["v"] = v;
    payload["weight"] = weight;
    QByteArray body = QJsonDocument(payload).toJson(QJsonDocument::Compact);

    QNetworkReply *reply = m_networkManager->post(request, body);
    connect(reply, &QNetworkReply::finished, this, [this, reply, u, v, weight]() {
        emit requestStatus(false, "");
        if (reply->error() != QNetworkReply::NoError) {
            emit edgeSavedResult(false, u, v, weight, reply->errorString());
            reply->deleteLater();
            return;
        }

        QByteArray data = reply->readAll();
        handleSaveEdgeReply(data, u, v, weight);
        reply->deleteLater();
    });
}

void ApiClient::handleSaveEdgeReply(const QByteArray &data, int u, int v, int weight) {
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject root = doc.object();
    if (root.contains("error")) {
        emit edgeSavedResult(false, u, v, weight, root.value("error").toString());
    } else if (root.value("success").toBool(false)) {
        emit edgeSavedResult(true, u, v, weight, QString("Link R%1-R%2 saved with weight %3.").arg(u).arg(v).arg(weight));
        fetchState();
    } else {
        emit edgeSavedResult(false, u, v, weight, "Unexpected edge save response.");
    }
}

void ApiClient::deleteEdge(int u, int v) {
    emit requestStatus(true, QString("Deleting edge R%1 <-> R%2...").arg(u).arg(v));
    QUrl url(m_baseUrl + "/api/edge/delete");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setTransferTimeout(4000);

    QJsonObject payload;
    payload["u"] = u;
    payload["v"] = v;
    QByteArray body = QJsonDocument(payload).toJson(QJsonDocument::Compact);

    QNetworkReply *reply = m_networkManager->post(request, body);
    connect(reply, &QNetworkReply::finished, this, [this, reply, u, v]() {
        emit requestStatus(false, "");
        if (reply->error() != QNetworkReply::NoError) {
            emit edgeDeletedResult(false, u, v, reply->errorString());
            reply->deleteLater();
            return;
        }

        QByteArray data = reply->readAll();
        handleDeleteEdgeReply(data, u, v);
        reply->deleteLater();
    });
}

void ApiClient::handleDeleteEdgeReply(const QByteArray &data, int u, int v) {
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject root = doc.object();
    if (root.contains("error")) {
        emit edgeDeletedResult(false, u, v, root.value("error").toString());
    } else if (root.value("success").toBool(false)) {
        emit edgeDeletedResult(true, u, v, QString("Link R%1-R%2 deleted successfully.").arg(u).arg(v));
        fetchState();
    } else {
        emit edgeDeletedResult(false, u, v, "Unexpected edge delete response.");
    }
}
