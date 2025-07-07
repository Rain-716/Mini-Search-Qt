#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QTreeWidget>
#include <QLabel>
#include "SearchEngine.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
    public:
        MainWindow(QWidget* parent=nullptr);
        ~MainWindow();

    private slots:
        void selectDirectory();
        void performSearch(); // 此函数将从文件中读取查询

    private:
        QLineEdit* dirEdit;
        QPushButton* dirButton;
        QLineEdit* queryEdit; // 依然保留，但不再用于用户输入查询
        QPushButton* searchButton;
        QTreeWidget* resultView;
        QLabel* statusLabel;

        SearchEngine engine;
};

#endif // MAINWINDOW_H