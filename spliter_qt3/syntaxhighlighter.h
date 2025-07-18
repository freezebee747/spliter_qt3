#pragma once

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <unordered_set>
#include "syntax.h"
#include "keyword.h"

enum class HighlightType {
	Keyword,
	Variable,
	Function,
	Target,
	Prerequisite,
	Directive,
	Recipe,
	Comment,
	Pattern,
	Warning,
	Error
};

struct HighlightInfo {
	int column;             // 줄 내에서 시작 위치
	int length;             // 하이라이팅할 길이
	HighlightType type;     // 어떤 종류의 토큰인지
};


class ASTHighlighter : public QSyntaxHighlighter {
public:
	ASTHighlighter(QTextDocument* parent = nullptr);
	void setAST(const std::vector<std::shared_ptr<ASTNode>>& ast, const std::vector<QString>& sourceLines);
	void setComment(std::vector<Comment> comments, const std::vector<QString>& sourceLines);
	void setErrors(const ErrorCollector& ec, const std::vector<QString>& sourceLines);
	void clearHighlighter();
protected:
	void highlightBlock(const QString& text) override;

private:
	std::unordered_map<int, std::vector<HighlightInfo>> lineHighlights;
	std::unordered_map<int, std::vector<HighlightInfo>> ErrorHighlights;
};