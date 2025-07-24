#include "syntaxhighlighter.h"
#include <QTextCharFormat>
#include <QDebug>

int byteOffsetToUtf16Column(const QString& line, int byteOffset) {
	int currentByte = 0;

	for (int i = 0; i < line.length(); ++i) {
		int charByteLen = QString(line[i]).toUtf8().size();
		if (currentByte + charByteLen > byteOffset)
			return i;
		currentByte += charByteLen;
	}
	return line.length();
}


ASTHighlighter::ASTHighlighter(QTextDocument* parent)
	:QSyntaxHighlighter(parent)
{}

void ASTHighlighter::setAST(const std::vector<std::shared_ptr<ASTNode>>& ast,
	const std::vector<QString>& sourceLines) {

	for (const auto& node : ast) {
		if (auto* var = dynamic_cast<Variable*>(node.get())) {
			const int line = var->line - 1;
			if (line >= sourceLines.size()) continue;

			int col = byteOffsetToUtf16Column(sourceLines[line], var->name.first);
			int len = QString::fromStdString(var->name.second).length() + 1;

			lineHighlights[line].push_back({ col, len, HighlightType::Variable });
		}

		else if (auto* rule = dynamic_cast<Explicit_Rule*>(node.get())) {
			const int line = rule->line - 1;
			if (line >= sourceLines.size()) continue;

			int targetCol = byteOffsetToUtf16Column(sourceLines[line], rule->target.first);
			int targetLen = QString::fromStdString(rule->target.second).length();
			lineHighlights[line].push_back({ targetCol, targetLen, HighlightType::Target });

			for (const auto& preq : rule->prerequisite) {
				int preqCol = byteOffsetToUtf16Column(sourceLines[line], preq.first);
				int preqLen = QString::fromStdString(preq.second).length();
				lineHighlights[line].push_back({ preqCol, preqLen, HighlightType::Prerequisite });
			}

			if (!rule->semi_colon_recipe.second.empty()) {
				int SemiRecCol = byteOffsetToUtf16Column(sourceLines[line], rule->semi_colon_recipe.first);
				int SemiRecLen = QString::fromStdString(rule->semi_colon_recipe.second).length();
				lineHighlights[line].push_back({ SemiRecCol, SemiRecLen, HighlightType::Recipe });
			}

			for (const auto& recipe : rule->recipes) {
				int recipeLine = recipe.first - 1;
				if (recipeLine >= sourceLines.size()) continue;

				int len = QString::fromStdString(recipe.second).length() + 1;
				lineHighlights[recipeLine].push_back({ 0, len + 1, HighlightType::Recipe });
			}
		}

		else if (auto* rule = dynamic_cast<Multiple_Target*>(node.get())) {
			const int line = rule->line - 1;
			if (line >= sourceLines.size()) continue;

			for (const auto& target : rule->targets) {
				int targetCol = byteOffsetToUtf16Column(sourceLines[line], target.first);
				int targetLen = QString::fromStdString(target.second).length();
				lineHighlights[line].push_back({ targetCol, targetLen, HighlightType::Target });
			}

			for (const auto& preq : rule->prerequisite) {
				int preqCol = byteOffsetToUtf16Column(sourceLines[line], preq.first);
				int preqLen = QString::fromStdString(preq.second).length();
				lineHighlights[line].push_back({ preqCol, preqLen, HighlightType::Prerequisite });
			}

			if (!rule->semi_colon_recipe.second.empty()) {
				int SemiRecCol = byteOffsetToUtf16Column(sourceLines[line], rule->semi_colon_recipe.first);
				int SemiRecLen = QString::fromStdString(rule->semi_colon_recipe.second).length();
				lineHighlights[line].push_back({ SemiRecCol, SemiRecLen, HighlightType::Recipe });
			}

			for (const auto& recipe : rule->recipes) {
				int recipeLine = recipe.first - 1;
				if (recipeLine >= sourceLines.size()) continue;

				int len = QString::fromStdString(recipe.second).length() + 1;
				lineHighlights[recipeLine].push_back({ 0, len, HighlightType::Recipe });
			}
		}

		else if (auto* rule = dynamic_cast<Pattern_Rule*>(node.get())) {
			const int line = rule->line - 1;
			if (line >= sourceLines.size()) continue;

			int targetCol = byteOffsetToUtf16Column(sourceLines[line], rule->target_pattern.first);
			int targetLen = QString::fromStdString(rule->target_pattern.second).length();
			lineHighlights[line].push_back({ targetCol, targetLen, HighlightType::Pattern });

			for (const auto& preq : rule->prerequisite_pattern) {
				if (SeparatorCounter(preq.second, '%') == 1) {
					int preqCol = byteOffsetToUtf16Column(sourceLines[line], preq.first);
					int preqLen = QString::fromStdString(preq.second).length();
					lineHighlights[line].push_back({ preqCol, preqLen, HighlightType::Pattern });
				}
			}
			
			for (const auto& recipe : rule->recipes) {
				int recipeLine = recipe.first - 1;
				if (recipeLine >= sourceLines.size()) continue;

				int len = QString::fromStdString(recipe.second).length() + 1;
				lineHighlights[recipeLine].push_back({ 0, len + 1, HighlightType::Recipe });
			}

		}
		else if (auto* rule = dynamic_cast<Static_Pattern_Rule*>(node.get())) {
			for (const auto& target : rule->target) {
				const int line = rule->line - 1;
				if (line >= sourceLines.size()) continue;

				for (const auto& target : rule->target) {
					int targetCol = byteOffsetToUtf16Column(sourceLines[line], target.first);
					int targetLen = QString::fromStdString(target.second).length();
					lineHighlights[line].push_back({ targetCol, targetLen, HighlightType::Target });
				}

				int tpCol = byteOffsetToUtf16Column(sourceLines[line], rule->target_pattern.first);
				int tpLen = QString::fromStdString(rule->target_pattern.second).length();
				lineHighlights[line].push_back({ tpCol, tpLen, HighlightType::Pattern });

				for (const auto& preq : rule->prerequisite_pattern) {
					if (SeparatorCounter(preq.second, '%') == 1) {
						int preqCol = byteOffsetToUtf16Column(sourceLines[line], preq.first);
						int preqLen = QString::fromStdString(preq.second).length();
						lineHighlights[line].push_back({ preqCol, preqLen, HighlightType::Pattern });
					}
				}

				for (const auto& recipe : rule->recipes) {
					int recipeLine = recipe.first - 1;
					if (recipeLine >= sourceLines.size()) continue;

					int len = QString::fromStdString(recipe.second).length() + 1;
					lineHighlights[recipeLine].push_back({ 0, len, HighlightType::Recipe });
				}

			}
		}
		// 기타 노드 처리
	}
	rehighlight();
}

