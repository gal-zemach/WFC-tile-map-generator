#include "TileMapGenerator.h"

#include "ofMain.h"

TileMapGenerator::TileMapGenerator(const TileSet& tile_set) : m_tile_set{tile_set}
{
	for (const auto& key : m_tile_set.images | std::views::keys)
	{
		m_all_tile_names.insert(key);
	}
}

void TileMapGenerator::DrawTileMap(const TileMap& tile_map)
{
	const int mult = 3;  // todo: remove after debugging
	const int tile_width = 10*mult;
	const int tile_height = 10*mult;

	int image_width = tile_width * tile_map[0].size();
	int image_height = tile_height * tile_map.size();

	int x = 0;
	int y = 0;

	for (const auto& row : tile_map) {
		for (const auto& tile : row) {
			std::optional<string> tile_name = tile.get_collapsed_name();

			ofSetColor(ofColor::white);
			if (tile_name.has_value()) {
				draw_tile(tile_name.value(), x, y, tile_width, tile_height);
			}
			else {
				draw_multiple_possibilities(tile, x, y, tile_width, tile_height);
			}

			ofSetColor(ofColor::magenta);  // todo: remove after debugging
			ofDrawRectangle(x, y, 1*mult, 1*mult);

			x = (x + tile_width) % image_width;
		}

		y = y + tile_height;
	}
}

void TileMapGenerator::draw_tile(const string& tile_name, float x, float y, float tile_width, float tile_height) {
	m_tile_set.images.at(tile_name).draw(x, y, tile_width, tile_height);
}

/**
 * superimpose all possibilities with transparency
 */
void TileMapGenerator::draw_multiple_possibilities(const Tile& tile, float x, float y, float tile_width, float tile_height) {
	int number_of_possibilities = tile.m_possible_tiles.size();
	ofSetColor(ofColor::white, 255 / number_of_possibilities);

	for (const string& possibility_name : tile.m_possible_tiles) {
		draw_tile(possibility_name, x, y, tile_width, tile_height);
	}
}

TileMapGenerator::TileMap TileMapGenerator::generate_tile_map(const int width, const int height)
{
	TileMap output = init_tile_map(width, height);
	
	int cells_left = width * height;
	while (cells_left > 0)
	{
		generate_single_step(output);
		cells_left = count_remaining_cells(output);
	}
	
	return output;
}

TileMapGenerator::TileMap TileMapGenerator::init_tile_map(const int width, const int height)
{
	return vector(height, vector(width, Tile{m_all_tile_names}));  //todo: refactor to 1D ?
}

void TileMapGenerator::generate_single_step(TileMap& tile_map)
{
	if (count_remaining_cells(tile_map) <= 0)
	{
		return;
	}

	// pick the lowest entropy cell
	Tile& cell_to_collapse = get_next_index_to_collapse(tile_map);

	// collapse cell
	collapse_cell(cell_to_collapse);

	// propagate constraints
	recalculate_constraints(tile_map);
}

// can find a cell, find all cells have 1, find cell with no options (0)
Tile& TileMapGenerator::get_next_index_to_collapse(TileMap& cells) const
{
	Tile* output_ptr = nullptr;
	int minimal_value = m_all_tile_names.size();

	for (auto& row : cells)
	{
		for (auto& cell : row)
		{
			int possible_tiles_in_cell = cell.m_possible_tiles.size();
			if (possible_tiles_in_cell <= minimal_value &&
				possible_tiles_in_cell > 1)
			{
				output_ptr = &cell;
				minimal_value = possible_tiles_in_cell;
			}
		}
	}

	if (output_ptr == nullptr)
	{
		std::cerr << "No cell to collapse" << std::endl;
		throw std::exception();
	}

	return *output_ptr;
}

