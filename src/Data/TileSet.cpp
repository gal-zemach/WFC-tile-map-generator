#include "TileSet.h"

#include <filesystem>
#include <iostream>

#include "pugixml.hpp"
#include "ofMain.h"

using pugi::xml_node;
using pugi::xml_document;

// XML attribute names
static constexpr const char* SET_ATTRIBUTE_NAME = "set";
static constexpr const char* TILES_ATTRIBUTE_NAME = "tiles";
static constexpr const char* SYMMETRY_ATTRIBUTE_NAME = "symmetry";
static constexpr const char* WEIGHT_ATTRIBUTE_NAME = "weight";
static constexpr const char* EDGES_ATTRIBUTE_NAME = "edges";
static constexpr const char* EDGE_ATTRIBUTE_NAME = "edge";
static constexpr const char* VALUE_ATTRIBUTE_NAME = "value";

static constexpr const char* TILE_ATTRIBUTE_NAME = "tile";
static constexpr const char* NAME_ATTRIBUTE_NAME = "name";
static constexpr const char* SIDE_ATTRIBUTE_NAME = "side";

TileSet::TileSet(const string& xml_path, const string& images_folder_path)
{
	SetData set_data = parse_set_data(xml_path);
	m_set_data = add_rotated_tiles(set_data);
	adjacency = load_adjacency_rules(m_set_data);

	images = load_set_images(images_folder_path);
}


TileSet::SetData TileSet::parse_set_data(const string& xml_path)
{
	SetData set_data;
	xml_document doc;

	if (!doc.load_file(xml_path.c_str()))
	{
		std::cerr << "Failed to load XML file: " << xml_path << std::endl;
		return set_data;
	}

	xml_node set_parent = doc.child(SET_ATTRIBUTE_NAME);
	xml_node tiles_parent = set_parent.child(TILES_ATTRIBUTE_NAME);

	for (xml_node tile : tiles_parent.children(TILE_ATTRIBUTE_NAME))
	{
		string tile_name = tile.attribute(NAME_ATTRIBUTE_NAME).value();
		string symmetry_type = tile.attribute(SYMMETRY_ATTRIBUTE_NAME).value();

		string weight_str = tile.attribute(WEIGHT_ATTRIBUTE_NAME).value();
		float weight = weight_str.empty() ? DEFAULT_WEIGHT : atof(weight_str.c_str());

		vector<string> edges_map{NUMBER_OF_SIDES};
		xml_node edges_parent = tile.child(EDGES_ATTRIBUTE_NAME);

		for (xml_node edge : edges_parent.children(EDGE_ATTRIBUTE_NAME))
		{
			string edge_side = edge.attribute(SIDE_ATTRIBUTE_NAME).value();

			int edge_idx = SIDES.at(edge_side);

			string edge_value = edge.attribute(VALUE_ATTRIBUTE_NAME).value();
			edges_map[edge_idx] = edge_value;
		}

		set_data.tiles[tile_name] = TileData{symmetry_type, weight, edges_map};
	}

	return set_data;
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

vector<string> TileSet::rotate_edges_map(const vector<string>& edges_map, int rotate_by)
{
	vector<string> shifted_edges{NUMBER_OF_SIDES};
	for (int i = 0; i < shifted_edges.size(); i++) {
		shifted_edges[rotate_side(i, rotate_by)] = edges_map[i];
	}

	return shifted_edges;
}

void TileSet::add_tile_rotations(SetData& set_data, const pair<string, TileData>& tile_data)
{
	int n = symmetry_type_to_rotations.at(tile_data.second.symmetry_type);
	float weight_sum = 0;

	for (int i = 0; i < n; i++)
	{
		string tile_name = tile_data.first;
		int tile_rotation = i*360/n;

		if (n > 1) {
			tile_name += "_" + std::to_string(tile_rotation);
		}

		set_data.tiles[tile_name] = TileData{
			tile_data.second.symmetry_type,
			tile_data.second.weight,
			rotate_edges_map(tile_data.second.edges, tile_rotation)
		};

		weight_sum += tile_data.second.weight;
	}

	// normalize weights
	for (auto& tile : set_data.tiles)
	{
		tile.second.weight /= weight_sum;
	}
}

TileSet::SetData TileSet::add_rotated_tiles(const SetData& set_data)
{
	SetData set_data_with_symmetry;
	for (const auto& tile : set_data.tiles)
	{
		add_tile_rotations(set_data_with_symmetry, tile);
	}

	return set_data_with_symmetry;
}

// For each tile, adds to the adjacency list all tile names with an opposite matching side
TileSet::AdjacencyRules TileSet::load_adjacency_rules(const SetData& set_data)
{
	AdjacencyRules rules;
	for (const auto& [tile_name, tile_data] : set_data.tiles)
	{
		for (const auto& [neighbor_tile_name, neighbor_tile_data] : set_data.tiles)
		{
			for (int i=0; i < NUMBER_OF_SIDES; i++)
			{
				string set_tile_edge_value = tile_data.edges[i];
				string neighbor_tile_opposite_edge_value = neighbor_tile_data.edges[opposite_side(i)];
				if (set_tile_edge_value == neighbor_tile_opposite_edge_value)
				{
					if (!rules.contains(tile_name))
					{
						rules[tile_name] = vector<vector<string>>(NUMBER_OF_SIDES);
					}

					rules[tile_name][i].push_back(neighbor_tile_name);
				}
			}
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
		for (int j = 0; j < NUMBER_OF_SIDES; j++)
		{
			vector<string> neighbors = sides[j];

			std::cout << " " << j << ": [";
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
