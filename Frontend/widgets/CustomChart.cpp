#include "CustomChart.h"
#include <QPainter>
#include <QPaintEvent>
#include <QtMath>

CustomChart::CustomChart(ChartType type, const QString &title, QWidget *parent)
    : QWidget(parent)
    , m_type(type)
    , m_title(title)
    , m_barColor("#0ea5e9")
    , m_percentage(100.0)
    , m_centerText("100%")
    , m_subText("AVAILABLE")
    , m_donutColor("#10b981")
{
    setMinimumSize(220, 180);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void CustomChart::setTitle(const QString &title) {
    m_title = title;
    update();
}

void CustomChart::setBarData(const QStringList &labels, const QList<double> &values, const QColor &barColor) {
    m_barLabels = labels;
    m_barValues = values;
    m_barColor = barColor;
    update();
}

void CustomChart::setDonutData(double percentage, const QString &centerText, const QString &subText, const QColor &fillColor) {
    m_percentage = qBound(0.0, percentage, 100.0);
    m_centerText = centerText;
    m_subText = subText;
    m_donutColor = fillColor;
    update();
}

void CustomChart::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Card background
    painter.setBrush(QColor("#0f172a"));
    painter.setPen(QPen(QColor("#1e293b"), 1));
    painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 8, 8);

    // Title
    if (!m_title.isEmpty()) {
        painter.setPen(QColor("#94a3b8"));
        QFont font = painter.font();
        font.setPointSize(11);
        font.setBold(true);
        font.setLetterSpacing(QFont::AbsoluteSpacing, 0.5);
        painter.setFont(font);
        painter.drawText(QRect(16, 12, width() - 32, 22), Qt::AlignLeft | Qt::AlignVCenter, m_title.toUpper());
    }

    if (m_type == ChartType::BarChart) {
        paintBarChart(painter);
    } else {
        paintDonutGauge(painter);
    }
}

void CustomChart::paintBarChart(QPainter &painter) {
    if (m_barValues.isEmpty()) {
        painter.setPen(QColor("#64748b"));
        painter.drawText(rect(), Qt::AlignCenter, "No telemetry data available");
        return;
    }

    int topMargin = m_title.isEmpty() ? 20 : 44;
    int bottomMargin = 30;
    int leftMargin = 20;
    int rightMargin = 20;

    int chartWidth = width() - leftMargin - rightMargin;
    int chartHeight = height() - topMargin - bottomMargin;

    if (chartWidth <= 0 || chartHeight <= 0) return;

    double maxVal = 1.0;
    for (double v : m_barValues) {
        if (v > maxVal) maxVal = v;
    }

    int n = m_barValues.size();
    double colWidth = (double)chartWidth / n;
    double barWidth = qMin(colWidth * 0.55, 36.0);

    // Horizontal baseline
    painter.setPen(QPen(QColor("#1e293b"), 1));
    painter.drawLine(leftMargin, topMargin + chartHeight, width() - rightMargin, topMargin + chartHeight);

    for (int i = 0; i < n; ++i) {
        double val = m_barValues[i];
        double barH = (val / maxVal) * (chartHeight - 15);
        double x = leftMargin + i * colWidth + (colWidth - barWidth) / 2.0;
        double y = topMargin + chartHeight - barH;

        QRectF barRect(x, y, barWidth, barH);

        // Bar gradient
        QLinearGradient grad(x, y, x, topMargin + chartHeight);
        grad.setColorAt(0, m_barColor);
        grad.setColorAt(1, m_barColor.darker(160));

        painter.setPen(Qt::NoPen);
        painter.setBrush(grad);
        painter.drawRoundedRect(barRect, 4, 4);

        // Value text on top of bar
        painter.setPen(QColor("#f8fafc"));
        QFont valFont = painter.font();
        valFont.setPointSize(10);
        valFont.setBold(true);
        painter.setFont(valFont);
        painter.drawText(QRectF(x - 10, y - 18, barWidth + 20, 16), Qt::AlignCenter, QString::number(val, 'g', 2));

        // Label below baseline
        if (i < m_barLabels.size()) {
            painter.setPen(QColor("#94a3b8"));
            QFont lblFont = painter.font();
            lblFont.setPointSize(10);
            lblFont.setBold(false);
            painter.setFont(lblFont);
            painter.drawText(QRectF(x - 15, topMargin + chartHeight + 6, barWidth + 30, 20),
                             Qt::AlignCenter, m_barLabels[i]);
        }
    }
}

void CustomChart::paintDonutGauge(QPainter &painter) {
    int topMargin = m_title.isEmpty() ? 10 : 36;
    int chartSize = qMin(width() - 40, height() - topMargin - 20);
    if (chartSize <= 0) return;

    QRectF donutRect((width() - chartSize) / 2.0, topMargin + (height() - topMargin - 20 - chartSize) / 2.0,
                     chartSize, chartSize);

    int penWidth = qMax(12, chartSize / 8);
    donutRect.adjust(penWidth / 2.0, penWidth / 2.0, -penWidth / 2.0, -penWidth / 2.0);

    // Track circle
    painter.setPen(QPen(QColor("#1e293b"), penWidth, Qt::SolidLine, Qt::RoundCap));
    painter.setBrush(Qt::NoBrush);
    painter.drawArc(donutRect, 0, 360 * 16);

    // Filled arc (progress)
    int spanAngle = (int)(-m_percentage / 100.0 * 360.0 * 16);
    painter.setPen(QPen(m_donutColor, penWidth, Qt::SolidLine, Qt::RoundCap));
    painter.drawArc(donutRect, 90 * 16, spanAngle);

    // Center Value Text
    painter.setPen(QColor("#f8fafc"));
    QFont centerFont = painter.font();
    centerFont.setPointSize(qMax(14, chartSize / 6));
    centerFont.setBold(true);
    painter.setFont(centerFont);
    painter.drawText(donutRect.adjusted(0, -8, 0, -8), Qt::AlignCenter, m_centerText);

    // Subtitle text
    painter.setPen(QColor("#94a3b8"));
    QFont subFont = painter.font();
    subFont.setPointSize(qMax(9, chartSize / 14));
    subFont.setBold(true);
    painter.setFont(subFont);
    painter.drawText(donutRect.adjusted(0, chartSize / 4, 0, 0), Qt::AlignCenter, m_subText);
}
