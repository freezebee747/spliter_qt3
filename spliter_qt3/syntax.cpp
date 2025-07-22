#include "function.h"
#include "syntax.h"

SyntaxChecker::SyntaxChecker(Parser& p)
	:parser(p){

}

void SyntaxChecker::SyntaxCheck() {

	variable_name_map.clear();
	ec.clearError();

	for (const auto& node : parser.Getnodes()) {
		try {
			if (auto var = dynamic_cast<Variable*>(node.get())) {
				if (VariableCheck(*var)) {
					auto it = variables.find(var->name.second);
					if (it != variables.end()) {
						variable_expend(it->second);
					}
				}
			}
			else if (auto ex = dynamic_cast<Explicit_Rule*>(node.get())) {
				ExplicitRuleCheck(*ex, true);
			}
			else if (auto ex = dynamic_cast<Multiple_Target*>(node.get())) {
				MultipleTargetCheck(*ex, true);
			}
			else if (auto pattern = dynamic_cast<Pattern_Rule*>(node.get())) {
				PatternRuleCheck(*pattern, true);
			}
			else if (auto spattern = dynamic_cast<Static_Pattern_Rule*>(node.get())) {
				StaticPatternRuleCheck(*spattern, true);
			}
		}
		catch (const std::vector<SyntaxError>& err) {
			for (const auto& e : err) {
				ec.AddError(e.code, e.line, e.column, e.size, e.severity);
			}
		}
	}

}

Token SyntaxChecker::ASTVariabletoToken(Variable& var)
{
	Token result;
	result.name = var.name.second;
	//즉시확장한 변수인 경우엔 velue의 값을 expend(variable) 값으로
	//지연확장된 변수인 경우엔 value 값을 value(variable)의 값으로
	if (var.type == VariableType::Immediate) {
		result.value = var.expended;
	}
	else {
		result.value = var.value;
	}
	result.type = DeduceExpansionType(result.value);
	return result;
}

/////variable//////

bool SyntaxChecker::Function_expend(std::string& func) {

	std::string result;

	std::string func_name = ExtractFunctionName(func);
	std::vector<std::string> func_arg = ExtractFunctionArguments(func);


	if (IsNeedFunctionContext(func_name)) {
		FunctionContext fc;
		fc.checker = this;
		fc.call_by_syntax = true;
		result = Active_function(func_name, func_arg, fc);
	}
	else {
		result = Active_function(func_name, func_arg);
	}
	func.swap(result);
	return true;
}

bool SyntaxChecker::VariableRef_expend(std::string& var) {
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

	auto it = variables.find(raw_variable);
	if (it != variables.end()) {
		if (!variable_expend(it->second)) {
			return false;
		}
		var = it->second.expended;
		return true;
	}
	return false;
}


