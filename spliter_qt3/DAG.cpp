#include "DAG.h"

void DirectedAcyclicGraph::AddEdge(const std::string& target, const std::string& prerequisite) {
    dag[target].insert(prerequisite);
}

void DirectedAcyclicGraph::AddEdge(const std::string& target, const std::unordered_set<std::string>& prerequisites) {
    for (const auto& pre : prerequisites) {
        dag[target].insert(pre);  // 자동으로 없으면 생성됨
    }
}


std::vector<std::string> DirectedAcyclicGraph::FindLeafNodes() {
    std::unordered_set<std::string> all_nodes;
    std::unordered_set<std::string> targets;

    for (const auto& [target, prereqs] : dag) {
        targets.insert(target);
        all_nodes.insert(target);
        for (const auto& p : prereqs) {
            all_nodes.insert(p);
        }
    }

    std::vector<std::string> leaf_nodes;
    for (const auto& node : all_nodes) {
        if (targets.find(node) == targets.end()) {
            leaf_nodes.push_back(node);
        }
    }
    return leaf_nodes;
}

void DirectedAcyclicGraph::print_dag() {
    for (const auto& [key, value] : dag) {
        std::cout << key << ": ";
        for (const auto& i : value) {
            std::cout << i << " ";
        }
        std::cout << '\n';
    }
}

