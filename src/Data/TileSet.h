#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <unordered_set>

#include "ofMain.h"

using std::string;
using std::vector;
using std::unordered_map;

using std::unordered_set;

/**
 * @class TileSet
 * @brief Represents a loaded tile set with adjacency list used for procedural generation
 */
class TileSet
{
public:
	// Constants with keys for the tiles' sides in the adjacency list
	static constexpr int NUMBER_OF_SIDES = 4;
	static constexpr int TOP_SIDE_IDX = 0;
	static constexpr int RIGHT_SIDE_IDX = 1;
	static constexpr int BOTTOM_SIDE_IDX = 2;
	static constexpr int LEFT_SIDE_IDX = 3;

	// Maps tile name to it's image
	using TileImages = unordered_map<string, ofImage>;
	
	// Maps tile name to its adjacency list in the for of rules[tile_name][side_constant] = allowed_tile_names_vector
	using AdjacencyRules = unordered_map<string, vector<unordered_set<string>>>;

	/**
	 * @brief Constructs a TileSet by loading images and adjacency rules.
	 * @param xml_path Path to the XML file containing adjacency rules.
	 * @param images_folder_path Path to the folder containing tile images.
	 */
	TileImages images;
	AdjacencyRules adjacency;
	
	TileSet(const string& xml_path, const string& images_folder_path);

	float get_weight(const string& tile_name) const {return m_set_data.tiles.at(tile_name).weight;}

	static int rotate_side(const int side_idx, const int degrees) {return (side_idx + degrees/90) % NUMBER_OF_SIDES;}
	static int opposite_side(const int side_idx) {return rotate_side(side_idx, 180);}

private:
	static constexpr float DEFAULT_WEIGHT = 1;

	inline static unordered_map<string, int> SIDES{
		{"top", TOP_SIDE_IDX}, {"right", RIGHT_SIDE_IDX}, {"bottom", BOTTOM_SIDE_IDX}, {"left", LEFT_SIDE_IDX}
	};

	static constexpr const char* SYMMETRY_TYPE_I = "I";
	static constexpr const char* SYMMETRY_TYPE_L = "L";
	static constexpr const char* SYMMETRY_TYPE_T = "T";
	static constexpr const char* SYMMETRY_TYPE_X = "X";

	inline static unordered_map<string, int> symmetry_type_to_rotations{
				{SYMMETRY_TYPE_I, 2}, {SYMMETRY_TYPE_L, 4}, {SYMMETRY_TYPE_T, 4}, {SYMMETRY_TYPE_X, 1}
	};

	struct TileData
	{
		string symmetry_type;
		float weight;
		vector<string> edges;
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

	static vector<string> rotate_edges_map(const vector<string>& edges_map, int rotate_by);
	static void add_tile_rotations(SetData& set_data, const pair<string, TileData>& tile_data);
};
