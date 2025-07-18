#pragma once
#include <string>
#include <vector>

using column = int;
using Target = std::pair<column, std::string>;
using Prerequisite = std::pair<column, std::string>;
using Recipe = std::pair<unsigned, std::string>;
using Semi_colon = std::pair<column, std::string>;

enum class VariableType {
	Simple,    // =
	Immediate, // :=
	Append,    // +=
	Conditional // ?=
};

struct ASTNode {
	virtual ~ASTNode() = default;
};

struct Directive : ASTNode {
	unsigned line;
	std::pair<column, std::string> directive;
	std::vector<std::string> arguments;
};

struct Variable :ASTNode {
	unsigned line;
	std::pair<column, std::string> name;
	std::string value;
	std::string expended;
	VariableType type;
};

struct Multiple_Target : ASTNode {
	unsigned line;
	std::vector<Target> targets;
	std::vector<Prerequisite> prerequisite;
	Semi_colon semi_colon_recipe;
	std::vector<Recipe> recipes;
};

struct Explicit_Rule : ASTNode {
	unsigned line;
	Target target;
	std::vector<Prerequisite> prerequisite;
	Semi_colon semi_colon_recipe;
	std::vector<Recipe> recipes;
};

struct Pattern_Rule : ASTNode {
	unsigned line;
	Target target_pattern;
	std::vector<Prerequisite> prerequisite_pattern;
	Semi_colon semi_colon_recipe;
	std::vector<Recipe> recipes;
};

struct Static_Pattern_Rule : ASTNode {
	unsigned line;
	std::vector<Target> target;
	std::pair<column, std::string> target_pattern;
	std::vector<Prerequisite> prerequisite_pattern;
	Semi_colon semi_colon_recipe;
	std::vector<Recipe> recipes;
};

struct Phony_Target : ASTNode {
	unsigned line;
	std::pair<column, std::string> target;
	std::vector<std::pair<unsigned, std::string>> recipes;
};

