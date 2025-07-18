#include "read.h"
#include "keyword.h"

//$(var)들을 각각 나눈다
//예로 "$(var1) $(var2)" 문자열이 입력으로 들어왔다면, $(var1)과 $(var2)로 나누고 그것의 집합을 리턴한다.
//함수의 경우 마지막 값이 입력값이다.
std::vector<std::string> SplitValues(const std::string& target) {
	int depth = 0;
	int count = 0;
	int begin = 0;
	std::vector<std::string> result;

	while (count < target.size()) {

		if (target[count] == '$' && target[count + 1] == '(' && count + 1 < target.size()) {
			if (depth == 0) {
				std::string str = target.substr(begin, count - begin);
				if (str != "") {
					result.push_back(str);
					begin = count;
				}

			}
			count += 2;
			depth++;
		}

		else if (target[count] == ')') {
			depth--;

			if (depth == 0) {
				std::string str = target.substr(begin, count - begin + 1);
				result.push_back(str);
				count++;
				begin = count;
			}
			else
				count++;
		}
		else count++;

	}
	std::string remind = target.substr(begin, target.size() - begin);
	result.push_back(remind);
	return result;
}

int SeparatorCounter(const std::string& target, char sep) {

	int count = 0;
	std::string temp = target;
	while (true) {
		int sep_pos = temp.find_first_of(sep);
		if (sep_pos == std::string::npos) break;
		count++;
		temp = temp.substr(sep_pos + 1);
	}

	return count;
}

unsigned VariableCounter(const std::string& var) {
	unsigned counter = 0;
	size_t dollar_left_bracket = var.find("$(");
	size_t right_bracket = var.find(')');

	if (dollar_left_bracket == std::string::npos || right_bracket == std::string::npos) {
		return 0;
	}

	while (dollar_left_bracket != std::string::npos && right_bracket != std::string::npos) {
		if (dollar_left_bracket > right_bracket) {
			right_bracket = var.find(')', dollar_left_bracket);
			continue;
		}

		counter++;
		dollar_left_bracket = var.find("$(", dollar_left_bracket + 1);
		right_bracket = var.find(')', right_bracket + 1);
	}
	return counter;
}



std::string ReplaceVariable(std::vector<std::string>& rep, const std::string& target) {
	std::string temp = target;
	int rep_counter = 0;
	std::vector<std::string> split_by_value = tokenizeMakefileStyle(target);
	if (rep.size() < VariableCounter(target)) return "";

	size_t dollar_left_bracket = temp.find("$(");
	size_t right_bracket = temp.find(')');
	while (dollar_left_bracket != std::string::npos && right_bracket != std::string::npos) {
		if (dollar_left_bracket > right_bracket) {
			right_bracket = temp.find(')', dollar_left_bracket);
			continue;
		}
		int counter = right_bracket - dollar_left_bracket;
		temp.replace(dollar_left_bracket, counter + 1, rep[rep_counter]);
		rep_counter++;

		dollar_left_bracket = temp.find("$(");
		right_bracket = temp.find(')');
	}
	return temp;
}

// 왼쪽 공백 제거
std::string ltrim(const std::string& s) {
	size_t start = s.find_first_not_of(" \t\n\r");
	return (start == std::string::npos) ? "" : s.substr(start);
}

