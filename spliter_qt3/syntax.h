#pragma once
#include "DAG.h"
#include "file.h"
#include "parser.h"
#include <optional>

using Mini_Error = std::pair<std::string, Severity>;
using RecipeMap = std::unordered_map<std::string, std::string>;

struct Token {
	std::string name;
	std::string value;
	std::string expended;
	ExpansionType type;
};

struct CheckResult {
	bool success;
	std::optional<Mini_Error> error;
};


struct RecipeSet {
	Recipe raw_recipe;
	RecipeMap recipe_map;
	std::string expend_recipe;
};

struct SyntaxError : std::exception {
	std::string code;
	int line;
	int column;
	int size;
	Severity severity;
	SyntaxError(const std::string& m,int l, int c, int s, Severity se) 
		: code(m), line(l), column(c), size(s), severity(se){}
};

class SyntaxChecker {
private:
	Parser& parser;
	
	std::unordered_map<std::string, Token> variables;
	std::unordered_set<std::string> variable_name_map;


	bool Function_expend(std::string& func);
	bool VariableRef_expend(std::string& var);

	std::pair<bool, std::optional<Mini_Error>> TargetCheck(Target& target);
	std::pair<bool, std::optional<Mini_Error>> PrerequisiteCheck(Prerequisite& prerequisite, std::unordered_set<std::string>& result_preqs);
	bool RecipeCheck(std::optional<std::pair<int, Semi_colon>> semi_colon, std::vector<Recipe>& recipe);
	bool precise_analysis_of_recipes(RecipeSet& rs);

	ErrorCollector ec;
	FileManagement fm;
	DirectedAcyclicGraph dag;
public:
	SyntaxChecker(Parser& p);
	void SyntaxCheck();
	Token ASTVariabletoToken(Variable& var);
	bool variable_expend(Token& token);
	bool VariableCheck(Variable& var);
	bool IsMatchingPattern(const std::string& target);
	bool IsMatchingPattern(const std::string& target, Pattern_Rule& pattern);
	bool AddPatternToDag(const std::string& target);
	bool ExplicitRuleCheck(Explicit_Rule& ex, bool toplevel);
	bool MultipleTargetCheck(Multiple_Target& mt, bool toplevel);
	bool PatternRuleCheck(Pattern_Rule& pr, bool toplevel);
	bool StaticPatternRuleCheck(Static_Pattern_Rule& spr, bool toplevel);

	ErrorCollector GetError();
};