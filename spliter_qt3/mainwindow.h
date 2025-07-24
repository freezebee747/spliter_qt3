#pragma once
#include "syntaxhighlighter.h"
#include "codeeditor.h"
#include <QMainWindow>
#include <QPlainTextEdit>
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

	void createActions();
	void Analyze();

	QAction* exitAct;
	ASTNodeList ASTList;

	Parser parser;
	SyntaxChecker syntaxChecker;
};