#pragma once

#include <unordered_map>
#include <vector>
#include <string>

#include "ofMain.h"

using std::string;
using std::vector;
using std::unordered_map;

/**
 * @class TileSet
 * @brief Represents a loaded tile set with adjacency list used for procedural generation
 */
class TileSet
{
public:
	// Constants with keys for the tiles' sides in the adjacency list
	static constexpr const char* TOP_SIDE_KEY = "top";
	static constexpr const char* BOTTOM_SIDE_KEY = "bottom";
	static constexpr const char* LEFT_SIDE_KEY = "left";
	static constexpr const char* RIGHT_SIDE_KEY = "right";

	// Maps tile name to it's image
	using TileImages = unordered_map<string, ofImage>;
	
	// Maps tile name to its adjacency list in the for of rules[tile_name][side_constant] = allowed_tile_names_vector
	using AdjacencyRules = unordered_map<string, unordered_map<string, vector<string>>>;

	/**
	 * @brief Constructs a TileSet by loading images and adjacency rules.
	 * @param xml_path Path to the XML file containing adjacency rules.
	 * @param images_folder_path Path to the folder containing tile images.
	 */
	TileImages images;
	AdjacencyRules adjacency;
	
	TileSet(const string& xml_path, const string& images_folder_path);

	float get_weight(const string& tile_name) const {return m_set_data.tiles.at(tile_name).weight;}
	
private:
	static constexpr float DEFAULT_WEIGHT = 1;

	static constexpr const char* SYMMETRY_TYPE_I = "I";
	static constexpr const char* SYMMETRY_TYPE_L = "L";
	static constexpr const char* SYMMETRY_TYPE_T = "T";
	static constexpr const char* SYMMETRY_TYPE_X = "X";

	inline static const vector<string> SIDES{
		TOP_SIDE_KEY, RIGHT_SIDE_KEY, BOTTOM_SIDE_KEY, LEFT_SIDE_KEY
	};

	inline static unordered_map<string, int> symmetry_type_to_rotations{
				{SYMMETRY_TYPE_I, 2}, {SYMMETRY_TYPE_L, 4}, {SYMMETRY_TYPE_T, 4}, {SYMMETRY_TYPE_X, 1}
	};

	struct TileData
	{
		string symmetry_type;
		float weight;
		unordered_map<string, string> edges;
	};

	struct SetData
	{
		unordered_map<string, TileData> tiles;
	};

	SetData m_set_data;

	static SetData parse_set_data(const string& xml_path);
	static SetData add_rotated_tiles(const SetData& set_data);

	static TileImages load_set_images(const string& images_folder_path);
	static AdjacencyRules load_adjacency_rules(const SetData& set_data);
	static void print_rules(const AdjacencyRules& rules);

	static unordered_map<string, string> rotate_edges_map(const unordered_map<string, string>& edges_map, int rotate_by);
	static void add_tile_rotations(SetData& set_data, const pair<string, TileData>& tile_data);
};
