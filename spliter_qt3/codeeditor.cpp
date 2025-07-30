#include "codeeditor.h"
#include "keyword.h"
#include <QPainter>
#include <QTextBlock>
#include <QAbstractItemView>
#include <QScrollBar>

QString customWordUnderCursor(QPlainTextEdit* edit){
	QTextCursor cursor = edit->textCursor();
	int posInBlock = cursor.positionInBlock();
	QString blockText = cursor.block().text();

	// 왼쪽 확장
	int start = posInBlock;
	while (start > 0 && !blockText[start - 1].isSpace()) {
		start--;
	}

	// 오른쪽 확장
	int end = posInBlock;
	while (end < blockText.length() && !blockText[end].isSpace()) {
		end++;
	}

	return blockText.mid(start, end - start);
}

void customRemoveSelectedText(QPlainTextEdit* edit){
	if (!edit) return;

	QTextCursor cursor = edit->textCursor();
	int posInBlock = cursor.positionInBlock();
	QString blockText = cursor.block().text();

	int start = posInBlock;
	while (start > 0 && !blockText[start - 1].isSpace())
		start--;

	int end = posInBlock;
	while (end < blockText.length() && !blockText[end].isSpace())
		end++;

	QTextCursor tempCursor = cursor;
	tempCursor.setPosition(cursor.block().position() + start);
	tempCursor.setPosition(cursor.block().position() + end, QTextCursor::KeepAnchor);

	tempCursor.removeSelectedText();
}

class LineNumberArea : public QWidget {
public:
	LineNumberArea(CodeEditor* editor) : QWidget(editor), codeEditor(editor) {}
	QSize sizeHint() const override { return QSize(codeEditor->lineNumberAreaWidth(), 0); }

protected:
	void paintEvent(QPaintEvent* event) override {
		codeEditor->lineNumberAreaPaintEvent(event);
	}

private:
	CodeEditor* codeEditor;
};

CodeEditor::CodeEditor(QWidget* parent) : QPlainTextEdit(parent) {
	lineNumberArea = new LineNumberArea(this);
	model = CombineKeyword(this);
	completer = new QCompleter(model, this);

	completer->setWidget(this);
	completer->setCompletionMode(QCompleter::PopupCompletion);
	completer->setCaseSensitivity(Qt::CaseInsensitive);

	connect(completer, QOverload<const QString&>::of(&QCompleter::activated),
		this, &CodeEditor::insertCompletion);

	connect(this, &CodeEditor::blockCountChanged, this, &CodeEditor::updateLineNumberAreaWidth);
	connect(this, &CodeEditor::updateRequest, this, &CodeEditor::updateLineNumberArea);
	connect(this, &CodeEditor::cursorPositionChanged, this, &CodeEditor::highlightCurrentLine);

	updateLineNumberAreaWidth(0);
	highlightCurrentLine();
}

int CodeEditor::lineNumberAreaWidth() const {
	int digits = 1;
	int max = qMax(1, blockCount());

	// 최소 자릿수 강제 (예: 최소 3자리 확보 → 999까지)
	max = qMax(max, 999);  // 최소 줄 번호는 999까지 확보

	while (max >= 10) {
		max /= 10;
		++digits;
	}

	int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
	return space;
}

void CodeEditor::updateLineNumberAreaWidth(int) {
	setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect& rect, int dy) {
	if (dy)
		lineNumberArea->scroll(0, dy);
	else
		lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

	if (rect.contains(viewport()->rect()))
		updateLineNumberAreaWidth(0);
}

QString CodeEditor::wordUnderCursor() const {
	QTextCursor tc = textCursor();
	tc.select(QTextCursor::WordUnderCursor);
	QString word = tc.selectedText();
	return word;
}


QString CodeEditor::contextUnderCursor() {
	QTextCursor tc = textCursor();
	QString line = tc.block().text().left(tc.positionInBlock());

	// 함수 문맥인지 확인
	if (line.startsWith("$(")) {
		return "function";
	}
	// 특수 타겟인지 확인
	if (line.trimmed().startsWith(".")) {
		return "special";
	}
	return "directive";
}

