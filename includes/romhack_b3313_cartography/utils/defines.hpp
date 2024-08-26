#pragma once
#define OR ||
#define AND &&
#define REPA(type, container, action) \
    for (type * item : container) {   \
        if (item) {                   \
            item->action;             \
        }                             \
    }
#define REPA2(type, container, action) \
    for (type * item : container) {    \
        if (item) {                    \
            action;                    \
        }                              \
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
#define REMOVE_ITEMS_OF_TYPE_WITH_VERIF(container, type, condition) \
    do {                                                            \
        QList<QGraphicsItem *> items = container->items();          \
        for (QGraphicsItem * item : items) {                        \
            if (type *specificItem = dynamic_cast<type *>(item)) {  \
                if (condition(specificItem)) {                      \
                    container->removeItem(specificItem);            \
                    delete specificItem;                            \
                }                                                   \
            }                                                       \
        }                                                           \
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
#define IS_VALID_INDEX(index, container) ((index) >= 0 && (index) < (container).size())
#define ADD_IF_VALID_INDEX(index, container, list) \
    if (IS_VALID_INDEX(index, container)) {        \
        list.push_back(index);                     \
    }
#define REMOVE_NODES_CONNECTIONS(nodeIndex, connections, indicesToRemove, connectedNodes)  \
    for (int i = 0; i < connections.size(); ++i) {                                         \
        QPair<int, int> conn = connections[i];                                             \
        if (conn.first == nodeIndex || conn.second == nodeIndex) {                         \
            indicesToRemove.push_back(i);                                                  \
            int connectedNodeIndex = (conn.first == nodeIndex) ? conn.second : conn.first; \
            ADD_IF_VALID_INDEX(connectedNodeIndex, nodes, connectedNodes);                 \
        }                                                                                  \
    }
#define ADD_ACTION(contextMenu, actionName, method)                      \
    QAction *actionName = new QAction(#actionName, this);                \
    connect(actionName, &QAction::triggered, this, &MainWindow::method); \
    contextMenu->addAction(actionName);
#define STOP_AND_WAIT_THREAD(thread) \
    if (thread) {                    \
        thread->stop();              \
        thread->wait();              \
    }
#define CHECK_JSON_ARRAY(obj, key)                                        \
    if (!obj.contains(key) || !obj[key].isArray()) {                      \
        qWarning() << "jsonData does not contain valid '" key "' array."; \
        return courseNames;                                               \
    }
#define SHOW_MESSAGE(text)                      \
    QMessageBox msgBox;                         \
    msgBox.setIcon(QMessageBox::Information);   \
    msgBox.setText(text);                       \
    msgBox.setWindowTitle("Updater");           \
    msgBox.setStandardButtons(QMessageBox::Ok); \
    msgBox.exec()