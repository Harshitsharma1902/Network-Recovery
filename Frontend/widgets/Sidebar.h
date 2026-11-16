#pragma once

#include <QFrame>
#include <QPushButton>
#include <QButtonGroup>
#include <QList>

class Sidebar : public QFrame {
    Q_OBJECT

public:
    explicit Sidebar(QWidget *parent = nullptr);

    void setCurrentIndex(int index);
    int currentIndex() const { return m_currentIndex; }

signals:
    void navSelected(int index);

private:
    void setupUi();
    QPushButton* createNavButton(int index, const QString &icon, const QString &text);
    void updateStyles();

    QButtonGroup *m_buttonGroup;
    QList<QPushButton*> m_navButtons;
    int m_currentIndex;
};