// 오른쪽 공백 제거
std::string rtrim(const std::string& s) {
	size_t end = s.find_last_not_of(" \t\n\r");
	return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

// 양쪽 공백 제거
std::string trim(const std::string& s) {
	return rtrim(ltrim(s));
}

//토큰 이전의 마지막 문자에서 부터 토큰 까지의 공백의 개수
int calc_space(const std::string& str, char token, int pos) {
	int token_length = str.find(token, pos);
	if (token_length == std::string::npos) {
		return 0;
	}
	if (str[token_length - 1] != ' ') return 0;
	int temp = token_length - 1;
	while (temp >= 0) {
		if (str[temp] == ' ') {
			temp--;
		}
		else break;
	}

	return token_length - temp;
}

std::string safe_substr(const std::string& str, size_t pos, size_t count) {
	if (pos == 0 && count == 0) {
		return "";
	}
	if (pos >= str.size()) {
		return "";  // pos가 범위를 벗어나면 빈 문자열
	}
	return str.substr(pos, count);
}

std::vector<std::string> SplitSpace(const std::string& target) {
	std::istringstream iss(target);
	std::vector<std::string> tokens;
	std::string token;
	while (iss >> token) {
		tokens.push_back(token);
	}
	return tokens;
}

std::vector<std::string> SplitComma(const std::string& target) {
	std::stringstream ss(target);
	std::vector<std::string> tokens;
	std::string item;

	while (std::getline(ss, item, ',')) {
		tokens.push_back(trim(item));
	}

	return tokens;
}

std::vector<std::string> tokenizeMakefileStyle(const std::string& input) {
	std::vector<std::string> tokens;
	size_t i = 0;
	std::string temp = "";

	while (i < input.size()) {
		if (input[i] == '$' && i + 1 < input.size() && input[i + 1] == '(') {
			if (temp != "") {
				tokens.push_back(temp);
				temp = "";
			}
			// Start of $() block
			size_t start = i;
			i += 2; // Skip "$("
			int depth = 1;
			while (i < input.size() && depth > 0) {
				if (input[i] == '(') depth++;
				else if (input[i] == ')') depth--;
				i++;
			}
			tokens.push_back(input.substr(start, i - start));
		}
		else {
			// Regular character
			temp = temp + input[i];
			i++;
		}
	}

	return tokens;
}
bool hasWhitespace(const std::string& str) {
	for (char ch : str) {
		if (std::isspace(static_cast<unsigned char>(ch))) {
			return true;
		}
	}
	return false;
}

std::string join(const std::vector<std::string>& vec, const std::string& delimiter) {
	std::string result;
	for (size_t i = 0; i < vec.size(); ++i) {
		result += vec[i];
		if (i != vec.size() - 1) {
			result += delimiter;
		}
	}
	return result;
}

bool IsVariable(const std::string& str)
{
	if (IsFunction(str)) return false;
	if (str[0] == '$' && str[1] == '(' && str.back() == ')') {
		return true;
	}
	else if (str[0] == '$' && str[1] == '$' && str[2] == '(' && str.back() == ')') {
		return true;
	}
	return false;
}

bool IsFunction(const std::string& str) {
	if (str[0] == '$' && str[1] == '(' && str.back() == ')') {
		std::string temp = str.substr(2, str.size() - 3);
		temp = SplitSpace(temp)[0];
		auto i = functions.find(temp);
		if (i != functions.end()) {
			return true;
		}
	}
	return false;
}

std::vector<std::pair<unsigned, std::string>> ReadFileWithLineNumbers(const std::string& text) {
	std::vector<std::pair<unsigned, std::string>> lines;
	std::istringstream iss(text);
	std::string line;
	unsigned line_number = 1;

	while (std::getline(iss, line)) {
		lines.emplace_back(line_number++, line);
	}

	return lines;
}

std::vector<std::pair<unsigned, std::string>> JoinSplitLine(std::vector<std::pair<unsigned, std::string>>& raw) {
	std::vector<std::pair<unsigned, std::string>> combinedLines;
	for (size_t i = 0; i < raw.size(); ++i) {
		const auto& [lineNum, lineText] = raw[i];
		std::string combined = lineText;
		unsigned originalLine = lineNum;

		while (!combined.empty() && combined.back() == '\\') {
			combined.pop_back();
			if (++i < raw.size()) {
				combined += trim(raw[i].second);
			}
			else {
				break;
			}
		}
		combinedLines.emplace_back(originalLine, combined);
	}
	return combinedLines;
}

std::pair<std::string, std::string> SplitPattern(const std::string& pattern) {
	std::string prefix;
	std::string suffix;
	std::pair<std::string, std::string> result;
	size_t PercentSign = 0;

	if (pattern.find('%') == std::string::npos) {
		return std::pair<std::string, std::string>();
	}
	PercentSign = pattern.find('%');

	prefix = safe_substr(pattern, 0, PercentSign);
	suffix = safe_substr(pattern, PercentSign + 1, pattern.size() - PercentSign - 1);

	result = { prefix, suffix };
	return result;
}

bool PatternMatching(const std::string& target, const std::string& pattern) {
	std::pair<std::string, std::string> split = SplitPattern(pattern);
	std::string target_prefix = safe_substr(target, 0, split.first.size());
	std::string target_suffix = safe_substr(target, target.size() - split.second.size(), split.second.size());

	if (split.first == target_prefix && split.second == target_suffix) {
		return true;
	}
	return false;
}

std::string ExtractStem(std::string target, std::pair<std::string, std::string>& pattern) {
	std::string result;
	size_t prefix = pattern.first.size();
	size_t suffix = pattern.second.size();

	std::string target_prefix = target.substr(0, prefix);
	std::string target_suffix = target.substr(target.size() - suffix, suffix);

	if (target_prefix != pattern.first || target_suffix != pattern.second) {
		return "";
	}
	result = target.substr(prefix, target.size() - prefix - suffix);

	return result;
}
