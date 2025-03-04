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
	using TileMap = vector<vector<Tile>>;

	TileMapGenerator(const TileSet& tile_set);

	TileMap GenerateTileMap(int width, int height);

	void DrawTileMap(const TileMap& tile_map); //todo: save map as member?

private:
	struct QueueEntry {
		std::reference_wrapper<Tile> tile;
		std::pair<int, int> position;
	};

	const TileSet& m_tile_set;
	unordered_set<string> m_all_tile_names;

	Tile& get_next_index_to_collapse(TileMap& cells) const;
	void recalculate_constraints(TileMap& cells);
	deque<QueueEntry> update_domain(const std::pair<int, int>& position, TileMap& cells);
	bool update_domain_according_to_neighbor(Tile& tile_to_update, const Tile& neighbor, const string& direction_from_neighbor);
	int count_uncollapsed_cells(const TileMap& cells);

	static void collapse_cell(Tile& tile);
};
