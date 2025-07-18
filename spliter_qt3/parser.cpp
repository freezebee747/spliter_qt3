#include "parser.h"
#include "function.h"

Variable MakeVariable(int line, const std::string& variable) {
	Variable var;

	var.line = line;
	size_t Sep = variable.find('=');

	std::string name = trim(safe_substr(variable, 0, Sep - 1));
	std::string value = trim(safe_substr(variable, Sep + 1, variable.size()));

	var.name.first = 0;
	var.name.second = name;
	var.value = value;

	if (variable.find(":=") != std::string::npos) {
		var.type = VariableType::Immediate;
		var.expended = "";
	}
	else if (variable.find("?=") != std::string::npos) {
		var.type = VariableType::Conditional;
		var.expended = var.value;
	}
	else if (variable.find("+=") != std::string::npos) {
		var.type = VariableType::Append;
		var.expended = var.value;
	}
	else {
		var.type = VariableType::Simple;
		var.expended = var.value;
	};

	return var;
}

//지시자 처리 메서드
std::string ExtractDirectiveName(Block& block) {
	std::string first_line = block._lines[0].second;
	std::string name = trim(SplitSpace(first_line)[0]);
	auto it = directive_set.find(name);
	if (it != directive_set.end()) {
		return name;
	}
	else return "";
}
using DirectiveHandler = std::function<bool(Parser&, Block&)>;

//override a = b
bool Parser::directive_override(Block& block) {
	std::string first_line = block._lines[0].second;
	std::string raw_arg = trim(first_line.substr(8, first_line.size() - 8));

	size_t equal = raw_arg.find('=');
	std::string name = trim(safe_substr(raw_arg, 0, equal - 1));

	Variable var;
	var = MakeVariable(block._lines[0].first, raw_arg);
	variable_emplace(var);
	override_name.insert(name);
	return true;
}

static std::unordered_map<std::string, DirectiveHandler> Directive_map = {
	{"override", [](Parser& parser, Block& block) {
		return parser.directive_override(block);
	}}
};

bool Parser::ProcessingDirective(Block& block) {
	std::string directive_name = ExtractDirectiveName(block);
	auto it = Directive_map.find(directive_name);
	if (it != Directive_map.end()) {
		return it->second(*this, block);  // Parser 인스턴스 전달
	}
	return false;
}
//여기까지가 지시자 처리

ExpansionType DeduceExpansionType(const std::string& str) {
	if (str.size() >= 3 && str[0] == '$' && str[1] == '(' && str.back() == ')') {
		std::string inner = str.substr(2, str.size() - 3);
		std::string head = SplitSpace(inner)[0];
		if (functions.find(head) != functions.end()) {
			return ExpansionType::Function;
		}
		return ExpansionType::VariableRef;
	}
	else if (str.size() >= 4 && str[0] == '$' && str[1] == '$' && str[2] == '(' && str.back() == ')') {
		return ExpansionType::VariableRef;
	}
	return ExpansionType::Literal;
}

bool IsPatternRule(const std::string& str) {
	if (str.find(":") != std::string::npos) {
		size_t colon_pos = str.find(':');
		std::string target = trim(safe_substr(str, 0, colon_pos));
		if (target.find("%") != std::string::npos) {
			return true;
		}
	}
	return false;
}

const VariableMap& Parser::GetVariableMap() {
	return variable_map;
}


