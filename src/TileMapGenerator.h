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

	explicit TileMapGenerator(const TileSet& tile_set);

	void generate_tile_map(int width, int height);
	void init_tile_map(int width, int height);
	void generate_single_step();

	void draw_tile_map() const;

private:
	using TileMap = vector<Tile>;
	using SupportCount = vector<unordered_map<string, int>>;

	struct QueueEntry {
		std::reference_wrapper<Tile> tile;
		int idx;
	};

	int m_output_width, m_output_height;
	const TileSet& m_tile_set;
	unordered_set<string> m_all_tile_names;

	TileMap m_tile_map;

	// m_support_count[idx + side][tile_name] = how many neighbor tiles on this side support/allow tile_name
	SupportCount m_support_count;

	std::optional<int> get_idx(const int row, const int col) const;
	std::optional<int> get_neighbor_idx(const int idx, const int side) const;
	pair<int, int> get_coord(const int idx) const;

	void init_support_count();

	int get_next_cell_to_collapse(TileMap& cells) const;
	float compute_cell_entropy(const Tile& cell) const;


	string random_domain_tile(const Tile& tile) const;
	void collapse_cell(int idx);

	void recalculate_constraints(TileMap& cells, std::deque<QueueEntry>& tiles_to_update_queue);
	deque<QueueEntry> update_neighbors_domain(int idx, TileMap& cells);
	bool update_neighbor_domain(const Tile& current_tile, Tile& neighbor, const int direction_from_neighbor);

	int count_remaining_cells(const TileMap& cells);

	void draw_tile(const string& tile_name, float x, float y, float tile_width, float tile_height) const;
	void draw_multiple_possibilities(const Tile& tile, float x, float y, float tile_width, float tile_height) const;
	static void draw_image(const ofImage& image, float x, float y, float width, float height, int rotation);
};
