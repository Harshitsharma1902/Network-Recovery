#include "NetworkCanvas.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QWheelEvent>
#include <QtMath>
#include <QStyleOptionGraphicsItem>
#include <QToolTip>
#include <QTimer>
#include <QResizeEvent>
#include <QShowEvent>

// -------------------------------------------------------------
// RouterNodeItem Implementation
// -------------------------------------------------------------

RouterNodeItem::RouterNodeItem(int id, const QString &name, const QString &ip)
    : m_id(id)
    , m_name(name)
    , m_ip(ip)
    , m_state(Healthy)
{
    setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges);
    setAcceptHoverEvents(true);
    setZValue(10);
    setToolTip(QString("Router R%1\nName: %2\nIP: %3\nStatus: %4")
                   .arg(m_id)
                   .arg(m_name)
                   .arg(m_ip)
                   .arg(m_state == Failed ? "FAILED / ISOLATED" : "ACTIVE"));
}

void RouterNodeItem::setState(NodeState state) {
    if (m_state != state) {
        m_state = state;
        setToolTip(QString("Router R%1\nName: %2\nIP: %3\nStatus: %4")
                       .arg(m_id)
                       .arg(m_name)
                       .arg(m_ip)
                       .arg(m_state == Failed ? "FAILED / ISOLATED" : (m_state == InRoute ? "IN RECOVERY ROUTE" : "ACTIVE")));
        update();
    }
}

void RouterNodeItem::addEdge(ConnectionEdgeItem *edge) {
    if (!m_edges.contains(edge)) {
        m_edges.append(edge);
    }
}

void RouterNodeItem::clearEdges() {
    m_edges.clear();
}

QRectF RouterNodeItem::boundingRect() const {
    qreal adjust = 14;
    return QRectF(-28 - adjust, -28 - adjust, 56 + adjust * 2, 74 + adjust * 2);
}

QPainterPath RouterNodeItem::shape() const {
    QPainterPath path;
    path.addEllipse(-26, -26, 52, 52);
    return path;
}

void RouterNodeItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    painter->setRenderHint(QPainter::Antialiasing);

    QColor baseColor;
    QColor glowColor;

    switch (m_state) {
        case Failed:
            baseColor = QColor("#ef4444"); // Red
            glowColor = QColor(239, 68, 68, 80);
            break;
        case InRoute:
            baseColor = QColor("#a855f7"); // Neon Purple
            glowColor = QColor(168, 85, 247, 100);
            break;
        case Warning:
            baseColor = QColor("#f59e0b"); // Amber
            glowColor = QColor(245, 158, 11, 80);
            break;
        case Healthy:
        default:
            baseColor = QColor("#10b981"); // Emerald Green
            glowColor = QColor(16, 185, 129, 70);
            break;
    }

    // Outer glow / halo
    painter->setPen(Qt::NoPen);
    painter->setBrush(glowColor);
    painter->drawEllipse(-34, -34, 68, 68);

    // Selection ring
    if (isSelected()) {
        painter->setPen(QPen(QColor("#38bdf8"), 2.5, Qt::SolidLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(-31, -31, 62, 62);
    }

    // Node body gradient
    QRadialGradient grad(0, 0, 26);
    grad.setColorAt(0, baseColor.lighter(130));
    grad.setColorAt(0.7, baseColor);
    grad.setColorAt(1, baseColor.darker(150));

    painter->setBrush(grad);
    painter->setPen(QPen(QColor("#0f172a"), 2));
    painter->drawEllipse(-26, -26, 52, 52);

    // Inner highlight ring
    painter->setBrush(Qt::NoBrush);
    painter->setPen(QPen(QColor(255, 255, 255, 60), 1));
    painter->drawEllipse(-23, -23, 46, 46);

    // Router Label inside node (e.g. "R0")
    painter->setPen(QColor("#ffffff"));
    QFont idFont = painter->font();
    idFont.setPointSize(12);
    idFont.setBold(true);
    painter->setFont(idFont);
    painter->drawText(QRectF(-26, -26, 52, 52), Qt::AlignCenter, QString("R%1").arg(m_id));

    // Router Sub-label below node (IP / Name)
    painter->setPen(QColor("#94a3b8"));
    QFont nameFont = painter->font();
    nameFont.setPointSize(9);
    nameFont.setBold(false);
    painter->setFont(nameFont);
    painter->drawText(QRectF(-50, 30, 100, 18), Qt::AlignCenter, m_name.isEmpty() ? QString("Node %1").arg(m_id) : m_name);
}

QVariant RouterNodeItem::itemChange(GraphicsItemChange change, const QVariant &value) {
    if (change == ItemPositionChange || change == ItemPositionHasChanged) {
        for (auto *edge : m_edges) {
            edge->adjust();
        }
    }
    return QGraphicsItem::itemChange(change, value);
}

void RouterNodeItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    QGraphicsItem::mousePressEvent(event);
    auto *canvas = qobject_cast<NetworkCanvas*>(scene()->views().value(0));
    if (canvas) {
        emit canvas->routerSelected(m_id);
    }
}