std::vector<Block> Parser::SplitByBlock(FileLines& file) {
	std::vector<Block> blocks;
	bool recipe_flag = false;

	FileLines RemovedCommentLines;
	//주석 처리
	for (size_t i = 0; i < file.size(); ++i) {
		auto& [lineNum, lineText] = file[i];

		if (lineText.find('#') != std::string::npos) {
			//주석 저장
			Comment comment;
			size_t finder = lineText.find_first_of('#');

			comment.comment = lineText.substr(finder, lineText.size() - finder);
			comment.line = lineNum;
			comment.column = finder;

			comment_list.push_back(comment);

			//주석 제거
			lineText.erase(finder);

		}

	}


	// 백슬래시로 이어지는 라인들을 합치기 위한 코드
	FileLines combinedLines;
	for (size_t i = 0; i < file.size(); ++i) {
		auto& [lineNum, lineText] = file[i];
		std::string combined = lineText;
		unsigned originalLine = lineNum;

		while (!combined.empty() && combined.back() == '\\') {
			combined.pop_back();
			if (++i < raw_file.size()) {
				combined += trim(file[i].second);
			}
			else {
				break;
			}
		}
		combinedLines.emplace_back(originalLine, combined);
	}

	std::unique_ptr<Block> block;
	block.reset();

	for (auto it = combinedLines.begin(); it != combinedLines.end(); ++it) {
		const auto& [line, str] = *it;

		//만일 빈 라인이면 일단 넘긴다.
		if (str.empty())continue;

		//지시자 처리
		if (auto directive = TryParseDirectiveLine(line, str)) {
			block = std::make_unique<Block>();
			block->type = BlockType::directive;
			block->_lines.push_back({ line, str });

			// define, undefine 등 블록형 directive 처리
			if (directive->directive.second == "define") {
				++it;
				while (it != combinedLines.end()) {
					const auto& [next_line, next_str] = *it;
					block->_lines.emplace_back(next_line, next_str);
					if (trim(next_str) == "endef") {
						break;
					}
					++it;
				}
			}

			blocks.push_back(*block);
			block.reset();
			continue;
		}
		//변수 처리
		//만일 tab으로 시작하지 않으면서 = 가 포함되어있다면 변수이므로 한 블록으로 처리한다..
		//일단 즉시 확장 변수이던 누적 변수이던 변수로 취급하자
		if (str.find("=") != std::string::npos && str[0] != '\t') {
			block = std::make_unique<Block>();
			block->type = BlockType::variable;
			block->var_line = { line, str };
			blocks.push_back(*block);
			block.reset();
			continue;
		}

		//규칙 처리
		//만일 tab으로 시작하지 않으면서 : 가 포함되어 있다면 규칙이므로 그 블록을 저장한다.
		//다음 문장이 tab으로 시작하지 않을 때 까지 recipe 로 취급한다.
		//tab이 포함된 빈 줄은 블록에 포함하고, tab이 포함되지 않은 빈 줄은 버린다.
		if (str.find(":") != std::string::npos && str[0] != '\t') {
			block = std::make_unique<Block>();
			block->type = BlockType::rule;
			block->_lines.push_back({ line, str });

		}

		if (str.find(";") != std::string::npos) {
			auto next_it = std::next(it);
			if (next_it != combinedLines.end() && next_it->second[0] != '\t') {
				blocks.push_back(*block);
				block.reset();
			}
		}

		if (block && str[0] == '\t') {
			block->_lines.push_back({ line, str });
			//만일 다음 라인이(빈 라인은 첫번째 조건문에서 버려진다.) 탭으로 시작하지 않는다면 block를 저장한 후 클리어한다.
			auto next_it = std::next(it);
			if (next_it != combinedLines.end() && next_it->second[0] != '\t') {
				blocks.push_back(*block);
				block.reset();
			}
			continue;
		}
	}

	if (block) {
		blocks.push_back(*block);
	}
	return blocks;
}

bool Parser::variable_expend(Variable& var) {
	std::unordered_set<std::string> visited;
	return variable_expend(var, visited);
}

bool Parser::variable_expend(Variable& var, std::unordered_set<std::string>& visited) {
	if (visited.contains(var.name.second)) {
		return false; // 순환 참조 방지
	}
	visited.insert(var.name.second);

	std::vector<std::string> variables = SplitValues(var.value);
	std::string result_str;
	bool overall_result = true;

	for (std::string& token : variables) {
		ExpansionType et = DeduceExpansionType(token);

		switch (et) {
		case ExpansionType::VariableRef:
			if (!VariableRef_expend(token, visited)) {
				overall_result = false;
			}
			break;

		case ExpansionType::Function:
			if (!Function_expend(token)) {
				overall_result = false;
			}
			break;

		case ExpansionType::Literal:
			// 그대로 사용
			break;
		}

		result_str += token;
	}

	var.expended = result_str;
	visited.erase(var.name.second);
	return overall_result;
}