bool SyntaxChecker::variable_expend(Token& token)
{
	std::vector<std::string> variables = SplitValues(token.value);
	std::string result_str;
	bool overall_result = true;

	for (std::string& token : variables) {
		ExpansionType et = DeduceExpansionType(token);

		switch (et) {
		case ExpansionType::VariableRef:
			if (!VariableRef_expend(token)) {
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

	token.expended = result_str;
	return overall_result;
}



bool SyntaxChecker::VariableCheck(Variable& var)
{
	//변수 이름 중복 검사
	auto finder = variable_name_map.find(var.name.second);
	if (finder != variable_name_map.end()) {
		//에러 처리
		int line = var.line;
		int column = var.name.first;
		int size = var.name.second.size();
		ec.AddError("E012",line, column, size, Severity::Error);
		return false;
	}
	variable_name_map.emplace(var.name.second);

	//변수 순환 검사
	{
		std::string checker = var.value;
		std::vector<std::string> stacks;

		while (true) {
			if (!(checker.size() >= 4 && checker[0] == '$' && checker[1] == '(' && checker.back() == ')')) break;
			checker = checker.substr(2, checker.size() - 3);
			auto variable = variables.find(checker);
			if (variable != variables.end()) {
				stacks.push_back(checker);
				if (variable->second.type != ExpansionType::VariableRef) break;
				std::string temp = variable->second.value.substr(2, variable->second.value.size() - 3);
				auto key = std::find(stacks.begin(), stacks.end(), temp);
				if (key != stacks.end()) {
					int line = var.line;
					int column = var.name.first;
					int size = var.name.second.size();
					ec.AddError("E013", line, column, size, Severity::Error);
					return false;
				}
				checker = variable->second.value;
			}
		}
	}
	Token tk = ASTVariabletoToken(var);
	variables.emplace(var.name.second, tk);
	return true;
}

/////variable//////
bool SyntaxChecker::IsMatchingPattern(const std::string& target) {
	for (const auto& pattern : parser.GetPatternMap()) {
		if (IsMatchingPattern(target, *pattern)) {
			return true;
		}
	}
	return false;
}

bool SyntaxChecker::IsMatchingPattern(const std::string& target, Pattern_Rule& pattern) {

	auto [prefix, suffix] = SplitPattern(pattern.target_pattern.second);

	bool prefix_match = prefix.empty() ||
		(target.size() >= prefix.size() &&
			target.compare(0, prefix.size(), prefix) == 0);

	bool suffix_match = suffix.empty() ||
		(target.size() >= suffix.size() &&
			target.compare(target.size() - suffix.size(), suffix.size(), suffix) == 0);

	if (prefix_match && suffix_match) {
		return true;
	}
	return false;
}

bool SyntaxChecker::AddPatternToDag(const std::string& target) {
	for (const auto& pattern : parser.GetPatternMap()) {
		// 각 패턴에 대해 매칭 검사
		if (!IsMatchingPattern(target, *pattern))
			continue;

		// target_pattern에서 stem 추출
		auto [prefix, suffix] = SplitPattern(pattern->target_pattern.second);
		std::pair<std::string, std::string> pattern_pair = { prefix, suffix };
		std::string stem = ExtractStem(target, pattern_pair);
		if (stem.empty()) {
			continue; // 유효한 stem이 아니면 무시
		}

		std::string prerequisite_expanded;

		// prerequisite_pattern에 %가 있는 경우 stem 치환
		std::unordered_set<std::string> prerequisites;
		std::vector<std::string> preq_tokens;


		for (const auto& preq : pattern->prerequisite_pattern) {
			std::string expanded;

			if (preq.second.find('%') != std::string::npos) {
				auto [preq_prefix, preq_suffix] = SplitPattern(preq.second);
				expanded = preq_prefix + stem + preq_suffix;
			}
			else {
				expanded = preq.second;
			}

			// 공백 분할 (Make는 공백으로 여러 prerequisite를 나눔)
			std::vector<std::string> tokens =
				expanded.find(' ') != std::string::npos
				? SplitSpace(expanded)
				: std::vector<std::string>{ expanded };

			for (std::string& token : tokens) {
				switch (DeduceExpansionType(token)) {
				case ExpansionType::VariableRef:
					if (!VariableRef_expend(token)) return false;
					break;
				case ExpansionType::Function:
					if (!Function_expend(token)) return false;
					break;
				case ExpansionType::Literal:
					break;
				}
				prerequisites.insert(token);
			}
		}

		// DAG에 엣지 추가
		dag.AddEdge(target, prerequisites);

		return true; // 첫 번째 일치하는 패턴만 처리
	}
	return false; // 어떤 패턴도 일치하지 않음
}

std::pair<bool, std::optional<Mini_Error>> SyntaxChecker::TargetCheck(Target& target) {

	std::string _target = target.second;
	std::pair<bool, std::optional<Mini_Error>> result;

	ExpansionType et = DeduceExpansionType(target.second);
	if (et == ExpansionType::VariableRef) {
		if (!VariableRef_expend(_target)) {
			result.first = false;
			Mini_Error me({ "E104", Severity::Error });
			result.second = me;
			return result;
		}
	}

	else if (et == ExpansionType::Function) {
		if (!Function_expend(_target)) {
			result.first = false;
			Mini_Error me({ "E105", Severity::Error });
			result.second = me;
			return result;
		}
	}
	result.first = true;
	result.second = std::nullopt;
	return result;
}

std::pair<bool, std::optional<Mini_Error>> SyntaxChecker::PrerequisiteCheck(Prerequisite& prerequisite, 
	std::unordered_set<std::string>& result_preqs) {

	std::pair<bool, std::optional<Mini_Error>> result;


	std::string temp = prerequisite.second;
	ExpansionType et = DeduceExpansionType(prerequisite.second);
	if (et == ExpansionType::VariableRef && !VariableRef_expend(temp)) {
		result.first = false;
		Mini_Error me({ "E154", Severity::Error });
		result.second = me;
		return result;
	}
	if (et == ExpansionType::Function && !Function_expend(temp)) {
		result.first = false;
		Mini_Error me({ "E155", Severity::Error });
		result.second = me;
		return result;
	};

	std::vector<std::string> sp = (temp.find(' ') != std::string::npos)
		? SplitSpace(temp)
		: std::vector<std::string>{ temp };
	result_preqs.insert(sp.begin(), sp.end());

	// prerequisite 유효성 검사 및 패턴 처리

	if (fm.IsExistFile(prerequisite.second)) {
		result.first = true;
		result.second = std::nullopt;
		return result;
	}

	if (parser.GetTargets().find(prerequisite.second) != parser.GetTargets().end()) {
		result.first = true;
		result.second = std::nullopt;
		return result;
	}

	if (IsMatchingPattern(prerequisite.second)) {
		if (!AddPatternToDag(prerequisite.second)) {
			result.first = false;
			Mini_Error me({ "E151", Severity::Error });
			result.second = me;
			return result;
		}
		else {
			result.first = true;
			result.second = std::nullopt;
			return result;
		}
	}
	
	result.first = false;
	Mini_Error me({ "E151", Severity::Error });
	result.second = me;
	return result;
}

RecipeSet SyntaxChecker::MakeRecipeSet(Recipe& raw, int pos = 0) {
	RecipeSet rs;
	std::vector<SyntaxError> error_stack;

	rs.raw_recipe = raw;
	std::string raw_string = raw.second;

	std::string expend_result = "";
	std::vector<std::pair<int, std::string>> elements = SpaceColumnSet(raw_string, pos);

	for (const auto& [column, elem] : elements) {
		std::string expend = elem;
		ExpansionType et = DeduceExpansionType(elem);
		if (et == ExpansionType::VariableRef) {
			if (!VariableRef_expend(expend)) {
				SyntaxError e("E501", raw.first, column, elem.size(), Severity::Error);
				error_stack.push_back(e);
			}
			else {
				rs.recipe_map.emplace(elem, expend);
			}
		}

		else if (et == ExpansionType::Function) {
			if (!Function_expend(expend)) {
				SyntaxError e("E502", raw.first, column, elem.size(), Severity::Error);
				error_stack.push_back(e);
			}
			else {
				rs.recipe_map.emplace(expend, elem);
			}
		}

		expend_result = expend_result + " " + expend;
	}

	rs.expend_recipe = trim(expend_result);

	if (!error_stack.empty()) {
		throw error_stack;
	}

	return rs;
}

bool SyntaxChecker::RecipeCheck(std::optional<std::pair<int, Semi_colon>> semi_colon, std::vector<Recipe>& recipes) {

	if (semi_colon) {
		column pos = semi_colon->second.first;
		Recipe rp = { semi_colon->first, semi_colon->second.second };
		RecipeSet rs = MakeRecipeSet(rp, pos);
		precise_analysis_of_recipes(rs);
	}

	for (auto& recipe : recipes) {
		RecipeSet rs = MakeRecipeSet(recipe);
		precise_analysis_of_recipes(rs);
	}


	return true;
}

bool SyntaxChecker::precise_analysis_of_recipes(RecipeSet& rs){
	return true;
}


bool SyntaxChecker::ExplicitRuleCheck(Explicit_Rule& ex, bool toplevel){

	std::vector<SyntaxError> error_stack;

	{
		std::pair<bool, std::optional<Mini_Error>>target_res = TargetCheck(ex.target);

		if (!target_res.first && toplevel) {
			SyntaxError e(target_res.second->first, ex.line, ex.target.first, ex.target.second.size(), target_res.second->second);
			error_stack.push_back(e);
		}
		else if (!target_res.first && !toplevel) {
			return false;
		}
	}

	std::unordered_set<std::string> result_preqs;
	for (auto& preq : ex.prerequisite) {
		std::pair<bool, std::optional<Mini_Error>> preq_res = PrerequisiteCheck(preq, result_preqs);
		if (!preq_res.first && toplevel) {
			SyntaxError e(preq_res.second->first, ex.line, preq.first, preq.second.size(), preq_res.second->second);
			error_stack.push_back(e);
		}
		else if (!preq_res.first && !toplevel) {
			return false;
		}
	}

	if (ex.semi_colon_recipe.second  == "") {
		RecipeCheck(std::nullopt, ex.recipes);
	}
	else {
		std::pair<int, Semi_colon> semi = {ex.line, ex.semi_colon_recipe};
		RecipeCheck(semi, ex.recipes);
	}

	//throw
	if (!error_stack.empty()) {
		throw error_stack;
	}

	dag.AddEdge(ex.target.second, result_preqs);
	return true;
}

bool SyntaxChecker::MultipleTargetCheck(Multiple_Target& mt, bool toplevel){

	std::vector<SyntaxError> error_stack;

	for (auto& target : mt.targets) {
		std::pair<bool, std::optional<Mini_Error>>target_res = TargetCheck(target);

		if (!target_res.first && toplevel) {
			SyntaxError e(target_res.second->first, mt.line, target.first, target.second.size(), target_res.second->second);
			error_stack.push_back(e);
		}
		else if (!target_res.first && !toplevel) {
			return false;
		}
	}

	std::unordered_set<std::string> result_preqs;
	for (auto& preq : mt.prerequisite) {
		std::pair<bool, std::optional<Mini_Error>> preq_res = PrerequisiteCheck(preq, result_preqs);
		if (!preq_res.first && toplevel) {
			SyntaxError e(preq_res.second->first, mt.line, preq.first, preq.second.size(), preq_res.second->second);
			error_stack.push_back(e);
		}
		else if (!preq_res.first && !toplevel) {
			return false;
		}
	}

	for (const auto& target : mt.targets) {
		Explicit_Rule ex;
		ex.line = mt.line;
		ex.target = target;
		ex.prerequisite = mt.prerequisite;
		ex.semi_colon_recipe = mt.semi_colon_recipe;
		ex.recipes = mt.recipes;

		if (!ExplicitRuleCheck(ex, false)) {
			if (toplevel) {
				SyntaxError e("E105", mt.line, target.first, target.second.size(), Severity::Error);
				error_stack.push_back(e);
			}
			else {
				return false;
			}
		}
	}

	//throw
	if (!error_stack.empty()) {
		throw error_stack;
	}

	return true;
}

bool SyntaxChecker::PatternRuleCheck(Pattern_Rule& pr, bool toplevel){
	std::vector<SyntaxError> error_stack;
	if (pr.target_pattern.second.find('%') == std::string::npos) {
		if (toplevel) {
			SyntaxError e("E201", pr.line, pr.target_pattern.first, pr.target_pattern.second.size(), Severity::Error);
			error_stack.push_back(e);
		}
		else {
			return false;
		}
	}

	else if (SeparatorCounter(pr.target_pattern.second, '%') != 1) {
		if (toplevel) {
			SyntaxError e("E202", pr.line, pr.target_pattern.first, pr.target_pattern.second.size(), Severity::Error);
			error_stack.push_back(e);
		}
		else {
			return false;
		}
	}

	for (const auto& preq : pr.prerequisite_pattern) {
		if (preq.second.find('%') == std::string::npos) {
			continue;
		}

		else if (SeparatorCounter(preq.second, '%') > 1) {
			if (toplevel) {
				SyntaxError e("E203", pr.line, preq.first, preq.second.size(), Severity::Error);
				error_stack.push_back(e);
			}
			else {
				return false;
			}
		}
	}

	if (!error_stack.empty()) {
		throw error_stack;
	}


	return true;
}

bool SyntaxChecker::StaticPatternRuleCheck(Static_Pattern_Rule& spr, bool toplevel){
	std::vector<SyntaxError> error_stack;

	//target_pattern 검사
	if (spr.target_pattern.second.find('%') == std::string::npos) {
		if (toplevel) {
			SyntaxError e("E201", spr.line, spr.target_pattern.first, spr.target_pattern.second.size(), Severity::Error);
			error_stack.push_back(e);
		}
		else {
			return false;
		}
	}

	else if (SeparatorCounter(spr.target_pattern.second, '%') != 1) {
		if (toplevel) {
			SyntaxError e("E202", spr.line, spr.target_pattern.first, spr.target_pattern.second.size(), Severity::Error);
			error_stack.push_back(e);
		}
		else {
			return false;
		}
	}

	//target 집합들의 원소를 target_pattern에 맞는지 비교
	std::vector<std::string> matching_targets;
	for (const auto& target : spr.target) {
		if (!PatternMatching(target.second, spr.target_pattern.second)) {
			ec.AddError("E401", spr.line, target.first, target.second.size(), Severity::Warning);
		}
		else {
			matching_targets.push_back(target.second);
		}
	}

	std::vector<std::string> preq_patterns;
	std::vector<std::string> preq_files;

	for (const auto& preq : spr.prerequisite_pattern) {
		if (SeparatorCounter(preq.second, '%') > 1) {
			SyntaxError e("E202", spr.line, preq.first, preq.second.size(), Severity::Error);
			error_stack.push_back(e);
		}
		else {
			if (SeparatorCounter(preq.second, '%') == 1) {
				preq_patterns.push_back(preq.second);
			}
			else {
				preq_files.push_back(preq.second);
			}
		}
	}

	if (!error_stack.empty()) {
		throw error_stack;
	}

	for (const auto& target : matching_targets) {
		//stem 추출
		std::pair<std::string, std::string> target_split = SplitPattern(spr.target_pattern.second);
		std::string stem = ExtractStem(target, target_split);

		for (const auto& preq : preq_patterns) {
			std::pair<std::string, std::string> preq_split = SplitPattern(preq);
			preq_files.push_back(preq_split.first + stem + preq_split.second);
		}

		Explicit_Rule ex;
		ex.line = 0;
		ex.target = { 0, target };
		for (const auto& preq : preq_files) {
			ex.prerequisite.emplace_back(0, preq);
		}
		ex.recipes = spr.recipes;
		if (!ExplicitRuleCheck(ex, false)) {
			return false;
		}
	}

	return true;
}

ErrorCollector SyntaxChecker::GetError() {
	return ec;
}