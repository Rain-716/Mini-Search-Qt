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
        void performSearch();

    private:
        QLineEdit* dirEdit;
        QPushButton* dirButton;
        QLineEdit* queryEdit;
        QPushButton* searchButton;
        QTreeWidget* resultView;
        QLabel* statusLabel;

        SearchEngine engine;
};

#endif // MAINWINDOW_H