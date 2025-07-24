#pragma once

#include <QPlainTextEdit>
#include <QStringListModel>
#include <QCompleter>
#include <QWidget>

class LineNumberArea;

class CodeEditor : public QPlainTextEdit {
	Q_OBJECT

public:
	CodeEditor(QWidget* parent = nullptr);

	int lineNumberAreaWidth() const;
	void lineNumberAreaPaintEvent(QPaintEvent* event);

protected:
	void resizeEvent(QResizeEvent* event) override;
	void keyPressEvent(QKeyEvent* e) override;

private slots:
	void updateLineNumberAreaWidth(int newBlockCount);
	void highlightCurrentLine();
	void updateLineNumberArea(const QRect& rect, int dy);

private:
	QWidget* lineNumberArea;
	QStringListModel* model;
	QCompleter* completer;

	QString wordUnderCursor() const;
	QString contextUnderCursor();
	void insertCompletion(const QString& completion);
};
QStringListModel* CombineKeyword(QObject* parent = nullptr);