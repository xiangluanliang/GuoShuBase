#include "form.h"
#include "ui_form.h"
#include"mainwindow.h"
#include <QFileDialog>
#include <QDebug>
#include <QInputDialog>
#include <QDir>
#include<QWidget>
#include <iostream>

Form::Form(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Form)
{
    ui->setupUi(this);
}

Form::~Form()
{
    delete ui;
}

QList<QString> Form::getFilesWithoutExtension(const QString &folderPath) {
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

void Form::on_toolButton_clicked()
{
    QString folderPath = QFileDialog::getExistingDirectory(
        this,
        tr("Select Folder"),
        QDir::homePath(), // 默认路径（如用户主目录）
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
        );


    if (!folderPath.isEmpty()) {
        qDebug() << "Selected folder:" << folderPath;
        this->setVisible(false);
        std::cout<< folderPath.toStdString() <<std::endl;

        MainWindow* m =new MainWindow(folderPath);
        m->show();

    }

}


void Form::on_toolButton_2_clicked()
{
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
            std::cout<< newFolderPath.toStdString() <<std::endl;
            
            if (QDir().mkdir(newFolderPath)) {
                std::cout << "Folder created:" << newFolderPath.toStdString();
                this->setVisible(false);
                std::cout<< "success" <<std::endl;
                
                // 在这里处理新创建的文件夹路径
                MainWindow* m = new MainWindow(newFolderPath);
                m->show();
                std::cout<< "success111" <<std::endl;
                
            } else {
                qDebug() << "Failed to create folder!";
            }
        }
    }

}

