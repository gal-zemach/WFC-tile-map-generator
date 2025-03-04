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
	TileMapGenerator(const TileSet& tile_set);

	void generate_tile_map(int width, int height);
	void init_tile_map(int width, int height);
	void generate_single_step();

	void DrawTileMap() const;

private:
	using TileMap = vector<vector<Tile>>;

	struct QueueEntry {
		std::reference_wrapper<Tile> tile;
		std::pair<int, int> position;
	};

	const TileSet& m_tile_set;
	unordered_set<string> m_all_tile_names;

	TileMap m_tile_map;
	bool m_is_tile_map_finished = false;

	Tile& get_next_cell_to_collapse(TileMap& cells) const;
	static void collapse_cell(Tile& tile);
	void recalculate_constraints(TileMap& cells);
	deque<QueueEntry> update_domain(const std::pair<int, int>& position, TileMap& cells);
	bool update_domain_according_to_neighbor(Tile& tile_to_update, const Tile& neighbor, const string& direction_from_neighbor);
	int count_remaining_cells(const TileMap& cells);

	void draw_tile(const string& tile_name, float x, float y, float tile_width, float tile_height) const;
	void draw_multiple_possibilities(const Tile& tile, float x, float y, float tile_width, float tile_height) const;
};
