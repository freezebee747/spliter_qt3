#pragma once
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <string>

class DirectedAcyclicGraph {
private:
	std::unordered_map<std::string, std::unordered_set<std::string>> dag;
public:
	void AddEdge(const std::string& target, const std::string& prerequisite);
	void AddEdge(const std::string& target, const std::unordered_set<std::string>& prerequisites);
	std::vector<std::string> FindLeafNodes();
	void print_dag();
};