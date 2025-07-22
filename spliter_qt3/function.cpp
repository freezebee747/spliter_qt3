#include <algorithm>
#include <regex>
#include <cassert>

#include "function.h"
#include "read.h"



std::string ExtractFunctionName(const std::string& function) {
	std::string result = "";
	if (IsFunction(function)) {
		result = safe_substr(function, 2, function.find(" ") - 2);
	}
	return result;
}

std::vector<std::string> ExtractFunctionArguments(const std::string& function) {
	std::vector<std::string> result;
	if (IsFunction(function)) {
		std::string args = safe_substr(function, function.find(" ") + 1, function.size() - function.find(" ") - 2);
		result = SplitComma(args);
		std::string filename = ExtractFunctionName(function);
		if (filename == "let" && filename == "file") {
			std::vector<std::string> result2 = SplitSpace(result[0]);
			result.insert(result.begin(), result2.begin(), result2.end());
		}
	}
	return result;
}

static std::unordered_map<std::string, FunctionHandler> function_map = {
	{"subst", [](const std::vector<std::string>& args) -> std::string {
		if (args.size() >= 3) return function_subst(args[0], args[1], args[2]);
		return ""; // or throw error
	}},
	{"patsubst", [](const std::vector<std::string>& args) -> std::string {
		if (args.size() >= 3) return function_patsubst(args[0], args[1], args[2]);
		return "";
	}},
	{"strip", [](const std::vector<std::string>& args) -> std::string {
		if (!args.empty()) return function_strip(args[0]);
		return "";
	}},
	{"findstring", [](const std::vector<std::string>& args) -> std::string {
		if (args.size() >= 2) return function_findstring(args[0], args[1]);
		return "";
	}},
	{"filter", [](const std::vector<std::string>& args) -> std::string {
		if (args.size() >= 2) return function_filter(args[0], args[1]);
		return "";
	}},
	{"sort", [](const std::vector<std::string>& args)-> std::string {
		 if (args.size() >= 1) return function_sort(args[0]);
		 return "";
	}},
	{"word",[](const std::vector<std::string>& args)->std::string {
		if (args.size() >= 2) return function_word(args[0], args[1]);
		return "";
	}},
	{"wordlist", [](const std::vector<std::string>& args)->std::string {
		if (args.size() >= 3) return function_wordlist(args[0], args[1], args[2]);
		return "";
	}},
	{"words",[](const std::vector<std::string>& args)->std::string {
		if (args.size() >= 1) return function_words(args[0]);
		return "";
	}},
	{"firstword", [](const std::vector<std::string>& args)->std::string {
		if (args.size() >= 1) return function_firstword(args[0]);
		return "";
	}},
	{"lastword", [](const std::vector<std::string>& args)->std::string {
		if (args.size() >= 1) return function_firstword(args[0]);
		return "";
	}},
	{"dir", [](const std::vector<std::string>& args)->std::string {
		if (args.size() >= 1) return function_dir(args[0]);
		return "";
	}},
	{"notdir", [](const std::vector<std::string>& args)-> std::string {
		if (args.size() >= 1) return function_notdir(args[0]);
		return "";
	}},
	{"suffix", [](const std::vector<std::string>& args)->std::string {
		if (args.size() >= 1) return function_suffix(args[0]);
		return "";
	}},
	{"basename", [](const std::vector<std::string>& args)->std::string {
		if (args.size() >= 1) return function_basename(args[0]);
		return "";
	}},
	{"addsuffix", [](const std::vector<std::string>& args)->std::string {
		if (args.size() >= 2) return function_addsuffix(args[0], args[1]);
		return "";
	}},
	{"addprefix",[](const std::vector<std::string>& args)->std::string {
		if (args.size() >= 2) return function_addprefix(args[0], args[1]);
		return "";
	}},
	{"join", [](const std::vector<std::string>& args)->std::string {
		if (args.size() >= 2) return function_join(args[0], args[1]);
		return "";
	}},
	{"wildcard", [](const std::vector<std::string>& args)->std::string {
		if (args.size() >= 1) return function_basename(args[0]);
		return "";
	}},
	// 추가
};

//여기서 부터 수정
std::unordered_map<std::string, ContextFunctionHandler> context_function_map = {
	{"if", [](const std::vector<std::string>& args, const FunctionContext& ctx)->std::string {
		if (args.size() == 2) return function_if(args[0], args[1], ctx);
		else if (args.size() > 2) return function_if(args[0], args[1], args[2] ,ctx);

		return "";
	}}
};


