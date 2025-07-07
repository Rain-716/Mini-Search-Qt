#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QHeaderView>
#include <QMessageBox> // For showing file open errors
#include <fstream>     // For file operations
#include <QTextStream> // For reading text file
#include "mainwindow.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    QWidget* central = new QWidget(this);
    setCentralWidget(central);

    // Directory selector
    dirEdit = new QLineEdit;
    dirButton = new QPushButton("Browse...");
    connect(dirButton, &QPushButton::clicked, this, &MainWindow::selectDirectory);

    QHBoxLayout* dirLayout = new QHBoxLayout;
    dirLayout->addWidget(new QLabel("Directory:"));
    dirLayout->addWidget(dirEdit);
    dirLayout->addWidget(dirButton);

    // Query input (now primarily for display/placeholder, not direct input for file-based search)
    queryEdit = new QLineEdit;
    queryEdit->setPlaceholderText("Queries will be read from search_queries_100k.txt");
    queryEdit->setReadOnly(true); // Make it read-only
    searchButton = new QPushButton("Perform File Search"); // Changed button text
    connect(searchButton, &QPushButton::clicked, this, &MainWindow::performSearch);

    QHBoxLayout* queryLayout = new QHBoxLayout;
    queryLayout->addWidget(new QLabel("Query Source:"));
    queryLayout->addWidget(queryEdit);
    queryLayout->addWidget(searchButton);

    // Results
    resultView = new QTreeWidget;
    resultView->setColumnCount(1);
    resultView->setHeaderHidden(true);

    // Status
    statusLabel = new QLabel;

    QVBoxLayout* mainLayout = new QVBoxLayout(central);
    mainLayout->addLayout(dirLayout);
    mainLayout->addLayout(queryLayout);
    mainLayout->addWidget(resultView);
    mainLayout->addWidget(statusLabel);

    setWindowTitle("Mini Search Engine (File-based Queries)");
    resize(700, 500); // Slightly larger window
}

MainWindow::~MainWindow() {}

void MainWindow::selectDirectory()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select Directory", dirEdit->text());
    if (!dir.isEmpty()) {
        dirEdit->setText(dir);
        statusLabel->setText("Building index for: " + dir + "...");
        engine.loadDirectory(dir.toStdString());
        statusLabel->setText("Index built for: " + dir + ". Ready to search from file.");
    }
}

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QHeaderView>
#include <QMessageBox> // For showing file open errors
#include <fstream>     // For file operations
// #include <QTextStream> // No longer needed for this approach
#include "mainwindow.h"

// ... (other parts of MainWindow.cpp)

void MainWindow::performSearch()
{
    resultView->clear();
    QString queryFilePath = "search_queries_100k.txt"; // 指定查询文件路径
    std::ifstream queryFile(queryFilePath.toStdString());

    if (!queryFile.is_open()) {
        QMessageBox::critical(this, "Error", "Could not open search_queries_100k.txt. Make sure it's in the same directory as the executable.");
        statusLabel->setText("Error: Could not open " + queryFilePath);
        return;
    }

    std::string firstLineStr;
    std::getline(queryFile, firstLineStr); // Use std::getline for the first line

    QString firstLine = QString::fromStdString(firstLineStr).trimmed(); // Convert to QString and trim
    bool ok = false;
    int Q = firstLine.toInt(&ok);

    if (!ok || Q <= 0) {
        QMessageBox::critical(this, "Error", "Invalid query count in search_queries_100k.txt. First line must be a positive integer.");
        statusLabel->setText("Error: Invalid query count in " + queryFilePath);
        queryFile.close();
        return;
    }

    statusLabel->setText(QString("Performing %1 queries from %2...").arg(Q).arg(queryFilePath));

    std::string lineStr;
    for (int i = 0; i < Q; ++i) {
        if (!std::getline(queryFile, lineStr)) { // Use std::getline to read each query line
            QMessageBox::warning(this, "Warning", QString("Expected %1 queries but found only %2. Stopping search.").arg(Q).arg(i));
            break;
        }
        QString q = QString::fromStdString(lineStr).trimmed(); // Convert to QString and trim
        if (q.isEmpty()) {
            // Skip empty lines if any, or handle as an error if strict
            continue;
        }

        auto results = engine.search(q.toStdString());

        // Display results for each query in the QTreeWidget
        QTreeWidgetItem* queryRootItem = new QTreeWidgetItem(resultView);
        queryRootItem->setText(0, "Query: \"" + q + "\" - Found " + QString::number(results.size()) + " files");

        if (results.empty()) {
            QTreeWidgetItem* notFoundItem = new QTreeWidgetItem(queryRootItem);
            notFoundItem->setText(0, "Not Found");
        } else {
            for (const auto& res : results) {
                QTreeWidgetItem* fileItem = new QTreeWidgetItem(queryRootItem);
                fileItem->setText(0, QString::fromStdString(res.filename) + " (Occurrences: " + QString::number(res.count) + ")");
                for (const auto& line : res.lines) {
                    QTreeWidgetItem* lineItem = new QTreeWidgetItem(fileItem);
                    lineItem->setText(0, QString::fromStdString(line));
                }
            }
        }
    }
    queryFile.close();
    resultView->expandAll();
    statusLabel->setText("Search completed for all queries from " + queryFilePath);
}
