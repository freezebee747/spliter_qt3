#pragma once
#include "syntaxhighlighter.h"
#include "codeeditor.h"
#include <QMainWindow>
#include <QPlainTextEdit>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QString>
#include <QTimer>

class MainWindow : public QMainWindow {
	Q_OBJECT;

public:
	MainWindow(QWidget* parent = nullptr);
	~MainWindow();

private slots:

	void exitApp();
	void parsingText();
private:
	ASTHighlighter* highlighter;
	CodeEditor* textEdit;
	QTimer* parseTimer;
	QString currentFile;

	void createActions();
	void Analyze();
	void loadFile(const QString& fileName);

	QAction* exitAct;
	ASTNodeList ASTList;

	Parser parser;
	SyntaxChecker syntaxChecker;

	bool analyzing = false;

};