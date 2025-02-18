#pragma once

#include <unordered_map>
#include <vector>
#include <string>

using std::string;
using std::vector;
using std::unordered_map;

using AdjacencyRules = unordered_map<string, unordered_map<string, vector<string>>>;

AdjacencyRules load_adjacency_rules(const string& filename);
void print_rules(const AdjacencyRules& rules);
