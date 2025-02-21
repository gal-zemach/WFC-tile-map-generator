#include "TileSet.h"

#include <filesystem>
#include <iostream>

#include "pugixml.hpp"
#include "ofMain.h"

using pugi::xml_node;
using pugi::xml_document;

// XML attribute names
static constexpr const char* SET_ATTRIBUTE_NAME = "set";
static constexpr const char* ADJACENCY_RULES_ATTRIBUTE_NAME = "adjacency_rules";
static constexpr const char* TILE_ATTRIBUTE_NAME = "tile";
static constexpr const char* NAME_ATTRIBUTE_NAME = "name";
static constexpr const char* SIDE_ATTRIBUTE_NAME = "side";
static constexpr const char* NEIGHBOR_ATTRIBUTE_NAME = "neighbor";

TileSet::TileSet(const string& xml_path, const string& images_folder_path)
{
	images = load_set_images(images_folder_path);
	adjacency = load_adjacency_rules(xml_path);
}

TileSet::TileImages TileSet::load_set_images(const string& images_folder_path)
{
	TileImages images;
	
	if (!std::filesystem::exists(images_folder_path.c_str()))
	{
		std::cerr << "Failed to find folder: " << images_folder_path << std::endl;
		return images;
	}
	
	for (const auto& file : std::filesystem::directory_iterator(images_folder_path))
	{
		if (file.is_regular_file() && file.path().extension() == ".png")
		{
			images[file.path().stem()] = ofImage(file.path());
		}
	}
	
	return images;
}

TileSet::AdjacencyRules TileSet::load_adjacency_rules(const string& xml_path)
{
	AdjacencyRules rules;
	xml_document doc;
	
	if (!doc.load_file(xml_path.c_str()))
	{
		std::cerr << "Failed to load XML file: " << xml_path << std::endl;
		return rules;
	}
	
	xml_node set_parent = doc.child(SET_ATTRIBUTE_NAME);
	xml_node adjacency_rules_parent = set_parent.child(ADJACENCY_RULES_ATTRIBUTE_NAME);
	
	for (xml_node tile_rules : adjacency_rules_parent.children(TILE_ATTRIBUTE_NAME))
	{
		string tile_name = tile_rules.attribute(NAME_ATTRIBUTE_NAME).value();
		
		for (xml_node side : tile_rules.children(SIDE_ATTRIBUTE_NAME))
		{
			string side_name = side.attribute(NAME_ATTRIBUTE_NAME).value();
			vector<string> allowed_neighbors;
			
			for (xml_node neighbor : side.children(NEIGHBOR_ATTRIBUTE_NAME))
			{
				allowed_neighbors.push_back(neighbor.child_value());
			}
			
			rules[tile_name][side_name] = allowed_neighbors;
		}
	}
	
	return rules;
}

// Print adjacency rules in a human-readable format
void TileSet::print_rules(const AdjacencyRules& rules)
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
