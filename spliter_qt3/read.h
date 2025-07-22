#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <sstream>


std::vector<std::string> SplitValues(const std::string& target);
int SeparatorCounter(const std::string& target, char sep);

// 왼쪽 공백 제거
std::string ltrim(const std::string& s);
// 오른쪽 공백 제거
std::string rtrim(const std::string& s);
// 양쪽 공백 제거
std::string trim(const std::string& s);
//토큰 이전의 마지막 문자에서 부터 토큰 까지의 공백의 개수
int calc_space(const std::string& str, char token, int pos);

std::string safe_substr(const std::string& str, size_t pos, size_t count);
std::vector<std::string> SplitSpace(const std::string& target);
std::vector<std::string> SplitComma(const std::string& target);
std::vector<std::string> tokenizeMakefileStyle(const std::string& input);
std::pair<std::string, std::string> SplitPattern(const std::string& pattern);
bool PatternMatching(const std::string& target, const std::string& pattern);
std::string ExtractStem(std::string target, std::pair<std::string, std::string>& pattern);
std::vector<std::pair<int, std::string>> SpaceColumnSet(const std::string& str, int pos = 0);
bool hasWhitespace(const std::string& str);
std::string join(const std::vector<std::string>& vec, const std::string& delimiter);

bool IsVariable(const std::string& str);
bool IsFunction(const std::string& str);

unsigned VariableCounter(const std::string& var);

std::string ReplaceVariable(std::vector<std::string>& rep, const std::string& target);

std::vector<std::pair<unsigned, std::string>> ReadFileWithLineNumbers(const std::string& text);
std::vector<std::pair<unsigned, std::string>> JoinSplitLine(std::vector<std::pair<unsigned, std::string>>& raw);

