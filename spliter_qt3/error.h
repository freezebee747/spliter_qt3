#pragma once

#include <string>
#include <iostream>
#include <vector>
#include <unordered_map>

#include "AST.h"

enum class Severity { Warning, Error, Fatal };

struct Error {
	std::string external_filename = "";
	std::string code;
	int line;
	int column;
	int size;
	Severity severity;
};

class ErrorMessage {
private:
	std::unordered_map<std::string, std::string> messages;
	ErrorMessage();

public:
	static const ErrorMessage& GetInstance();
	std::string GetMessage(const std::string& code) const;
};

class ErrorCollector {
private:
	std::vector<Error> errors;
public:
	void AddError(const std::string code, int line, int column, int size, Severity sev);
	void Add(const Error& e) { errors.push_back(e); }
	const std::vector<Error>& GetAll() const { return errors; }
	void SetExternalErrors(const std::string& filename);
	void AppendErrorCollector(ErrorCollector& ec);
	void clearError();
};
