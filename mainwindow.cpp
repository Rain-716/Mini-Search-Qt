#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QHeaderView>
#include "mainwindow.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    QWidget* central=new QWidget(this);
    setCentralWidget(central);

    // Directory selector
    dirEdit=new QLineEdit;
    dirButton=new QPushButton("Browse...");
    connect(dirButton,&QPushButton::clicked,this,&MainWindow::selectDirectory);

    QHBoxLayout* dirLayout=new QHBoxLayout;
    dirLayout->addWidget(new QLabel("Directory:"));
    dirLayout->addWidget(dirEdit);
    dirLayout->addWidget(dirButton);

    // Query input
    queryEdit=new QLineEdit;
    queryEdit->setPlaceholderText("First line: number of queries, then each query per line");
    searchButton=new QPushButton("Search");
    connect(searchButton,&QPushButton::clicked,this,&MainWindow::performSearch);

    QHBoxLayout* queryLayout=new QHBoxLayout;
    queryLayout->addWidget(new QLabel("Query:"));
    queryLayout->addWidget(queryEdit);
    queryLayout->addWidget(searchButton);

    // Results
    resultView=new QTreeWidget;
    resultView->setColumnCount(1);
    resultView->setHeaderHidden(true);

    // Status
    statusLabel=new QLabel;

    QVBoxLayout* mainLayout=new QVBoxLayout(central);
    mainLayout->addLayout(dirLayout);
    mainLayout->addLayout(queryLayout);
    mainLayout->addWidget(resultView);
    mainLayout->addWidget(statusLabel);

    setWindowTitle("Mini Search Engine");
    resize(600,400);
}

MainWindow::~MainWindow() {}

void MainWindow::selectDirectory()
{
    QString dir=QFileDialog::getExistingDirectory(this,"Select Directory",dirEdit->text());
    if (!dir.isEmpty()){
        dirEdit->setText(dir);
        engine.loadDirectory(dir.toStdString());
        statusLabel->setText("Index built for: "+dir);
    }
}

void MainWindow::performSearch()
{
    resultView->clear();
    QString text=queryEdit->text().trimmed();
    if (text.isEmpty()){
        statusLabel->setText("Please enter queries.");
        return;
    }
    QStringList lines=text.split('\n',Qt::SkipEmptyParts);
    bool ok=false;
    int Q=lines[0].toInt(&ok);
    if (!ok||Q<=0||lines.size()<Q+1){
        statusLabel->setText("Invalid query format.");
        return;
    }
    for (int i=1;i<=Q;i++){
        QString q=lines[i].trimmed();
        auto results=engine.search(q.toStdString());
        QTreeWidgetItem* queryItem=new QTreeWidgetItem(resultView);
        queryItem->setText(0,QString::number(results.size()));
        if (results.empty()){
            QTreeWidgetItem* notFoundItem=new QTreeWidgetItem(queryItem);
            notFoundItem->setText(0,"Not Found");
        }
        else {
            for (const auto& res : results){
                QTreeWidgetItem* fileItem=new QTreeWidgetItem(queryItem);
                fileItem->setText(0,QString::fromStdString(res.filename));
                for (const auto& line : res.lines){
                    QTreeWidgetItem* lineItem=new QTreeWidgetItem(fileItem);
                    lineItem->setText(0,QString::fromStdString(line));
                }
            }
        }
    }
    resultView->expandAll();
    statusLabel->setText("Search completed.");
}