#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "aboutdialog.h"

#include <QColorDialog>
#include <QFileDialog>
#include <QFontDialog>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPushButton>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  connect(ui->action_New, &QAction::triggered, this, &MainWindow::newFile);
  connect(ui->action_Open, &QAction::triggered, this, &MainWindow::openFile);
  connect(ui->action_Save, &QAction::triggered, this, &MainWindow::saveFile);
  connect(ui->actionParse, &QAction::triggered, this, &MainWindow::selectFile);
  connect(ui->action_Expand_all, &QAction::triggered, this,
          &MainWindow::expandAll);
  connect(ui->action_Fold_all, &QAction::triggered, this, &MainWindow::foldAll);
  connect(ui->action_Nth_level, &QAction::triggered, this,
          &MainWindow::nthLevel);
  connect(ui->action_Set_font, &QAction::triggered, this, &MainWindow::setFont);
  connect(ui->action_About_TreeEdit, &QAction::triggered, this,
          &MainWindow::about);
  connect(ui->action_Quit, &QAction::triggered, this, &QApplication::quit);
  m_config.load();
  restoreGeometry(m_config.geom());
  ui->treeWidget->setFont(m_config.font());
  reload();
  setWindowIcon(QIcon("://images/treeedit-favicon.ico"));

  m_pEdit = new QLineEdit(ui->statusbar);
  ui->statusbar->addWidget(m_pEdit);

  QPushButton *pBtn = new QPushButton("Find", ui->statusbar);
  pBtn->setStyleSheet("font-size:9px;padding:1px 5px;");
  ui->statusbar->addWidget(pBtn);
  connect(pBtn, &QPushButton::clicked, this, &MainWindow::findItem);

  ui->treeWidget->setHeaderHidden(true);
  ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(ui->treeWidget, &QTreeWidget::customContextMenuRequested, this,
          &MainWindow::showContextMenu);
  connect(ui->treeWidget, &QTreeWidget::itemDoubleClicked, this,
          &MainWindow::updateItem);
  connect(ui->treeWidget, &QTreeWidget::itemSelectionChanged, this,
          &MainWindow::selectItem);
}