///
void CodeEditor::insertCompletion(const QString& completion){
	QTextCursor tc = textCursor();

	// 현재 단어가 '.'만 있는 경우, 또는 '.'으로 시작하는 경우 처리
	tc.select(QTextCursor::WordUnderCursor);
	QString currentWord = tc.selectedText();

	if (customWordUnderCursor(this).startsWith(".")) {
		customRemoveSelectedText(this);
	}

	tc.insertText(completion);
	setTextCursor(tc);
}

/// 

void CodeEditor::resizeEvent(QResizeEvent* event) {
	QPlainTextEdit::resizeEvent(event);

	QRect cr = contentsRect();
	lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::keyPressEvent(QKeyEvent* e) {
	if (completer && completer->popup()->isVisible()) {
		switch (e->key()) {
		case Qt::Key_Enter:
		case Qt::Key_Return:
		case Qt::Key_Escape:
		case Qt::Key_Tab:
		case Qt::Key_Backtab:
			e->ignore(); // 팝업 처리
			return;
		}
	}

	QPlainTextEdit::keyPressEvent(e);

	QString context = contextUnderCursor();
	QString currentWord = customWordUnderCursor(this);


	QStringList completionList;
	if (context == "function") {
		for (const auto& word : functions) {
			completionList << QString::fromStdString(word);
		}
	}
	else if (context == "special") {
		for (const auto& word : special_target) {
			completionList << QString::fromStdString(word);
		}
	}
	else {
		for (const auto& word : directive_set) {
			completionList << QString::fromStdString(word);
		}
	}
	completionList.removeDuplicates();

	completer->model()->deleteLater();
	completer->setModel(new QStringListModel(completionList, completer));
	completer->setCompletionPrefix(currentWord);

	if (!currentWord.isEmpty()) {
		int popupWidth = completer->popup()->sizeHintForColumn(0)
			+ completer->popup()->verticalScrollBar()->sizeHint().width();

		QRect cr = cursorRect();
		cr.translate(25, 0);
		cr.setWidth(popupWidth);
		completer->complete(cr);  // 팝업 표시
	}
	else {
		completer->popup()->hide();
	}
}


void CodeEditor::highlightCurrentLine() {
	QList<QTextEdit::ExtraSelection> extraSelections;

	if (!isReadOnly()) {
		QTextEdit::ExtraSelection selection;
		QColor lineColor = QColor(Qt::gray).lighter(160);
		selection.format.setBackground(lineColor);
		selection.format.setProperty(QTextFormat::FullWidthSelection, true);
		selection.cursor = textCursor();
		selection.cursor.clearSelection();
		extraSelections.append(selection);
	}

	setExtraSelections(extraSelections);
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent* event) {
	QPainter painter(lineNumberArea);
	painter.fillRect(event->rect(), Qt::lightGray);

	QTextBlock block = firstVisibleBlock();
	int blockNumber = block.blockNumber();
	int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
	int bottom = top + qRound(blockBoundingRect(block).height());

	while (block.isValid() && top <= event->rect().bottom()) {
		if (block.isVisible() && bottom >= event->rect().top()) {
			QString number = QString::number(blockNumber + 1);
			painter.setPen(Qt::black);
			painter.drawText(0, top, lineNumberAreaWidth() - 4, fontMetrics().height(),
				Qt::AlignRight, number);
		}

		block = block.next();
		top = bottom;
		bottom = top + qRound(blockBoundingRect(block).height());
		++blockNumber;
	}
}

QStringListModel* CombineKeyword(QObject* parent){
	QStringList allKeywords;

	// 각 set을 QStringList에 병합
	for (const auto& word : directive_set) {
		allKeywords << QString::fromStdString(word);
	}
	for (const auto& word : special_target) {
		allKeywords << QString::fromStdString(word);
	}

	// 중복 제거
	allKeywords.removeDuplicates();
	allKeywords.sort();

	// QStringListModel 생성 및 반환
	return new QStringListModel(allKeywords, parent);
}

