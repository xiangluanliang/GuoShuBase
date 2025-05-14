#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include<cstring>
#include<cstdio>
#include <QInputDialog>
#include <QDir>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QTreeView>
#include <QList>
#include <QString>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->treeWidget_dbBrowser->setRootIsDecorated(true);

}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::displayListInTreeView(QTreeView* treeView, const QList<QString>& stringList)
{
    // 1. 创建标准项模型
    QStandardItemModel* model = new QStandardItemModel(treeView);

    // 2. 设置表头（可选）
    model->setHorizontalHeaderLabels(QStringList() << "表");

    // 3. 添加数据到模型
    for (const QString& str : stringList) {
        QStandardItem* item = new QStandardItem(str);
        model->appendRow(item);
    }

    // 4. 设置模型到树视图
    treeView->setModel(model);

    // 5. 展开所有项（可选）
    treeView->expandAll();

    // 6. 调整列宽以适应内容（可选）
    treeView->resizeColumnToContents(0);
}

MainWindow::MainWindow(const QString &folderPath,QWidget *parent)
    : QMainWindow(parent)
    ,ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->treeWidget_dbBrowser->setRootIsDecorated(true);
    QList<QString> filesWithoutExtension = MgetFilesWithoutExtension(folderPath);
    displayListInTreeView(ui->treeWidget_dbBrowser,filesWithoutExtension);
    dbsname=folderPath;

}

QList<QString> MainWindow::MgetFilesWithoutExtension(const QString &folderPath) {
    QList<QString> filesWithoutExtension;
    QDir dir(folderPath);

    // 获取文件夹中的所有文件（不包括子文件夹）
    QStringList files = dir.entryList(QDir::Files);

    for (const QString &file : files) {
        QFileInfo fileInfo(dir, file);
        if (fileInfo.suffix().isEmpty()) {
            // 如果文件无后缀，则添加到列表中
            filesWithoutExtension.append(fileInfo.fileName());
        }
    }

    return filesWithoutExtension;
}




void MainWindow::on_DOIT_clicked()
{
    //TODO将plainTextEdit中的字符转到命令行内并运行，检测到有查询语句时进入新界面并展示查询结果

    QString sqls=ui->plainTextEdit->toPlainText();
    QByteArray ba = sqls.toLatin1();
    char* sqlss=ba.data();
    char sexit[]="exit";
    if(std::strcmp(sqlss,sexit)){
        exit(1);
    }else {
        ;//std::list sqla;//=GBparsesql(pfm, smm, qlm,sqlss);
    }
    QStringList qtList;

//    for (const std::string &item : sqla) {
//        qtList.append(QString::fromStdString(item));
//    }//将list转化为qtlist

    // 合并为字符串
    QString result = qtList.join("\n");

    // 输出到 QPlainTextEdit
    ui->plainTextEdit->appendPlainText(result);
    displayListInTreeView(ui->treeWidget_dbBrowser,MgetFilesWithoutExtension(dbsname));


}


void MainWindow::on_save_clicked()
{
    //用于保存
}

