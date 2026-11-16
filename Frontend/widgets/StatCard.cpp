#include "StatCard.h"

StatCard::StatCard(const QString &title, const QString &initialValue,
                   const QString &subtitle, const QString &accentColor,
                   QWidget *parent)
    : QFrame(parent)
    , m_accentColor(accentColor)
{
    setObjectName("statCard");
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    setMinimumHeight(105);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 14, 16, 14);
    layout->setSpacing(4);

    m_titleLabel = new QLabel(title.toUpper(), this);
    m_titleLabel->setStyleSheet("font-size: 11px; font-weight: 700; color: #94a3b8; letter-spacing: 0.8px;");

    m_valueLabel = new QLabel(initialValue, this);
    m_valueLabel->setStyleSheet(QString("font-size: 26px; font-weight: 800; color: %1;").arg(m_accentColor));

    m_subtitleLabel = new QLabel(subtitle, this);
    m_subtitleLabel->setStyleSheet("font-size: 12px; color: #64748b; font-weight: 500;");

    layout->addWidget(m_titleLabel);
    layout->addWidget(m_valueLabel);
    layout->addWidget(m_subtitleLabel);
    layout->addStretch();

    updateStyle();
}

void StatCard::setValue(const QString &value) {
    m_valueLabel->setText(value);
}

void StatCard::setSubtitle(const QString &subtitle) {
    m_subtitleLabel->setText(subtitle);
}

void StatCard::setAccentColor(const QString &accentColor) {
    m_accentColor = accentColor;
    m_valueLabel->setStyleSheet(QString("font-size: 26px; font-weight: 800; color: %1;").arg(m_accentColor));
    updateStyle();
}

void StatCard::updateStyle() {
    setStyleSheet(QString(
        "QFrame#statCard {"
        "  background-color: #0f172a;"
        "  border: 1px solid #1e293b;"
        "  border-left: 4px solid %1;"
        "  border-radius: 8px;"
        "}"
        "QFrame#statCard:hover {"
        "  border-color: #334155;"
        "  background-color: #131d35;"
        "}"
    ).arg(m_accentColor));
}