void TileMapGenerator::recalculate_constraints(TileMap& cells)
{
	std::deque<QueueEntry> tiles_to_update_queue;

	// populate the queue with all collapsed tiles
	for (int i = 0; i < cells.size(); ++i)
	{
		for (int j = 0; j < cells[i].size(); ++j)
		{
			if (cells[i][j].is_collapsed())
			{
				tiles_to_update_queue.push_back(QueueEntry{cells[i][j], std::make_pair(i, j)});
			}
		}
	}

	// propagate constraints: update cell's domain & add neighbors if changed
	while (!tiles_to_update_queue.empty())
	{
		auto [tile, position] = tiles_to_update_queue.front();
		tiles_to_update_queue.pop_front();

		std::deque<QueueEntry> changed_tiles = update_domain(position, cells);

		tiles_to_update_queue.insert(tiles_to_update_queue.end(), changed_tiles.begin(), changed_tiles.end());
	}
}

std::deque<TileMapGenerator::QueueEntry> TileMapGenerator::update_domain(const std::pair<int, int>& position, TileMap& cells)
{
	const int& i = position.first;
	const int& j = position.second;
	Tile& tile = cells[i][j];
	std::deque<QueueEntry> changed_tiles;

	// right neighbor
	if (i + 1 < cells.size()
		&& update_domain_according_to_neighbor(tile, cells[i+1][j], TileSet::LEFT_SIDE_KEY))
	{
		changed_tiles.push_back(QueueEntry{cells[i+1][j], std::make_pair(i+1, j)});
	}

	// left neighbor
	if (i - 1 > 0
		&& update_domain_according_to_neighbor(tile, cells[i-1][j], TileSet::RIGHT_SIDE_KEY))
	{
		changed_tiles.push_back(QueueEntry{cells[i-1][j], std::make_pair(i-1, j)});
	}

	// bottom neighbor
	if (j + 1 < cells[i].size()
		&& update_domain_according_to_neighbor(tile, cells[i][j+1], TileSet::TOP_SIDE_KEY))
	{
		changed_tiles.push_back(QueueEntry{cells[i][j+1], std::make_pair(i, j+1)});
	}

	// top neighbor
	if (j - 1 > 0
		&& update_domain_according_to_neighbor(tile, cells[i][j-1], TileSet::BOTTOM_SIDE_KEY))
	{
		changed_tiles.push_back(QueueEntry{cells[i][j-1], std::make_pair(i, j-1)});
	}

	return changed_tiles;
}

bool TileMapGenerator::update_domain_according_to_neighbor(Tile& tile_to_update, const Tile& neighbor, const string& direction_from_neighbor)
{
	bool tile_updated = false;
	for (const auto& tile_name : tile_to_update.m_possible_tiles)
	{
		bool tile_name_allowed = false;
		for (auto& neighbor_domain_tile_name :neighbor.m_possible_tiles)
		{
			for (auto& allowed_name : m_tile_set.adjacency.at(neighbor_domain_tile_name).at(direction_from_neighbor))
			{
				if (allowed_name == tile_name)
				{
					tile_name_allowed = true;
					break;
				}
			}
		}

		if (!tile_name_allowed)
		{
			tile_to_update.m_possible_tiles.erase(tile_name);
			tile_updated = true;
		}
	}

	return tile_updated;
}

/**
 * Either a single cell has domain.size() == 0 (invalid output) or all cells are collapsed (output complete)
 */
int TileMapGenerator::count_remaining_cells(const TileMap& cells)
{
	int remaining_cells = 0;
	for (auto& row : cells)
	{
		for (auto& cell : row)
		{
			if (cell.is_domain_empty())
			{
				return 0;
			}

			if (!cell.is_collapsed())
			{
				remaining_cells++;
			}
		}
	}

	return remaining_cells;
}

void TileMapGenerator::collapse_cell(Tile& tile)
{
	std::srand(static_cast<unsigned int>(std::time(nullptr)));
	int i_to_keep = rand() % tile.m_possible_tiles.size();

	auto it = tile.m_possible_tiles.begin();
	std::advance(it, i_to_keep);
	string name_to_keep = *it;

	tile.m_possible_tiles.clear();
	tile.m_possible_tiles.insert(name_to_keep);
}
