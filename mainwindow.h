#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include<cstring>
#include<cstdio>
#include <QInputDialog>
#include <QDir>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QTreeView>
#include <QList>
#include <QString>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    MainWindow(const QString &folderPath,QWidget *parent=nullptr);

    void displayListInTreeView(QTreeView* treeView, const QList<QString>& stringList);

    QList<QString> MgetFilesWithoutExtension(const QString &folderPath);


    QString dbpath;
    QString dbname;

private slots:

    void on_DOIT_clicked();

    void on_save_clicked();

private:
    Ui::MainWindow *ui;


};
#endif // MAINWINDOW_H
