#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QInputDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->action_Quit, &QAction::triggered, this, &QApplication::quit);

    setTree();

    ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeWidget, &QTreeWidget::customContextMenuRequested, this,
            &MainWindow::showContextMenu);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::addItem()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("New Item"),
                                         tr("New Item:"), QLineEdit::Normal,
                                         "", &ok);
    if (ok && !text.isEmpty()){
        QTreeWidgetItem* newItem=new QTreeWidgetItem(m_pItem);
        newItem->setText(0, text);
    }
}

void MainWindow::deleteItem()
{
    QList<QTreeWidgetItem *>  items = ui->treeWidget->selectedItems();
    QTreeWidgetItem          *pp    = nullptr;

    if ( !items.isEmpty() )
    {
        foreach (QTreeWidgetItem *item, items)
        {
            pp = item->parent();
            if(pp!=nullptr){
                pp->removeChild(item);
            }
            delete item;
        }
    }
}

void MainWindow::showContextMenu(const QPoint &pos)
{
    QMenu *menu = new QMenu(this);

    QAction *addAction = new QAction("&Add Item");
    connect(addAction, &QAction::triggered, this, &MainWindow::addItem);
    menu->addAction(addAction);

    QAction *deleteAction = new QAction("&Delete Item");
    connect(deleteAction, &QAction::triggered, this, &MainWindow::deleteItem);
    menu->addAction(deleteAction);

    menu->popup(ui->treeWidget->viewport()->mapToGlobal(pos));
    m_pItem = ui->treeWidget->itemAt(pos);
}

void MainWindow::setTree()
{
    QList<QTreeWidgetItem *> items;
    for (int i = 0; i < 10; ++i){
        QTreeWidgetItem *item=new QTreeWidgetItem(static_cast<QTreeWidget *>(nullptr), QStringList(QString("item: %1").arg(i)));
        items.append(item);
    }
    ui->treeWidget->insertTopLevelItems(0, items);

}

