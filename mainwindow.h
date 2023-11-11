#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define JSONLANG "lang"
#define JSONLIST "lang-list"

#include "config.h"
#include <QMainWindow>
#include <QTreeWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void addItem();
    void deleteItem();
    void updateItem(QTreeWidgetItem* pItem, int col);
    void closeEditItem();
    void showContextMenu(const QPoint &pos);
    void newFile();
    void saveFile();
    void openFile();

private:
    Ui::MainWindow *ui;
    QTreeWidgetItem *m_pItem;
    QTreeWidgetItem *m_pEditItem=nullptr;
    Config m_config;

    void setTree(const QString& fileName);
    QJsonObject parseTree(QTreeWidgetItem *pRoot);
    void parseJSON(QTreeWidgetItem *pRoot, const QJsonArray& arr);
};
#endif // MAINWINDOW_H