// -------------------------------------------------------------
// ConnectionEdgeItem Implementation
// -------------------------------------------------------------

ConnectionEdgeItem::ConnectionEdgeItem(RouterNodeItem *source, RouterNodeItem *dest, int weight, bool failed)
    : m_source(source)
    , m_dest(dest)
    , m_weight(weight)
    , m_failed(failed)
    , m_inRoute(false)
{
    setZValue(1);
    setAcceptedMouseButtons(Qt::LeftButton);
    m_source->addEdge(this);
    m_dest->addEdge(this);
    adjust();
    setToolTip(QString("Link: R%1 <-> R%2\nCost: %3\nStatus: %4")
                   .arg(m_source->id())
                   .arg(m_dest->id())
                   .arg(m_weight)
                   .arg(m_failed ? "SEVERED / FAILED" : "OPERATIONAL"));
}

void ConnectionEdgeItem::setFailed(bool failed) {
    if (m_failed != failed) {
        m_failed = failed;
        setToolTip(QString("Link: R%1 <-> R%2\nCost: %3\nStatus: %4")
                       .arg(m_source->id())
                       .arg(m_dest->id())
                       .arg(m_weight)
                       .arg(m_failed ? "SEVERED / FAILED" : "OPERATIONAL"));
        update();
    }
}

void ConnectionEdgeItem::setInRoute(bool inRoute) {
    if (m_inRoute != inRoute) {
        m_inRoute = inRoute;
        setZValue(m_inRoute ? 5 : 1);
        update();
    }
}

void ConnectionEdgeItem::adjust() {
    if (!m_source || !m_dest) return;

    prepareGeometryChange();
    m_sourcePoint = m_source->pos();
    m_destPoint = m_dest->pos();
}

QRectF ConnectionEdgeItem::boundingRect() const {
    if (!m_source || !m_dest) return QRectF();

    qreal extra = (m_inRoute ? 12 : 6) + 30; // accommodate weight pill
    return QRectF(m_sourcePoint, QSizeF(m_destPoint.x() - m_sourcePoint.x(),
                                        m_destPoint.y() - m_sourcePoint.y()))
        .normalized()
        .adjusted(-extra, -extra, extra, extra);
}

void ConnectionEdgeItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    if (!m_source || !m_dest) return;

    painter->setRenderHint(QPainter::Antialiasing);

    QLineF line(m_sourcePoint, m_destPoint);
    if (qFuzzyCompare(line.length(), qreal(0.0))) return;

    // Draw the link line
    if (m_inRoute) {
        // Glowing recovery path
        painter->setPen(QPen(QColor(168, 85, 247, 80), 8, Qt::SolidLine, Qt::RoundCap));
        painter->drawLine(line);

        painter->setPen(QPen(QColor("#a855f7"), 4, Qt::SolidLine, Qt::RoundCap));
        painter->drawLine(line);
    } else if (m_failed) {
        // Failed link - dashed red
        painter->setPen(QPen(QColor("#ef4444"), 2.5, Qt::DashLine, Qt::RoundCap));
        painter->drawLine(line);
    } else {
        // Healthy link - cyan
        painter->setPen(QPen(QColor("#0ea5e9"), 2.5, Qt::SolidLine, Qt::RoundCap));
        painter->drawLine(line);
    }

    // Weight Capsule Badge at midpoint
    QPointF mid = (m_sourcePoint + m_destPoint) / 2.0;
    QString weightText = QString::number(m_weight);

    QRectF badgeRect(mid.x() - 14, mid.y() - 10, 28, 20);

    // Badge background
    painter->setBrush(QColor("#0f172a"));
    QColor badgeBorder = m_inRoute ? QColor("#a855f7") : (m_failed ? QColor("#ef4444") : QColor("#334155"));
    painter->setPen(QPen(badgeBorder, 1.5));
    painter->drawRoundedRect(badgeRect, 6, 6);

    // Badge text
    painter->setPen(m_inRoute ? QColor("#f3e8ff") : (m_failed ? QColor("#fca5a5") : QColor("#94a3b8")));
    QFont badgeFont = painter->font();
    badgeFont.setPointSize(9);
    badgeFont.setBold(true);
    painter->setFont(badgeFont);
    painter->drawText(badgeRect, Qt::AlignCenter, weightText);
}

void ConnectionEdgeItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    QGraphicsItem::mousePressEvent(event);
    auto *canvas = qobject_cast<NetworkCanvas*>(scene()->views().value(0));
    if (canvas && m_source && m_dest) {
        emit canvas->connectionSelected(m_source->id(), m_dest->id());
    }
}

// -------------------------------------------------------------
// NetworkCanvas Implementation
// -------------------------------------------------------------

NetworkCanvas::NetworkCanvas(QWidget *parent)
    : QGraphicsView(parent)
    , m_scene(new QGraphicsScene(this))
    , m_selectedRouterId(-1)
    , m_initialFitDone(false)
{
    setScene(m_scene);
    setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
    setDragMode(QGraphicsView::ScrollHandDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorViewCenter);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameShape(QFrame::NoFrame);
    m_scene->setSceneRect(-800, -800, 1600, 1600);
}

void NetworkCanvas::drawBackground(QPainter *painter, const QRectF &rect) {
    painter->fillRect(rect, QColor("#080c14"));

    // Subtle NOC grid dots
    painter->setPen(QPen(QColor("#172033"), 1));
    qreal left = int(rect.left()) - (int(rect.left()) % 40);
    qreal top = int(rect.top()) - (int(rect.top()) % 40);

    QVarLengthArray<QPointF, 100> points;
    for (qreal x = left; x < rect.right(); x += 40) {
        for (qreal y = top; y < rect.bottom(); y += 40) {
            points.append(QPointF(x, y));
        }
    }
    painter->drawPoints(points.data(), points.size());
}

void NetworkCanvas::setNetworkData(int routerCount, const QList<Connection> &connections) {
    // Preserve positions of existing nodes
    QMap<int, QPointF> existingPositions;
    for (auto it = m_nodes.begin(); it != m_nodes.end(); ++it) {
        existingPositions[it.key()] = it.value()->pos();
    }

    m_scene->clear();
    m_nodes.clear();
    m_edges.clear();

    // Create router nodes
    for (int i = 0; i < routerCount; ++i) {
        auto *node = new RouterNodeItem(i, QString("Router R%1").arg(i), QString("192.168.1.%1").arg(10 + i));
        if (existingPositions.contains(i)) {
            node->setPos(existingPositions[i]);
        }
        m_scene->addItem(node);
        m_nodes[i] = node;
    }

    // Create edges
    for (const auto &conn : connections) {
        if (m_nodes.contains(conn.u) && m_nodes.contains(conn.v)) {
            auto *edge = new ConnectionEdgeItem(m_nodes[conn.u], m_nodes[conn.v], conn.weight, conn.failed);
            m_scene->addItem(edge);
            m_edges.append(edge);
        }
    }

    // If new layout, arrange circularly
    if (existingPositions.isEmpty() && routerCount > 0) {
        autoLayoutCircular();
    }

    // Determine router health states based on connected links
    for (int i = 0; i < routerCount; ++i) {
        int totalLinks = 0;
        int failedLinks = 0;
        for (const auto &conn : connections) {
            if (conn.u == i || conn.v == i) {
                totalLinks++;
                if (conn.failed) failedLinks++;
            }
        }

        if (totalLinks > 0 && failedLinks == totalLinks) {
            m_nodes[i]->setState(RouterNodeItem::Failed);
        } else if (failedLinks > 0) {
            m_nodes[i]->setState(RouterNodeItem::Warning);
        } else {
            m_nodes[i]->setState(RouterNodeItem::Healthy);
        }
    }

    // Reapply current route highlighting if active
    if (!m_currentRoute.isEmpty()) {
        highlightRoute(m_currentRoute);
    }
}

