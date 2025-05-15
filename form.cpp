#include "form.h"
#include "ui_form.h"
#include"mainwindow.h"
#include <QFileDialog>
#include <QDebug>
#include <QInputDialog>
#include <QDir>
#include<QWidget>
#include <iostream>
#include <QMessageBox>

Form::Form(QWidget *parent)
        : QWidget(parent), ui(new Ui::Form) {
    ui->setupUi(this);
}

Form::~Form() {
    delete ui;
}


void Form::on_toolButton_clicked() {
    QString folderPath = QFileDialog::getExistingDirectory(
            this,
            tr("Select Folder"),
            QDir::homePath(), // 默认路径（如用户主目录）
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );


    if (!folderPath.isEmpty()) {
        qDebug() << "Selected folder:" << folderPath;
        if (validate(folderPath) == 2) {
            this->setVisible(false);
            // 在这里处理新创建的文件夹路径
            MainWindow *m = new MainWindow(folderPath, false);
            m->show();
        } else {
            QMessageBox::information(this,
                                     "打开错误",
                                     "打开的目录对象不是合法的GuoShuBase数据库");
        }

    }

}


void Form::on_toolButton_2_clicked() {
    QString parentFolder = QFileDialog::getExistingDirectory(
            this,
            tr("Select Parent Folder"),
            QDir::homePath()
    );

    if (!parentFolder.isEmpty()) {
        bool ok;
        QString folderName = QInputDialog::getText(
                this,
                tr("New Folder Name"),
                tr("Enter folder name:"),
                QLineEdit::Normal,
                "",
                &ok
        );

        if (ok && !folderName.isEmpty()) {
            QString newFolderPath = parentFolder + "/" + folderName;
            std::cout << newFolderPath.toStdString() << std::endl;

            if (QDir().mkdir(newFolderPath)) {
                std::cout << "Folder created:" << newFolderPath.toStdString();
                this->setVisible(false);
                // 在这里处理新创建的文件夹路径
                MainWindow *m = new MainWindow(newFolderPath, true);
                m->show();

            } else {
                qDebug() << "Failed to create folder!";
            }
        }
    }

}

int Form::validate(const QString &folderPath) {
    int flag = 0;
    QDir dir(folderPath);
    // 获取文件夹中的所有文件（不包括子文件夹）
    QStringList files = dir.entryList(QDir::Files);

    for (const QString &file: files) {
        if ((file.compare("relcat") == 0) ||
            (file.compare("attrcat") == 0)
                )
            flag++;
    }

    return flag;
}