MainWindow::~MainWindow() {
  m_config.setFont(ui->treeWidget->font());
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

void MainWindow::selectItem() {
  QList<QTreeWidgetItem *> lst = ui->treeWidget->selectedItems();
  closeEditItem();
  if (0 < lst.size()) {
    selectPath(lst[0]);
  }
}

void MainWindow::colorItem() {
  QColor color = QColorDialog::getColor(Qt::white, this, tr("Select color"),
                                        QColorDialog::ShowAlphaChannel);
  if (color.isValid()) {
    m_colorTable.insert(m_pItem, color);
    refresh();
  }
}

void MainWindow::findItem() {
  refresh();
  QString str = m_pEdit->text();
  for (int i = 0; i < ui->treeWidget->topLevelItemCount(); i++) {
    QTreeWidgetItem *pRoot = ui->treeWidget->topLevelItem(i);
    if (pRoot->text(0).contains(str)) {
      pRoot->setBackground(0, Qt::lightGray);
      break;
    } else {
      if (_findItem(pRoot, str)) {
        pRoot->setExpanded(true);
      }
    }
  }
}

void MainWindow::closeEditItem() {
  if (m_pEditItem != nullptr) {
    ui->treeWidget->closePersistentEditor(m_pEditItem);
  }
}

void MainWindow::setBold(QTreeWidgetItem *pRoot, bool isBold) {
  QFont font = pRoot->font(0);
  if (isBold) {
    font.bold();
    pRoot->setBackground(0, Qt::lightGray);
  }
  pRoot->setFont(0, font);
}

void MainWindow::toggleAll(QTreeWidgetItem *pRoot, bool expand) {
  pRoot->setExpanded(expand);
  for (int i = 0; i < pRoot->childCount(); i++) {
    QTreeWidgetItem *pItem = pRoot->child(i);
    toggleAll(pItem, expand);
  }
}

void MainWindow::nthLevelExpand(QTreeWidgetItem *pRoot, int level, int target) {
  pRoot->setExpanded(level < target);
  level++;
  for (int i = 0; i < pRoot->childCount(); i++) {
    QTreeWidgetItem *pItem = pRoot->child(i);
    nthLevelExpand(pItem, level, target);
  }
}

void MainWindow::nthLevelFont(QTreeWidgetItem *pRoot, const QFont &font,
                              int target) {}

void MainWindow::reload() {
  QString fileName = m_config.fileName();
  if (!fileName.isEmpty()) {
    setTree(fileName);
  }
  m_colorTable.clear();
}

void MainWindow::showContextMenu(const QPoint &pos) {
  QMenu *menu = new QMenu(this);

  QAction *addAction = new QAction("&Add Item");
  connect(addAction, &QAction::triggered, this, &MainWindow::addItem);
  menu->addAction(addAction);

  QAction *deleteAction = new QAction("&Delete Item");
  connect(deleteAction, &QAction::triggered, this, &MainWindow::deleteItem);
  menu->addAction(deleteAction);

  QAction *colorAction = new QAction("&Set Color");
  connect(colorAction, &QAction::triggered, this, &MainWindow::colorItem);
  menu->addAction(colorAction);

  QAction *jsonAction = new QAction("&Add JSON");
  connect(jsonAction, &QAction::triggered, this, &MainWindow::openAddFile);
  menu->addAction(jsonAction);

  menu->popup(ui->treeWidget->viewport()->mapToGlobal(pos));
  m_pItem = ui->treeWidget->itemAt(pos);
}

void MainWindow::newFile() {
  m_pItem = nullptr;
  m_pEditItem = nullptr;
  ui->treeWidget->clear();
}

void MainWindow::setTree(const QString &fileName) {
  m_config.setFileName(fileName);
  QFile openFile(fileName);
  QFileInfo fileInfo(openFile.fileName());
  QString fn(fileInfo.fileName());
  setWindowTitle("TreeEdit - " + fn);

  openFile.open(QIODevice::ReadOnly);
  QByteArray data = openFile.readAll();
  QJsonDocument jsonDoc(QJsonDocument::fromJson(data));
  QJsonObject root(jsonDoc.object());
  QJsonArray jsonArr = root[JSONLIST].toArray();
  newFile();
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

// TreeWidget -> JSON
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

// JSON -> TreeWidget
void MainWindow::parseJSON(QTreeWidgetItem *pRoot, const QJsonArray &arr) {
  for (QJsonValueConstRef node : arr) {
    QJsonObject jsonObj = node.toObject();
    QString lang = jsonObj[JSONLANG].toString();
    QTreeWidgetItem *newItem = new QTreeWidgetItem(pRoot);
    newItem->setText(0, lang);
    parseJSON(newItem, jsonObj[JSONLIST].toArray());
  }
}

// TreeWidget -> JSON
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
    if (fileName.endsWith(".json")) {
      fileName += ".json";
    }
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

void MainWindow::openAddFile() {
  QString selFilter = tr("JSON Documents(*.json)");
  QString fileName = QFileDialog::getOpenFileName(
      this, tr("Open language list"), ".",
      tr("JSON Documents(*.json);;All(*.*)"), &selFilter,
      QFileDialog::DontUseCustomDirectoryIcons);
  if (!fileName.isEmpty()) {
    addTree(m_pItem, fileName);
  }
}

void MainWindow::addTree(QTreeWidgetItem *pItem, const QString &fileName) {
  m_config.setFileName(fileName);
  QFile openFile(fileName);
  openFile.open(QIODevice::ReadOnly);
  QByteArray data = openFile.readAll();
  QJsonDocument jsonDoc(QJsonDocument::fromJson(data));
  QJsonObject root(jsonDoc.object());
  QJsonArray jsonArr = root[JSONLIST].toArray();
  for (QJsonValueRef node : jsonArr) {
    QJsonObject jsonObj = node.toObject();
    QString lang = jsonObj[JSONLANG].toString();
    QJsonArray arr = jsonObj[JSONLIST].toArray();
    parseJSON(pItem, arr);
  }
}

bool MainWindow::_findItem(QTreeWidgetItem *pRoot, QString str) {
  for (int i = 0; i < pRoot->childCount(); i++) {
    QTreeWidgetItem *pItem = pRoot->child(i);
    if (pItem->text(0).contains(str)) {
      pItem->setBackground(0, Qt::lightGray);
      pItem->setExpanded(true);
      return true;
    } else {
      if (_findItem(pItem, str)) {
        pItem->setExpanded(true);
        return true;
      }
    }
  }
  return false;
}

void MainWindow::selectFile() {
  QString selFilter = tr("Text Documents(*.txt)");
  QString fileName = QFileDialog::getOpenFileName(
      this, tr("Open language list"), ".",
      tr("Text Documents(*.txt);;All(*.*)"), &selFilter,
      QFileDialog::DontUseCustomDirectoryIcons);
  if (!fileName.isEmpty()) {
    parseFile(fileName);
  }
}

void MainWindow::parseFile(const QString &fileName) {
  QFile openFile(fileName);
  QFileInfo fileInfo(openFile.fileName());
  QString fn(fileInfo.fileName());
  setWindowTitle("TreeEdit - " + fn);
  newFile();
  if (openFile.open(QIODevice::ReadOnly)) {
    char buf[1024];
    QList<QTreeWidgetItem *> nodeList;
    int level = 0;
    QStringList lst = {fileName};
    QTreeWidgetItem *item =
        new QTreeWidgetItem(static_cast<QTreeWidget *>(nullptr), lst);
    ui->treeWidget->addTopLevelItem(item);
    nodeList.append(item);
    while (openFile.readLine(buf, sizeof(buf)) != -1) {
      int cnt = 0;
      for (int i = 0; i < 1024; i++) {
        if (buf[i] != '*') {
          break;
        } else {
          cnt++;
        }
      }
      char title[1024];
      int idx = 0;
      for (int i = 0; i < 1024; i++) {
        if (buf[i] != '*' && buf[i] != '\n') {
          title[idx++] = buf[i];
        }
        if (buf[i] == '\0') {
          break;
        }
      }
      if (1 <= cnt) {
        level = cnt - 1;

        QTreeWidgetItem *pItem = new QTreeWidgetItem(nodeList[level]);
        pItem->setText(0, title);

        if (level + 1 <= nodeList.size() - 1) {
          nodeList[level + 1] = pItem;
        } else {
          nodeList.append(pItem);
        }
      }
    }
  }
}

void MainWindow::expandAll() {
  for (int i = 0; i < ui->treeWidget->topLevelItemCount(); i++) {
    QTreeWidgetItem *pRoot = ui->treeWidget->topLevelItem(i);
    toggleAll(pRoot, true);
  }
}

void MainWindow::foldAll() {
  for (int i = 0; i < ui->treeWidget->topLevelItemCount(); i++) {
    QTreeWidgetItem *pRoot = ui->treeWidget->topLevelItem(i);
    toggleAll(pRoot, false);
  }
}

void MainWindow::nthLevel() {
  bool ok;
  int level = QInputDialog::getInt(this, tr("Nth level expand"), tr("Level:"),
                                   1, 0, 100, 1, &ok);
  if (ok) {
    for (int i = 0; i < ui->treeWidget->topLevelItemCount(); i++) {
      QTreeWidgetItem *pRoot = ui->treeWidget->topLevelItem(i);
      nthLevelExpand(pRoot, 1, level);
    }
  }
}

void MainWindow::setFont() {
  bool ok;
  QFont font = QFontDialog::getFont(&ok, ui->treeWidget->font(), this);
  if (ok) {
    ui->treeWidget->setFont(font);
  }
}

bool MainWindow::_selectPath(QTreeWidgetItem *pRoot,
                             QTreeWidgetItem *pSelected) {
  bool res = false;
  if (pRoot == pSelected) {
    res = true;
  } else {
    for (int i = 0; i < pRoot->childCount(); i++) {
      QTreeWidgetItem *pItem = pRoot->child(i);
      if (pItem == pSelected) {
        res = true;
        setBold(pItem, res);
        break;
      } else {
        res = _selectPath(pItem, pSelected);
        setBold(pItem, res);
        if (res)
          break;
      }
    }
    setBold(pRoot, res);
  }
  return res;
}

void MainWindow::_refresh(QTreeWidgetItem *pRoot) {
  for (int i = 0; i < pRoot->childCount(); i++) {
    QTreeWidgetItem *pItem = pRoot->child(i);
    QColor color = m_colorTable.value(pItem);
    if (color != nullptr) {
      pItem->setBackground(0, color);
    } else {
      pItem->setBackground(0, Qt::white);
    }
    _refresh(pItem);
  }
}

void MainWindow::refresh() {
  for (int i = 0; i < ui->treeWidget->topLevelItemCount(); i++) {
    QTreeWidgetItem *pRoot = ui->treeWidget->topLevelItem(i);
    QColor color = m_colorTable.value(pRoot);
    if (color != nullptr) {
      pRoot->setBackground(0, color);
    } else {
      pRoot->setBackground(0, Qt::white);
    }
    _refresh(pRoot);
  }
}

void MainWindow::selectPath(QTreeWidgetItem *pItem) {
  refresh();
  for (int i = 0; i < ui->treeWidget->topLevelItemCount(); i++) {
    QTreeWidgetItem *pRoot = ui->treeWidget->topLevelItem(i);
    bool res = _selectPath(pRoot, pItem);
    setBold(pRoot, res);
  }
}

void MainWindow::about() {
  AboutDialog *dlg = new AboutDialog(this);
  dlg->exec();
}
