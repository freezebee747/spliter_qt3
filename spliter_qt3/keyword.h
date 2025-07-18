#pragma once
#include <unordered_set>
#include <string>

inline const std::unordered_set<std::string> functions = {
	"let", "foreach", "file", "call", "value", "eval",
	"origin", "flaver", "shell", "guile", "patsubst",
	"subst", "filter", "sort", "word", "wordlist", "words",
	"firstword", "lastword", "dir", "notdir", "suffix",
	"basename", "addsuffix", "addprefix", "join", "wildcard",
	"if"
};

inline const std::unordered_set<std::string> directive_set = {
	"include", "-include", "vpath", "override"
};

inline const std::unordered_set<std::string> special_target = {
	".PHONY", ".SUFFIXES", ".DEFAULT", ".PRECIOUS", ".INTERMEDIATE",
	".NOTINTERMEDIATE", ".SECONDARY", ".SECONDEXPANSION", ".DELETE_ON_ERROR",
	".IGNORE", ".LOW_RESOLUTION_TIME", ".SILENT", ".EXPORT_ALL_VARIABLES", ".NOTPARALLEL",
	".ONESHELL", ".POSIX"
};