bool IsNeedFunctionContext(const std::string& function_name) {
	auto that = context_function_map.find(function_name);
	if (that != context_function_map.end()) {
		return true;
	}
	return false;
}

std::string Active_function(const std::string& function_name, const std::vector<std::string>& args, std::optional<FunctionContext> ctx) {

	auto it = function_map.find(function_name);
	if (it != function_map.end()) {
		return it->second(args);
	}

	auto that = context_function_map.find(function_name);
	if (that != context_function_map.end() && ctx != std::nullopt) {
		return that->second(args, *ctx);
	}

	return "";

}

std::string patsubs(const std::string& wildcard, const std::string& changer, const std::string& target_str) {
	if (hasWhitespace(wildcard)) return std::string();

	size_t pos = wildcard.find('%');
	if (pos == std::string::npos) return std::string();

	std::string begin = safe_substr(wildcard, 0, pos);
	std::string end = safe_substr(wildcard, pos + 1, wildcard.size() - pos - 1);

	size_t cpos = changer.find('%');
	if (cpos == std::string::npos) return std::string();

	std::string cbegin = safe_substr(changer, 0, cpos);
	std::string cend = safe_substr(changer, cpos + 1, changer.size() - cpos - 1);

	if (target_str.size() < begin.size() + end.size()) return target_str;

	if (target_str.substr(0, begin.size()) == begin &&
		target_str.substr(target_str.size() - end.size()) == end) {

		std::string stem = target_str.substr(
			begin.size(), target_str.size() - begin.size() - end.size()
		);

		return cbegin + stem + cend;
	}

	return target_str;
}

std::string function_subst(const std::string& from, const std::string& to, const std::string& text) {
	int begin = 0;
	std::string return_string = text;
	while (text.find(from, begin) != std::string::npos) {
		begin = text.find(from, begin);
		return_string.replace(begin, from.size(), to);
		begin += to.size();
	}
	return return_string;
}

std::string function_patsubst(const std::string& pattern, const std::string& replacement, const std::string& text) {
	std::string result;
	std::vector<std::string> temp = SplitSpace(text);
	for (const auto& i : temp) {
		result = result + ' ' + patsubs(pattern, replacement, i);
	}

	return trim(result);

}

std::string function_strip(const std::string& string) {
	std::vector<std::string> temp = SplitSpace(string);
	std::string result;
	for (const auto& i : temp) {
		result = result + ' ' + i;
	}
	return trim(result);
}

std::string function_findstring(const std::string& find, const std::string& in) {
	if (in.find(find)) {
		return find;
	}
	return "";
}

std::string function_filter(const std::string& patterns, const std::string& text) {
	std::vector<std::string> pattern = SplitSpace(patterns);
	std::vector<std::string> texts = SplitSpace(text);
	std::string result;
	for (const auto& i : pattern) {
		for (const auto& j : texts) {

			size_t pos = i.find('%');
			if (pos == std::string::npos) return std::string();

			std::string begin = safe_substr(i, 0, pos);
			std::string end = safe_substr(i, pos + 1, i.size() - pos - 1);

			if (j.substr(0, begin.size()) == begin &&
				j.substr(j.size() - end.size()) == end) {
				result = result + ' ' + j;
			}

		}
	}
	return trim(result);
}

std::string function_sort(const std::string& list) {
	std::string result;
	std::vector<std::string> elems = SplitSpace(list);
	std::sort(elems.begin(), elems.end());
	for (const auto& elem : elems) {
		result = result + ' ' + elem;
	}
	return trim(result);
}

std::string function_word(const std::string& n, const std::string& text) {
	int num;
	try {
		num = std::stoi(n);
	}
	catch (const std::invalid_argument& e) {
		// 숫자가 아닌 입력 처리
		return "Invalid number format: " + n;
	}
	catch (const std::out_of_range& e) {
		// 숫자가 int 범위를 넘을 경우 처리
		return "Number out of range: " + n;
	}

	std::vector<std::string> texts = SplitSpace(text);
	if (num < 0 || num > static_cast<int>(texts.size())) {
		return "Index out of bounds: " + std::to_string(num);
	}

	return texts[num - 1];
}

