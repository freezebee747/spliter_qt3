#pragma once
#include <functional>
#include <string>
#include <optional>

#include "syntax.h"

struct FunctionContext {
	Parser* parser = nullptr;
	SyntaxChecker* checker = nullptr;
	bool call_by_parser = false;
	bool call_by_syntax = false;
	// 필요시 더 추가
};

using FunctionHandler = std::function<std::string(const std::vector<std::string>&)>;
using ContextFunctionHandler = std::function<std::string(const std::vector<std::string>&, const FunctionContext&)>;
bool IsNeedFunctionContext(const std::string& function_name);

std::string ExtractFunctionName(const std::string& function);
std::vector<std::string> ExtractFunctionArguments(const std::string& function);

std::string patsubs(const std::string& wildcard, const std::string& changer, const std::string& target_str);
std::string Active_function(const std::string& function_name, const std::vector<std::string>& args, std::optional<FunctionContext> ctx = std::nullopt);

std::string function_subst(const std::string& from, const std::string& to, const std::string& text);
std::string function_patsubst(const std::string& pattern, const std::string& replacement, const std::string& text);
std::string function_strip(const std::string& string);
std::string function_findstring(const std::string& find, const std::string& in);
std::string function_filter(const std::string& pattern, const std::string& text);
std::string function_sort(const std::string& list);
std::string function_word(const std::string& n, const std::string& text);
std::string function_wordlist(const std::string& start, const std::string& end, const std::string& text);
std::string function_words(const std::string& text);
std::string function_firstword(const std::string& names);
std::string function_lastword(const std::string& names);
std::string function_dir(const std::string& names);
std::string function_notdir(const std::string& names);
std::string function_suffix(const std::string& names);
std::string function_basename(const std::string& names);
std::string function_addsuffix(const std::string& suffix, const std::string& names);
std::string function_addprefix(const std::string& prefix, const std::string& names);
std::string function_join(const std::string& list1, const std::string& list2);
std::string function_wildcard(const std::string& pattern);
std::string function_if(const std::string& condition, const std::string& then, const FunctionContext& ctx);
std::string function_if(const std::string& condition, const std::string& then, const std::string& _else, const FunctionContext& ctx);
std::string function_or(const std::vector<std::string>& conditions);
