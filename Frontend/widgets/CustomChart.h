#pragma once

#include <QWidget>
#include <QStringList>
#include <QList>
#include <QColor>

class CustomChart : public QWidget {
    Q_OBJECT

public:
    enum class ChartType {
        BarChart,
        DonutGauge
    };

    explicit CustomChart(ChartType type, const QString &title = "", QWidget *parent = nullptr);

    void setTitle(const QString &title);
    void setBarData(const QStringList &labels, const QList<double> &values, const QColor &barColor = QColor("#0ea5e9"));
    void setDonutData(double percentage, const QString &centerText, const QString &subText, const QColor &fillColor = QColor("#10b981"));

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void paintBarChart(QPainter &painter);
    void paintDonutGauge(QPainter &painter);

    ChartType m_type;
    QString m_title;

    // Bar chart data
    QStringList m_barLabels;
    QList<double> m_barValues;
    QColor m_barColor;

    // Donut chart data
    double m_percentage;
    QString m_centerText;
    QString m_subText;
    QColor m_donutColor;
};
