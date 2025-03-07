#pragma once

#include <string>
#include <vector>
#include <unordered_set>
#include <deque>

#include "Data/TileSet.h"
#include "Data/Tile.h"

class TileMapGenerator
{
public:
	bool is_tile_map_finished = false;

	TileMapGenerator(const TileSet& tile_set);

	void generate_tile_map(int width, int height);
	void init_tile_map(int width, int height);
	void generate_single_step();

	void draw_tile_map() const;

private:
	using TileMap = vector<vector<Tile>>;

	struct QueueEntry {
		std::reference_wrapper<Tile> tile;
		std::pair<int, int> position;
	};

	const TileSet& m_tile_set;
	unordered_set<string> m_all_tile_names;

	TileMap m_tile_map;

	std::pair<int, int> get_next_cell_to_collapse(TileMap& cells) const;

	float compute_cell_entropy(const Tile& cell) const;

	void collapse_cell(const std::pair<int, int>& position);

	string random_domain_tile(const Tile& tile) const;

	void recalculate_constraints(TileMap& cells, std::deque<QueueEntry>& tiles_to_update_queue);

	deque<QueueEntry> update_neighbors_domain(const std::pair<int, int>& position, TileMap& cells);

	bool update_neighbor_domain(const Tile& current_tile, Tile& neighbor, const string& direction_from_neighbor);

	int count_remaining_cells(const TileMap& cells);

	void draw_multiple_possibilities(const Tile& tile, float x, float y, float tile_width, float tile_height) const;

	void draw_tile(const string& tile_name, float x, float y, float tile_width, float tile_height) const;

	static void draw_image(const ofImage& image, float x, float y, float width, float height, int rotation);
};