void NetworkCanvas::autoLayoutCircular() {
    int n = m_nodes.size();
    if (n == 0) return;

    qreal radius = qMax(160.0, n * 38.0);
    int idx = 0;
    for (auto *node : m_nodes) {
        qreal angle = 2.0 * M_PI * idx / n - (M_PI / 2.0);
        qreal x = radius * qCos(angle);
        qreal y = radius * qSin(angle);
        node->setPos(x, y);
        idx++;
    }

    for (auto *edge : m_edges) {
        edge->adjust();
    }

    resetZoom();
}

void NetworkCanvas::highlightRoute(const QList<int> &route) {
    m_currentRoute = route;

    // Reset edges
    for (auto *edge : m_edges) {
        edge->setInRoute(false);
    }

    // Reset node route states
    for (auto *node : m_nodes) {
        if (node->state() == RouterNodeItem::InRoute) {
            node->setState(RouterNodeItem::Healthy);
        }
    }

    if (route.size() < 2) return;

    // Highlight route nodes
    for (int nodeId : route) {
        if (m_nodes.contains(nodeId)) {
            m_nodes[nodeId]->setState(RouterNodeItem::InRoute);
        }
    }

    // Highlight route edges
    for (int i = 0; i < route.size() - 1; ++i) {
        int u = route[i];
        int v = route[i + 1];
        for (auto *edge : m_edges) {
            int srcId = edge->sourceNode()->id();
            int destId = edge->destNode()->id();
            if ((srcId == u && destId == v) || (srcId == v && destId == u)) {
                edge->setInRoute(true);
            }
        }
    }
}

void NetworkCanvas::clearRouteHighlight() {
    m_currentRoute.clear();
    for (auto *edge : m_edges) {
        edge->setInRoute(false);
    }
    for (auto *node : m_nodes) {
        if (node->state() == RouterNodeItem::InRoute) {
            node->setState(RouterNodeItem::Healthy);
        }
    }
}

void NetworkCanvas::selectRouter(int routerId) {
    m_selectedRouterId = routerId;
    m_scene->clearSelection();
    if (m_nodes.contains(routerId)) {
        m_nodes[routerId]->setSelected(true);
        centerOn(m_nodes[routerId]);
    }
}

void NetworkCanvas::selectConnection(int u, int v) {
    for (auto *edge : m_edges) {
        int srcId = edge->sourceNode()->id();
        int destId = edge->destNode()->id();
        if ((srcId == u && destId == v) || (srcId == v && destId == u)) {
            m_scene->clearSelection();
            centerOn(edge);
            break;
        }
    }
}

void NetworkCanvas::resetZoom() {
    QRectF rect = m_scene->itemsBoundingRect();
    if (!rect.isValid() || rect.isEmpty()) {
        rect = m_scene->sceneRect();
    }
    if (rect.isValid() && !rect.isEmpty()) {
        fitInView(rect.adjusted(-60, -60, 60, 60), Qt::KeepAspectRatio);
    }
}

void NetworkCanvas::zoomIn() {
    scale(1.2, 1.2);
}

void NetworkCanvas::zoomOut() {
    scale(1.0 / 1.2, 1.0 / 1.2);
}

bool NetworkCanvas::event(QEvent *event) {
    if (event->type() == QEvent::NativeGesture || event->type() == QEvent::Gesture) {
        event->ignore();
        return true;
    }
    return QGraphicsView::event(event);
}

void NetworkCanvas::wheelEvent(QWheelEvent *event) {
    // Two-finger trackpad zoom and mouse wheel zoom disabled per user specification.
    // Zoom is controlled cleanly via dedicated UI buttons.
    event->ignore();
}

void NetworkCanvas::showEvent(QShowEvent *event) {
    QGraphicsView::showEvent(event);
    QTimer::singleShot(30, this, &NetworkCanvas::resetZoom);
}

void NetworkCanvas::resizeEvent(QResizeEvent *event) {
    QGraphicsView::resizeEvent(event);
    if (!m_initialFitDone && !m_nodes.isEmpty()) {
        m_initialFitDone = true;
        QTimer::singleShot(30, this, &NetworkCanvas::resetZoom);
    }
}

void NetworkCanvas::mousePressEvent(QMouseEvent *event) {
    QGraphicsView::mousePressEvent(event);
    if (!itemAt(event->pos())) {
        emit backgroundClicked();
    }
}
