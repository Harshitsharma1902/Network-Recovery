#pragma once

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QMap>
#include <QList>
#include "models/Connection.h"

class ConnectionEdgeItem;

class RouterNodeItem : public QGraphicsItem {
public:
    enum NodeState {
        Healthy,
        Failed,
        InRoute,
        Warning
    };

    RouterNodeItem(int id, const QString &name, const QString &ip);

    int id() const { return m_id; }
    QString name() const { return m_name; }
    QString ip() const { return m_ip; }

    void setState(NodeState state);
    NodeState state() const { return m_state; }

    void addEdge(ConnectionEdgeItem *edge);
    void clearEdges();

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

private:
    int m_id;
    QString m_name;
    QString m_ip;
    NodeState m_state;
    QList<ConnectionEdgeItem*> m_edges;
};

class ConnectionEdgeItem : public QGraphicsItem {
public:
    ConnectionEdgeItem(RouterNodeItem *source, RouterNodeItem *dest, int weight, bool failed);

    RouterNodeItem* sourceNode() const { return m_source; }
    RouterNodeItem* destNode() const { return m_dest; }
    int weight() const { return m_weight; }
    bool isFailed() const { return m_failed; }

    void setFailed(bool failed);
    void setInRoute(bool inRoute);
    void adjust();

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

private:
    RouterNodeItem *m_source;
    RouterNodeItem *m_dest;
    int m_weight;
    bool m_failed;
    bool m_inRoute;
    QPointF m_sourcePoint;
    QPointF m_destPoint;
};

class NetworkCanvas : public QGraphicsView {
    Q_OBJECT

public:
    explicit NetworkCanvas(QWidget *parent = nullptr);

    void setNetworkData(int routerCount, const QList<Connection> &connections);
    void highlightRoute(const QList<int> &route);
    void clearRouteHighlight();

    void selectRouter(int routerId);
    void selectConnection(int u, int v);

    void autoLayoutCircular();
    void resetZoom();
    void zoomIn();
    void zoomOut();

signals:
    void routerSelected(int routerId);
    void connectionSelected(int u, int v);
    void backgroundClicked();

protected:
    bool event(QEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void drawBackground(QPainter *painter, const QRectF &rect) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    QGraphicsScene *m_scene;
    QMap<int, RouterNodeItem*> m_nodes;
    QList<ConnectionEdgeItem*> m_edges;
    QList<int> m_currentRoute;
    int m_selectedRouterId;
    bool m_initialFitDone;
};
