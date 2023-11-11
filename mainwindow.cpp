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
  m_config.load();
  restoreGeometry(m_config.geom());
  QString fileName = m_config.fileName();
  if (!fileName.isEmpty()) {
    setTree(m_config.fileName());
  }
  setWindowIcon(QIcon("://images/treeedit-favicon.ico"));

  ui->treeWidget->setHeaderHidden(true);
  ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(ui->treeWidget, &QTreeWidget::customContextMenuRequested, this,
          &MainWindow::showContextMenu);
  connect(ui->treeWidget, &QTreeWidget::itemDoubleClicked, this,
          &MainWindow::updateItem);
  connect(ui->treeWidget, &QTreeWidget::itemSelectionChanged, this,
          &MainWindow::closeEditItem);
}

MainWindow::~MainWindow() {
  m_config.setGeom(saveGeometry());
  m_config.save();
  delete ui;
}

void MainWindow::addItem() {
  bool ok;
  QString text = QInputDialog::getText(this, tr("New Item"), tr("New Item:"),
                                       QLineEdit::Normal, "", &ok);
  if (ok && !text.isEmpty()) {
    if (m_pItem == nullptr) {
      QStringList lst = {text};
      QTreeWidgetItem *item =
          new QTreeWidgetItem(static_cast<QTreeWidget *>(nullptr), lst);
      item->setFlags(item->flags() & Qt::ItemIsEditable);
      ui->treeWidget->addTopLevelItem(item);
    } else {
      QTreeWidgetItem *newItem = new QTreeWidgetItem(m_pItem);
      newItem->setFlags(newItem->flags() & Qt::ItemIsEditable);
      newItem->setText(0, text);
      m_pItem->setExpanded(true);
    }
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

void MainWindow::updateItem(QTreeWidgetItem *pItem, int col) {
  m_pEditItem = pItem;
  ui->treeWidget->openPersistentEditor(pItem);
  ui->treeWidget->editItem(pItem, col);
}

void MainWindow::closeEditItem() {
  if (m_pEditItem != nullptr) {
    ui->treeWidget->closePersistentEditor(m_pEditItem);
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

void MainWindow::setTree(const QString &fileName) {
  m_config.setFileName(fileName);
  setWindowTitle("TreeEdit - " + fileName);
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
    QJsonArray arr = jsonObj[JSONLIST].toArray();
    QStringList lst = {lang};
    QTreeWidgetItem *item =
        new QTreeWidgetItem(static_cast<QTreeWidget *>(nullptr), lst);
    ui->treeWidget->addTopLevelItem(item);
    parseJSON(item, arr);
  }
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

void MainWindow::parseJSON(QTreeWidgetItem *pRoot, const QJsonArray &arr) {
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
    setTree(fileName);
  }
}
