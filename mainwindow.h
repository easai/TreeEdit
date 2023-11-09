#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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
    void showContextMenu(const QPoint &pos);

private:
    Ui::MainWindow *ui;
    QTreeWidgetItem *m_pItem;

    void setTree();

};
#endif // MAINWINDOW_H