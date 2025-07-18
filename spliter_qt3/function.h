#pragma once
#include <iostream>
#include <functional>
//#include "file.h"

std::string ExtractFunctionName(const std::string& function);
std::vector<std::string> ExtractFunctionArguments(const std::string& function);

std::string patsubs(const std::string& wildcard, const std::string& changer, const std::string& target_str);
std::string Active_function(const std::string& function_name, const std::vector<std::string>& args);

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
std::string function_if(const std::string& condition, const std::string& then);
std::string function_if(const std::string& condition, const std::string& then, const std::string& _else);
std::string function_or(const std::vector<std::string>& conditions);