bool Parser::VariableRef_expend(std::string& var, std::unordered_set<std::string>& visited) {
	std::string raw_variable;

	if (var.starts_with("$$(") && var.ends_with(")")) {
		var = var.substr(1, var.size() - 1);
		return true;
	}

	// ${VAR} 또는 $(VAR) 형식 처리
	if ((var.starts_with("${") && var.ends_with("}")) ||
		(var.starts_with("$(") && var.ends_with(")"))) {
		raw_variable = var.substr(2, var.size() - 3);
	}
	else {
		return false; // 올바르지 않은 형식
	}

	auto it = variable_map.find(raw_variable);
	if (it != variable_map.end()) {
		if (!variable_expend(*it->second, visited)) {
			return false;
		}
		var = it->second->expended;
		return true;
	}

	return false; // 정의되지 않은 변수
}

bool Parser::Function_expend(std::string& func) {

	std::string func_name = ExtractFunctionName(func);
	std::vector<std::string> func_arg = ExtractFunctionArguments(func);
	std::string result = Active_function(func_name, func_arg);
	func.swap(result);
	return true;
}

std::unordered_set<std::string>& Parser::GetTargets() {
	return target_map;
}

PatternRuleList& Parser::GetPatternMap() {
	return pattern_map;
}

ASTNodeList Parser::Getnodes() {
	return nodes;
}

ErrorCollector& Parser::GetError() {
	return ec;
}

std::vector<Comment> Parser::GetComment(){
	return comment_list;
}


void Parser::variable_emplace(Variable& var) {
	if (var.type == VariableType::Conditional) {
		auto it = variable_map.find(var.name.second);
		if (it != variable_map.end()) {
			return;
		}
	}
	else if (var.type == VariableType::Immediate) {
		variable_expend(var);
	}

	//else if ...

	variable_map.emplace(var.name.second, std::make_shared<Variable>(var));
	nodes.push_back(std::make_shared<Variable>(var));
}

