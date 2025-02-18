#include "AdjacencyRulesLoader.h"
#include "pugixml.hpp"

#include <iostream>

using namespace pugi;

// Load adjacency rules from XML
AdjacencyRules load_adjacency_rules(const string& filename)
{
	AdjacencyRules rules;
	xml_document doc;
	
	if (!doc.load_file(filename.c_str()))
	{
		std::cerr << "Failed to load XML file: " << filename << std::endl;
		return rules;
	}
	
	xml_node set_parent = doc.child("set");
	xml_node adjacency_rules_parent = set_parent.child("adjacency_rules");
	
	for (xml_node tile_rules : adjacency_rules_parent.children("tile"))
	{
		string tile_name = tile_rules.attribute("name").value();
		
		for (xml_node side : tile_rules.children("side"))
		{
			string side_name = side.attribute("name").value();
			vector<string> allowed_neighbors;
			
			for (xml_node neighbor : side.children("neighbor"))
			{
				allowed_neighbors.push_back(neighbor.child_value());
			}
			
			rules[tile_name][side_name] = allowed_neighbors;
		}
	}
	
	return rules;
}

// Print adjacency rules in a human-readable format
void print_rules(const AdjacencyRules& rules)
{
	for (const auto& [tile, sides] : rules)
	{
		std::cout << tile << ":\n";
		for (const auto& [side, neighbors] : sides)
		{
			std::cout << " " << side << ": [";
			for (size_t i = 0; i < neighbors.size(); ++i)
			{
				std::cout << neighbors[i];
				if (i < neighbors.size() - 1)
				{
					std::cout << ", ";
				}
				else
				{
					std::cout << "]\n";
				}
			}
		}
	}
}
