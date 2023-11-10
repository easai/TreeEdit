#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  connect(ui->action_Open, &QAction::triggered, this, &MainWindow::openFile);
  connect(ui->action_Save, &QAction::triggered, this, &MainWindow::saveFile);
  connect(ui->action_Quit, &QAction::triggered, this, &QApplication::quit);

  setTree();

  ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(ui->treeWidget, &QTreeWidget::customContextMenuRequested, this,
          &MainWindow::showContextMenu);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::addItem() {
  bool ok;
  QString text = QInputDialog::getText(this, tr("New Item"), tr("New Item:"),
                                       QLineEdit::Normal, "", &ok);
  if (ok && !text.isEmpty()) {
    QTreeWidgetItem *newItem = new QTreeWidgetItem(m_pItem);
    newItem->setText(0, text);
  }
}

void MainWindow::deleteItem() {
  QList<QTreeWidgetItem *> items = ui->treeWidget->selectedItems();
  QTreeWidgetItem *pp = nullptr;

  if (!items.isEmpty()) {
    foreach (QTreeWidgetItem *item, items) {
      pp = item->parent();
      if (pp != nullptr) {
        pp->removeChild(item);
      }
      delete item;
    }
  }
}

void MainWindow::showContextMenu(const QPoint &pos) {
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

void MainWindow::setTree() {
  QList<QTreeWidgetItem *> items;
  for (int i = 0; i < 10; ++i) {
    QTreeWidgetItem *item =
        new QTreeWidgetItem(static_cast<QTreeWidget *>(nullptr),
                            QStringList(QString("item: %1").arg(i)));
    items.append(item);
  }
  ui->treeWidget->insertTopLevelItems(0, items);
}

QJsonObject MainWindow::parseTree(QTreeWidgetItem *pRoot) {
  QJsonObject jsonObj;
  QJsonArray jsonArr;
  jsonObj[JSONLANG] = pRoot->text(0);
  for (int i = 0; i < pRoot->childCount(); i++) {
    QTreeWidgetItem *pItem = pRoot->child(i);
    QJsonObject obj = parseTree(pItem);
    jsonArr.append(obj);
  }
  jsonObj[JSONLIST] = jsonArr;
  return jsonObj;
}

void MainWindow::parseJSON(QTreeWidgetItem *pRoot, const QJsonArray &arr)
{
  for (QJsonValueConstRef node : arr) {
    QJsonObject jsonObj = node.toObject();
    QString lang = jsonObj[JSONLANG].toString();
    QTreeWidgetItem *newItem = new QTreeWidgetItem(pRoot);
    newItem->setText(0, lang);
    parseJSON(newItem, jsonObj[JSONLIST].toArray());
  }
}

void MainWindow::saveFile() {
  QJsonObject root;
  QJsonArray jsonArr;
  for (int i = 0; i < ui->treeWidget->topLevelItemCount(); i++) {
    QTreeWidgetItem *pRoot = ui->treeWidget->topLevelItem(i);
    QJsonObject jsonObj = parseTree(pRoot);
    jsonArr.append(jsonObj);
  }
  root[JSONLIST] = jsonArr;
  QJsonDocument jsonDoc(root);
  QByteArray data(jsonDoc.toJson());

  QString selFilter;
  QString fileName = QFileDialog::getSaveFileName(
      this, tr("Save"), ".", tr("JSON documents(*.json)"), &selFilter,
      QFileDialog::DontUseCustomDirectoryIcons);
  if (!fileName.isEmpty()) {
    QFile saveFile(fileName);
    saveFile.open(QIODevice::WriteOnly);
    saveFile.write(data);
    saveFile.close();
  }
}

void MainWindow::openFile() {
  QString selFilter = tr("JSON Documents(*.json)");
  QString fileName = QFileDialog::getOpenFileName(
      this, tr("Open language list"), ".",
      tr("JSON Documents(*.json);;All(*.*)"), &selFilter,
      QFileDialog::DontUseCustomDirectoryIcons);
  if (!fileName.isEmpty()) {
    QFile openFile(fileName);
    openFile.open(QIODevice::ReadOnly);
    QByteArray data = openFile.readAll();
    QJsonDocument jsonDoc(QJsonDocument::fromJson(data));
    QJsonObject root(jsonDoc.object());
    QJsonArray jsonArr = root[JSONLIST].toArray();
    ui->treeWidget->clear();
    for (QJsonValueRef node : jsonArr) {
      QJsonObject jsonObj = node.toObject();
      QString lang = jsonObj[JSONLANG].toString();
      QJsonArray arr=jsonObj[JSONLIST].toArray();
      QStringList lst = {lang};
      QTreeWidgetItem *item =
          new QTreeWidgetItem(static_cast<QTreeWidget *>(nullptr), lst);
      ui->treeWidget->addTopLevelItem(item);
      parseJSON(item,arr);
    }
  }
}