std::string function_wordlist(const std::string& start, const std::string& end, const std::string& text) {
	std::vector<std::string> texts = SplitSpace(text);
	std::string result;
	int num1;
	int num2;
	try {
		num1 = std::stoi(start);
	}
	catch (const std::invalid_argument& e) {
		// 숫자가 아닌 입력 처리
		return "Invalid number format: " + start;
	}
	catch (const std::out_of_range& e) {
		// 숫자가 int 범위를 넘을 경우 처리
		return "Number out of range: " + start;
	}

	try {
		num2 = std::stoi(end);
	}
	catch (const std::invalid_argument& e) {
		// 숫자가 아닌 입력 처리
		return "Invalid number format: " + end;
	}
	catch (const std::out_of_range& e) {
		// 숫자가 int 범위를 넘을 경우 처리
		return "Number out of range: " + end;
	}


	for (int i = num1 - 1; i < num2; i++) {
		result = result + ' ' + texts[i];
	}
	return trim(result);

}

std::string function_words(const std::string& text) {
	std::vector<std::string> texts = SplitSpace(text);
	int num = texts.capacity();
	return std::to_string(num);

}

std::string function_firstword(const std::string& names) {
	std::vector<std::string> name = SplitSpace(names);
	return name[0];
}

std::string function_lastword(const std::string& names) {
	std::vector<std::string> name = SplitSpace(names);
	return name[name.size() - 1];
}

std::string function_dir(const std::string& names) {
	std::vector<std::string> name = SplitSpace(names);
	std::string result;
	for (const auto& i : name) {
		int num = i.find_first_of("/");
		std::string temp;
		if (num != std::string::npos) {
			temp = safe_substr(i, 0, num + 1);
		}
		else {
			temp = "./";
		}

		result = result + " " + temp;
	}
	return trim(result);
}

std::string function_notdir(const std::string& names) {
	std::vector<std::string> name = SplitSpace(names);
	std::string result;
	for (const auto& i : name) {
		int num = i.find_last_of("/");
		std::string temp;
		if (num != std::string::npos) {
			temp = safe_substr(i, num, i.size() - num);
		}
		else {
			temp = i;
		}
		result = result + " " + temp;
	}
	return trim(result);
}

std::string function_suffix(const std::string& names) {
	std::vector<std::string> name = SplitSpace(names);
	std::string result;
	for (const auto& i : name) {
		int num = i.find_last_of("/");
		std::string temp;
		if (num != std::string::npos) {
			num = i.find_last_of(".");
			temp = safe_substr(i, num, i.size());
			result = result + " " + temp;
		}
	}
	return trim(result);
}

std::string function_basename(const std::string& names) {
	std::vector<std::string> name = SplitSpace(names);
	std::string result;
	for (const auto& i : name) {
		int num1 = i.find_last_of("/");
		std::string temp;
		if (num1 != std::string::npos) {
			int num2 = i.find_last_of(".");
			if (num2 != std::string::npos && num1 < num2) {
				temp = safe_substr(i, 0, num2);
			}
			else temp = i;
		}
		else temp = i;

		result = result + " " + temp;
	}
	return trim(result);
}

std::string function_addsuffix(const std::string& suffix, const std::string& names) {
	if (suffix[0] != '.') return "";

	std::vector<std::string> name = SplitSpace(names);
	std::string result;
	std::string temp;
	for (const auto& i : name) {
		temp = i + suffix;
		result = result + " " + temp;
	}
	return trim(result);
}

std::string function_addprefix(const std::string& prefix, const std::string& names) {
	if (prefix.back() != '/') return "";

	std::vector<std::string> name = SplitSpace(names);
	std::string result;
	std::string temp;
	for (const auto& i : name) {
		temp = prefix + i;
		result = result + " " + temp;
	}
	return trim(result);

}

std::string function_join(const std::string& list1, const std::string& list2) {
	std::vector<std::string> first_list = SplitSpace(list1);
	std::vector<std::string> second_list = SplitSpace(list2);
	int round = (first_list.size() > second_list.size()) ? second_list.size() : first_list.size();
	std::string result;
	for (int i = 0; i < round; i++) {
		result = result + " " + first_list[i] + second_list[i];
	}
	if (first_list.size() > second_list.size()) {
		for (int i = round; i < first_list.size(); i++) {
			result = result + " " + first_list[i];
		}
	}
	else if (first_list.size() < second_list.size()) {
		for (int i = round; i < second_list.size(); i++) {
			result = result + " " + second_list[i];
		}
	}
	return trim(result);
}

