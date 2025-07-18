#pragma once
#pragma once
#include <optional>

#include "read.h"
#include "error.h"
#include "keyword.h"

using ASTNodeList = std::vector<std::shared_ptr<ASTNode>>;
using FileLines = std::vector<std::pair<unsigned, std::string>>;
using OverrideSet = std::unordered_set<std::string>;
using VariableMap = std::unordered_map<std::string, std::shared_ptr<Variable>>;
using TargetSet = std::unordered_set<std::string>;
using PatternRuleList = std::vector<std::shared_ptr<Pattern_Rule>>;

enum class ExpansionType { Literal, VariableRef, Function };
enum class BlockType { variable, rule, directive, none };

struct Comment {
	std::string comment;
	int line;
	int column;
};

struct Block {
	BlockType type;
	std::pair<int, std::string> var_line;
	std::vector<std::pair<int, std::string>> _lines;
};

class Parser {
private:
	ASTNodeList nodes;
	FileLines raw_file;
	OverrideSet override_name;
	VariableMap variable_map;
	TargetSet target_map;
	PatternRuleList pattern_map;
	ErrorCollector ec;
	std::vector<Comment> comment_list;

	bool variable_expend(Variable& var, std::unordered_set<std::string>& visited);
	bool VariableRef_expend(std::string& var, std::unordered_set<std::string>& visited);
	bool Function_expend(std::string& func);

public:
	TargetSet& GetTargets();
	PatternRuleList& GetPatternMap();
	ASTNodeList Getnodes();
	ErrorCollector& GetError();
	std::vector<Comment> GetComment();

	void SetVariable_map(VariableMap& map) {
		variable_map = map;
	}
	const VariableMap& GetVariableMap();
	bool variable_expend(Variable& var);
	void variable_emplace(Variable& var);
	void parsing(const std::string& text);
	std::vector<Block> SplitByBlock(FileLines& file);

	bool directive_override(Block& block);

	//지시자 처리 메서드
	bool ProcessingDirective(Block& block);


};

ExpansionType DeduceExpansionType(const std::string& str);
bool IsPatternRule(const std::string& str);

std::optional<Directive> TryParseDirectiveLine(int line, const std::string& str);
