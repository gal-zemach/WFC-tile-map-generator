#pragma once

#include <unordered_map>
#include <vector>
#include <string>

#include "ofMain.h"

using std::string;
using std::vector;
using std::unordered_map;

class TileSet
{
public:
	static constexpr const char* TOP_SIDE_KEY = "top";
	static constexpr const char* BOTTOM_SIDE_KEY = "bottom";
	static constexpr const char* LEFT_SIDE_KEY = "left";
	static constexpr const char* RIGHT_SIDE_KEY = "right";
	
	using TileImages = unordered_map<string, ofImage>;
	using AdjacencyRules = unordered_map<string, unordered_map<string, vector<string>>>;
	
	TileImages images;
	AdjacencyRules adjacency;
	
	TileSet(const string& xml_path, const string& images_folder_path);
	
private:
	static TileImages load_set_images(const string& images_folder_path);
	static AdjacencyRules load_adjacency_rules(const string& xml_path);
	static void print_rules(const AdjacencyRules& rules);
};
