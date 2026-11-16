#pragma once

#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>

class StatCard : public QFrame {
    Q_OBJECT

public:
    explicit StatCard(const QString &title, const QString &initialValue,
                      const QString &subtitle, const QString &accentColor = "#0ea5e9",
                      QWidget *parent = nullptr);

    void setValue(const QString &value);
    void setSubtitle(const QString &subtitle);
    void setAccentColor(const QString &accentColor);

private:
    QLabel *m_titleLabel;
    QLabel *m_valueLabel;
    QLabel *m_subtitleLabel;
    QString m_accentColor;
    void updateStyle();
};
