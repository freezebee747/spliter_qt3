#pragma once
#pragma once
#include <optional>

#include "error.h"
#include "keyword.h"
#include "read.h"

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
	std::optional<std::string> default_target = std::nullopt;

	bool variable_expend(Variable& var, std::unordered_set<std::string>& visited);

public:
	TargetSet& GetTargets();
	PatternRuleList& GetPatternMap();
	ASTNodeList Getnodes();
	ErrorCollector& GetError();
	std::vector<Comment> GetComment();
	std::optional<std::string> GetDefaultTarget();

	void SetVariable_map(VariableMap& map) {
		variable_map = map;
	}
	const VariableMap& GetVariableMap();
	bool variable_expend(Variable& var);
	void variable_emplace(Variable& var);
	void parsing(const std::string& text);
	std::vector<Block> SplitByBlock(FileLines& file);

	bool VariableRef_expend(std::string& var, std::unordered_set<std::string>& visited);
	bool Function_expend(std::string& func);

	//지시자 처리 메서드
	bool ProcessingDirective(Block& block);
	bool directive_override(Block& block);

};

ExpansionType DeduceExpansionType(const std::string& str);
bool IsPatternRule(const std::string& str);

std::optional<Directive> TryParseDirectiveLine(int line, const std::string& str);
