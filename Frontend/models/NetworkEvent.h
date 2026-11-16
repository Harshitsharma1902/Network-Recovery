#pragma once

#include <QString>
#include <QDateTime>

enum class EventCategory {
    Info,
    Warning,
    Failure,
    Recovery,
    RouteCalculation,
    Undo
};

struct NetworkEvent {
    int id = 0;
    QDateTime timestamp;
    QString description;
    EventCategory category = EventCategory::Info;
    int routerId = -1;
    int u = -1;
    int v = -1;

    QString categoryName() const {
        switch (category) {
            case EventCategory::Info: return "INFO";
            case EventCategory::Warning: return "WARNING";
            case EventCategory::Failure: return "FAILURE";
            case EventCategory::Recovery: return "RECOVERY";
            case EventCategory::RouteCalculation: return "ROUTING";
            case EventCategory::Undo: return "UNDO";
        }
        return "GENERAL";
    }

    QString categoryColor() const {
        switch (category) {
            case EventCategory::Info: return "#00b4d8";
            case EventCategory::Warning: return "#f59e0b";
            case EventCategory::Failure: return "#ef4444";
            case EventCategory::Recovery: return "#10b981";
            case EventCategory::RouteCalculation: return "#8b5cf6";
            case EventCategory::Undo: return "#ec4899";
        }
        return "#94a3b8";
    }
};