void Parser::parsing(const std::string& text){
	nodes.clear();
	raw_file = ReadFileWithLineNumbers(text);
	FileLines combine_line = JoinSplitLine(raw_file);
	std::vector<Block> blocks = SplitByBlock(combine_line);

	for (auto& block : blocks) {

		if (block.type == BlockType::directive) {
			//여기서 지시자를 처리한다.
			if (!ProcessingDirective(block)) {

			}
			continue;
		}

		else if (block.type == BlockType::variable) {
			Variable var;
			var = MakeVariable(block.var_line.first, block.var_line.second);
			auto find = override_name.find(var.name.second);
			if (find != override_name.end()) {
				//오버라이드 된 variable이 이미 존재함
				//따라서 이 variable은 버려짐
				continue;
			}
			variable_emplace(var);
		}


		else if (block.type == BlockType::rule) {
			std::string first_line = block._lines[0].second;

			if (SeparatorCounter(first_line, ':') == 1) {
				size_t colon_pos = first_line.find(':');

				//phony target
				if (trim(safe_substr(first_line, colon_pos + 1, first_line.size() - colon_pos - 1)) == "") {

					Phony_Target pt;

					std::string phony_name = trim(safe_substr(first_line, 0, colon_pos));
					int phony_column = first_line.find(phony_name);

					//라인 번호를 저장한다.
					pt.line = block._lines[0].first;

					//phony target의 target을 지정한다.
					pt.target.first = phony_column;
					pt.target.second = phony_name;

					//블록에서 레시피 추출
					std::vector<std::pair<unsigned, std::string>> phony_recipe;
					for (size_t i = 1; i < block._lines.size(); i++) {
						phony_recipe.push_back({ block._lines[i].first,  trim(block._lines[i].second) });
					}

					//레시피 저장
					pt.recipes = phony_recipe;

					//노드에 밀어넣기
					nodes.push_back(std::make_shared<Phony_Target>(pt));
					continue;

				}

				//pattern rule
				else if (first_line.find("%") != std::string::npos) {
					if (IsPatternRule(first_line)) {
						Pattern_Rule pr;

						//라인 번호를 저장한다.
						pr.line = block._lines[0].first;

						// target pattern 저장
						std::string target = trim(safe_substr(first_line, 0, colon_pos));
						size_t target_column = first_line.find(target);

						pr.target_pattern.first = target_column;
						pr.target_pattern.second = target;

						// ";" 가 있는가?
						size_t intTemp = (first_line.find(';') != std::string::npos) ? first_line.find(';') : first_line.size();
						// prerequisite pattern 저장
						std::string preq = trim(safe_substr(first_line, colon_pos + 1, intTemp - colon_pos - 1));
						std::vector<std::string> raw_preqs = SplitSpace(preq);
						for (const auto& raw_preq : raw_preqs) {
							size_t preq_column = first_line.find(raw_preq);
							pr.prerequisite_pattern.push_back({ preq_column, raw_preq });
						}
						

						if (first_line.find(';') != std::string::npos) {
							std::string rec = safe_substr(first_line, intTemp, first_line.size() - intTemp);
							int finder = first_line.find(rec);
							if (rec != "") {
								pr.semi_colon_recipe = { finder, rec };
							}
							else {
								pr.semi_colon_recipe = { finder, "" };
							}
						}

						for (size_t i = 1; i < block._lines.size(); i++) {
							pr.recipes.push_back({ block._lines[i].first, trim(block._lines[i].second) });
						}
						pattern_map.push_back(std::make_shared<Pattern_Rule>(pr));
						nodes.push_back(std::make_shared<Pattern_Rule>(pr));
					}
					else {
						//error
					}
					continue;
				}
				std::vector<std::string> targets = SplitSpace(trim(safe_substr(first_line, 0, colon_pos)));
				std::vector<std::pair<int, std::string>> target_with_column;

				for (const auto& target : targets) {
					target_map.insert(target);
					std::pair<int, std::string> temp;
					temp.first = first_line.find(target);
					temp.second = target;
					target_with_column.push_back(temp);
				}

				//multiple target rule
				if (targets.size() != 1) {
					Multiple_Target mt;
					mt.line = block._lines[0].first;
					mt.targets = target_with_column;

					size_t intTemp = (first_line.find(';') != std::string::npos) ? first_line.find(';') : first_line.size();

					std::vector<std::string> prerequisites = SplitSpace(trim(safe_substr(first_line, colon_pos + 1, intTemp - colon_pos - 1)));
					std::vector<std::pair<int, std::string>> preq_with_column;
					for (const auto& prereq : prerequisites) {
						std::pair<size_t, std::string> temp;
						temp.first = first_line.find(prereq);
						temp.second = prereq;
						preq_with_column.push_back(temp);
					}

					mt.prerequisite = preq_with_column;

					if (first_line.find(';') != std::string::npos) {
						std::string rec = safe_substr(first_line, intTemp, first_line.size() - intTemp);
						int finder = first_line.find(rec);
						if (rec != "") {
							mt.semi_colon_recipe = { finder, rec };
						}
						else {
							mt.semi_colon_recipe = { finder, "" };
						}
					}

					for (size_t i = 1; i < block._lines.size(); i++) {
						mt.recipes.push_back({ block._lines[i].first, trim(block._lines[i].second) });
					}
					nodes.push_back(std::make_shared<Multiple_Target>(mt));
					continue;
				}
				//normal explicit rule
				else {
					Explicit_Rule ex;
					ex.line = block._lines[0].first;
					ex.target.first = first_line.find(targets[0]);
					ex.target.second = targets[0];

					int intTemp = (first_line.find(';') != std::string::npos) ? first_line.find(';') : first_line.size();

					std::vector<std::string> prerequisites = SplitSpace(trim(safe_substr(first_line, colon_pos + 1, intTemp - colon_pos - 1)));
					std::vector<std::pair<int, std::string>> preq_with_column;
					for (const auto& prereq : prerequisites) {
						std::pair<size_t, std::string> temp;
						temp.first = first_line.find(prereq);
						temp.second = prereq;
						preq_with_column.push_back(temp);
					}

					ex.prerequisite = preq_with_column;

					if (first_line.find(';') != std::string::npos) {
						std::string rec = safe_substr(first_line, intTemp, first_line.size() - intTemp);
						int finder = first_line.find(rec);
						if (rec != "") {
							ex.semi_colon_recipe = { finder, rec };
						}
						else {
							ex.semi_colon_recipe = { finder, "" };
						}
					}

					for (size_t i = 1; i < block._lines.size(); i++) {
						ex.recipes.push_back({ block._lines[i].first, trim(block._lines[i].second) });
					}
					nodes.push_back(std::make_shared<Explicit_Rule>(ex));
					continue;
				}
			}
			else if (SeparatorCounter(first_line, ':') == 2) {
				//static pattern rule
				Static_Pattern_Rule spr;
				spr.line = block._lines[0].first;
				int colon_pos = first_line.find(':');

				std::vector<std::string> targets = SplitSpace(trim(safe_substr(first_line, 0, colon_pos)));
				std::vector<std::pair<int, std::string>> target_with_column;

				for (const auto& target : targets) {
					target_map.insert(target);
					std::pair<size_t, std::string> temp;
					temp.first = first_line.find(target);
					temp.second = target;
					target_with_column.push_back(temp);
				}

				spr.target = target_with_column;

				int next_colon = first_line.find(':', colon_pos + 1);

				std::string target = trim(safe_substr(first_line, colon_pos + 1, next_colon - colon_pos - 1));
				size_t target_column = first_line.find(target);

				spr.target_pattern.first = target_column;
				spr.target_pattern.second = target;

				size_t intTemp = (first_line.find(';') != std::string::npos) ? first_line.find(';') : first_line.size();
				std::string preq = trim(safe_substr(first_line, next_colon + 1, intTemp - next_colon - 1));

				std::vector<std::string> raw_preqs = SplitSpace(preq);
				for (const auto& raw_preq : raw_preqs) {
					size_t preq_column = first_line.find(raw_preq);
					spr.prerequisite_pattern.push_back({ preq_column, raw_preq });
				}

				if (first_line.find(';') != std::string::npos) {
					std::string rec = safe_substr(first_line, intTemp, first_line.size() - intTemp);
					int finder = first_line.find(rec);
					if (rec != "") {
						spr.semi_colon_recipe = { finder, rec };
					}
					else {
						spr.semi_colon_recipe = { finder, "" };
					}
				}

				for (size_t i = 1; i < block._lines.size(); i++) {
					spr.recipes.push_back({ block._lines[i].first, trim(block._lines[i].second) });
				}

				nodes.push_back(std::make_shared<Static_Pattern_Rule>(spr));
			}
		}
	}

}
//지시자 처리 헬퍼 함수
std::optional<Directive> TryParseDirectiveLine(int line, const std::string& str) {
	std::string trimmed = trim(str);
	if (trimmed.empty() || trimmed[0] == '#') return std::nullopt;

	std::vector<std::string> tokens = SplitSpace(trimmed);
	if (tokens.empty()) return std::nullopt;

	if (directive_set.find(tokens[0]) != directive_set.end()) {
		Directive directive;
		directive.line = line;
		directive.directive.second = tokens[0];
		directive.arguments = { tokens.begin() + 1, tokens.end() };
		return directive;
	}
	return std::nullopt;
}
