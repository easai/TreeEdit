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
    void selectItem();
    void showContextMenu(const QPoint &pos);
    void newFile();
    void saveFile();
    void openFile();
    void selectFile();
    void parseFile(const QString& fileName);

private:
    Ui::MainWindow *ui;
    QTreeWidgetItem *m_pItem;
    QTreeWidgetItem *m_pEditItem=nullptr;
    Config m_config;

    void setTree(const QString& fileName);
    QJsonObject parseTree(QTreeWidgetItem *pRoot);
    void parseJSON(QTreeWidgetItem *pRoot, const QJsonArray& arr);
    void selectPath(QTreeWidgetItem *pItem);
    bool _selectPath(QTreeWidgetItem *pRoot, QTreeWidgetItem *pSelected);
    bool _clearPath(QTreeWidgetItem *pRoot);
    void closeEditItem();
    void setBold(QTreeWidgetItem *, bool isBold);

};
#endif // MAINWINDOW_H
