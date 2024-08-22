#pragma once
#define OR ||
#define AND &&
#define REPA(type, container, action) \
    for (type * item : container) {   \
        if (item) {                   \
            item->action;             \
        }                             \
    }
#define REMOVE_ITEMS_OF_TYPE(container, type)                      \
    do {                                                           \
        QList<QGraphicsItem *> items = container->items();         \
        for (QGraphicsItem * item : items) {                       \
            if (type *specificItem = dynamic_cast<type *>(item)) { \
                container->removeItem(specificItem);               \
                delete specificItem;                               \
            }                                                      \
        }                                                          \
    } while (0)
#define SHOW_WIDGETS(...)                   \
    do {                                    \
        QWidget *widgets[] = {__VA_ARGS__}; \
        for (QWidget * widget : widgets) {  \
            if (widget)                     \
                widget->show();             \
        }                                   \
    } while (0)
#define HIDE_WIDGETS(...)                   \
    do {                                    \
        QWidget *widgets[] = {__VA_ARGS__}; \
        for (QWidget * widget : widgets) {  \
            if (widget)                     \
                widget->hide();             \
        }                                   \
    } while (0)
#define REMOVE_ALL_TABS(tabWidget)                          \
    do {                                                    \
        if (tabWidget) {                                    \
            while (tabWidget->count() > 0) {                \
                QWidget *tabContent = tabWidget->widget(0); \
                tabWidget->removeTab(0);                    \
                delete tabContent;                          \
            }                                               \
        }                                                   \
    } while (0)
#define UPDATE_LABEL(widget, condition, runningText, notRunningText)              \
    do {                                                                          \
        if (widget) {                                                             \
            widget->setText(condition ? runningText : notRunningText);            \
            widget->setStyleSheet(condition ? "color: green;" : "color: white;"); \
        } else {                                                                  \
            qWarning() << #widget " is null!";                                    \
        }                                                                         \
    } while (0)
#define ADD_WIDGETS(layout, ...)            \
    do {                                    \
        QWidget *widgets[] = {__VA_ARGS__}; \
        for (QWidget * widget : widgets) {  \
            layout->addWidget(widget);      \
        }                                   \
    } while (0)
#define IS_VALID_INDEX(index, container) ((index) >= 0 && (index) < (container).size())
#define ADD_IF_VALID_INDEX(index, container, list) \
    if (IS_VALID_INDEX(index, container)) {        \
        list.push_back(index);                     \
    }
#define REMOVE_NODES_CONNECTIONS(nodeIndex, connections, indicesToRemove, connectedNodes)        \
    for (int i = 0; i < connections.size(); ++i) {                                         \
        QPair<int, int> conn = connections[i];                                             \
        if (conn.first == nodeIndex || conn.second == nodeIndex) {                         \
            indicesToRemove.push_back(i);                                                  \
            int connectedNodeIndex = (conn.first == nodeIndex) ? conn.second : conn.first; \
            ADD_IF_VALID_INDEX(connectedNodeIndex, nodes, connectedNodes);                 \
        }                                                                                  \
    }