std::string function_if(const std::string& condition, const std::string& then, const FunctionContext& ctx) {

	assert(!(ctx.call_by_parser && ctx.call_by_syntax));

	std::string temp = condition;
	bool result = false;
	ExpansionType et = DeduceExpansionType(condition);
	std::unordered_set<std::string> visit;

	auto eval_var = [&](std::string& ref) -> bool {
		if (ctx.call_by_parser && ctx.parser)
			return ctx.parser->VariableRef_expend(ref, visit);
		else if (ctx.call_by_syntax && ctx.checker)
			return ctx.checker->VariableRef_expend(ref);
		return false;
		};

	auto eval_func = [&](std::string& ref) -> bool {
		if (ctx.call_by_parser && ctx.parser)
			return ctx.parser->Function_expend(ref);
		else if (ctx.call_by_syntax && ctx.checker)
			return ctx.checker->Function_expend(ref);
		return false;
		};

	if (et == ExpansionType::VariableRef)
		result = eval_var(temp);
	else if (et == ExpansionType::Function)
		result = eval_func(temp);
	else if (et == ExpansionType::Literal && !condition.empty())
		return then;
	else
		return "";

	return result ? then : "";
}

std::string function_if(const std::string& condition, const std::string& then, const std::string& _else, const FunctionContext& ctx) {
	return function_if(condition, then, ctx).empty() ? _else : then;
}

std::string function_or(const std::vector<std::string>& conditions) {
	return "";
}


std::string function_wildcard(const std::string& pattern) {
	std::vector<std::string> filenames = SearchFilesInWorkingDirectory();
	std::vector<std::string> patterns = SplitSpace(pattern);
	std::string result;
	for (const auto& ptn : patterns) {
		std::regex rx(glob_to_regex(ptn));
		for (const auto& files : filenames) {
			if (std::regex_match(files, rx)) {
				result = result + " " + files;
			}
		}
	}

	return trim(result);
}

//std::string function_if(const std::string& condition, const std::string& then) {
//	std::unordered_map<std::string, std::string> var = SyntaxChecker::GetVariables();
//	auto i = var.find(condition);
//	if (i != var.end()) {
//		std::string temp = then;
//		if (IsFunction(temp)) {
//			std::string function_name = ExtractFunctionName(temp);
//			std::vector<std::string> function_arg = ExtractFunctionArguments(temp);
//			return Active_function(function_name, function_arg);
//		}
//		else if (IsVariable(temp)) {
//			return var.find(temp)->second;
//		}
//		else {
//			return then;
//		}
//	}
//	else return "";
//}
//
//std::string function_if(const std::string& condition, const std::string& then, const std::string& _else) {
//	std::unordered_map<std::string, std::string> var = SyntaxChecker::GetVariables();
//	auto i = var.find(condition);
//	if (i != var.end()) {
//		std::string temp = then;
//		if (IsFunction(temp)) {
//			std::string function_name = ExtractFunctionName(temp);
//			std::vector<std::string> function_arg = ExtractFunctionArguments(temp);
//			return Active_function(function_name, function_arg);
//		}
//		else if (IsVariable(temp)) {
//			return var.find(temp)->second;
//		}
//		else {
//			return then;
//		}
//	}
//	else {
//		std::string temp = _else;
//		if (IsFunction(temp)) {
//			std::string function_name = ExtractFunctionName(temp);
//			std::vector<std::string> function_arg = ExtractFunctionArguments(temp);
//			return Active_function(function_name, function_arg);
//		}
//		else if (IsVariable(temp)) {
//			return var.find(temp)->second;
//		}
//		else {
//			return _else;
//		}
//	}
//}
//
//std::string function_or(const std::vector<std::string>& conditions) {
//	std::unordered_map<std::string, std::string> var = SyntaxChecker::GetVariables();
//	for (const auto& i : conditions) {
//		auto finder = var.find(i);
//		if (finder != var.end() && finder->second != "") {
//			return finder->second;
//		}
//	}
//	return "";
//}

//나중에 정의
//$(and condition1[,condition2[,condition3…]])
//std::string function_and(const std::vector<std::string>& conditions)
// 
//$(intcmp lhs,rhs[,lt-part[,eq-part[,gt-part]]])
//std::string function_intcmp