void ASTHighlighter::setComment(std::vector<Comment> comments,
	const std::vector<QString>& sourceLines){

	for (const auto& comment : comments) {
		int line = comment.line - 1;
		if (line >= sourceLines.size()) continue;

		int col = byteOffsetToUtf16Column(sourceLines[line], comment.column);
		int len = QString::fromStdString(comment.comment).length();

		lineHighlights[line].push_back({ col, len, HighlightType::Comment });
	}

	rehighlight();
}

void ASTHighlighter::setErrors(const ErrorCollector& ec, const std::vector<QString>& sourceLines) {
	ErrorHighlights.clear();
	for (const auto& error : ec.GetAll()) {
		int line = error.line - 1;
		if (line >= sourceLines.size()) continue;

		//여기서 1은 tab 보정
		int col = byteOffsetToUtf16Column(sourceLines[line], error.column) + 1;
		int len = error.size;

		HighlightType type = (error.severity == Severity::Warning) ? HighlightType::Warning : HighlightType::Error;
		ErrorHighlights[line].push_back({ col, len, type });
	}
	rehighlight();
}

void ASTHighlighter::clearHighlighter(){
	lineHighlights.clear();
}


void ASTHighlighter::highlightBlock(const QString& text) {
	int line = currentBlock().blockNumber();

	// 일반 하이라이팅 먼저 적용
	if (lineHighlights.contains(line)) {
		for (const auto& info : lineHighlights[line]) {
			QTextCharFormat fmt;
			switch (info.type) {
			case HighlightType::Keyword:
				fmt.setForeground(Qt::blue);
				fmt.setFontWeight(QFont::Bold);
				break;
			case HighlightType::Variable:
				fmt.setForeground(Qt::darkGreen);
				break;
			case HighlightType::Target:
				fmt.setForeground(Qt::darkMagenta);
				break;
			case HighlightType::Recipe:
				fmt.setForeground(Qt::gray);
				fmt.setFontItalic(true);
				break;
			case HighlightType::Comment:
				fmt.setForeground(Qt::green);
				fmt.setFontWeight(QFont::Bold);
				break;
			case HighlightType::Pattern:
				fmt.setForeground(Qt::darkBlue);
				fmt.setFontWeight(QFont::Bold);
				break;
			default:
				break;
			}
			setFormat(info.column, info.length, fmt);
		}
	}

	// 에러/경고 하이라이팅은 항상 마지막에 덮어씀
	if (ErrorHighlights.contains(line)) {
		for (const auto& info : ErrorHighlights[line]) {
			QTextCharFormat fmt;
			if (info.type == HighlightType::Warning) {
				fmt.setUnderlineColor(Qt::darkGreen);
				fmt.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
			}
			else if (info.type == HighlightType::Error) {
				fmt.setUnderlineColor(Qt::red);
				fmt.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
			}
			setFormat(info.column, info.length, fmt);
		}
	}
}