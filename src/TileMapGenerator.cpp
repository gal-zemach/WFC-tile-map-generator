#include "TileMapGenerator.h"

#include "ofMain.h"

TileMapGenerator::TileMapGenerator(const TileSet& tile_set) : m_tile_set{tile_set}
{
	std::srand(static_cast<unsigned int>(std::time(nullptr)));

	for (const auto& key : m_tile_set.images | std::views::keys)
	{
		m_all_tile_names.insert(key);
	}
}

void TileMapGenerator::generate_tile_map(const int width, const int height)
{
	init_tile_map(width, height);
	
	while (!is_tile_map_finished)
	{
		generate_single_step();
	}
}

void TileMapGenerator::init_tile_map(const int width, const int height)
{
	m_tile_map = vector(height, vector(width, Tile{m_all_tile_names}));  //todo: refactor to 1D ?
	is_tile_map_finished = false;
}

void TileMapGenerator::generate_single_step()
{
	if (is_tile_map_finished)
	{
		return;
	}

	std::cout << "step started." << std::endl;

	// pick the lowest entropy cell
	std::pair<int, int> position_to_collapse = get_next_cell_to_collapse(m_tile_map);

	// collapse cell
	collapse_cell(position_to_collapse);

	QueueEntry collapsed_cell = QueueEntry{
		m_tile_map[position_to_collapse.first][position_to_collapse.second],
		position_to_collapse
	};

	// propagate constraints
	std::deque<QueueEntry> tiles_to_update_queue{collapsed_cell};
	recalculate_constraints(m_tile_map, tiles_to_update_queue);

	int remaining_cells = count_remaining_cells(m_tile_map);
	is_tile_map_finished = remaining_cells <= 0;
}

std::pair<int, int> TileMapGenerator::get_next_cell_to_collapse(TileMap& cells) const
{
	int minimal_value = m_all_tile_names.size() + 1;
	std::optional<std::pair<int, int>> minimal_cell_position;

	for (int i = 0; i < cells.size(); ++i)
	{
		for (int j = 0; j < cells[i].size(); ++j)
		{
			Tile& cell = cells[i][j];

			int possible_tiles_in_cell = cell.domain.size();
			if (possible_tiles_in_cell < minimal_value &&
				possible_tiles_in_cell > 1)
			{
				minimal_cell_position = std::make_pair(i, j);
				minimal_value = possible_tiles_in_cell;
			}
		}
	}

	// // todo: remove random choosing? if leaving it add <=
	// output_ptr = minimal_tiles[0].tile;

	if (!minimal_cell_position.has_value())
	{
		std::cerr << "No cell to collapse" << std::endl;
		throw std::exception();
	}

	return minimal_cell_position.value();
}

void TileMapGenerator::collapse_cell(const std::pair<int, int>& position)
{
	Tile& tile = m_tile_map[position.first][position.second];

	int i_to_keep = rand() % tile.domain.size();
	// i_to_keep = 0;

	auto it = tile.domain.begin();
	std::advance(it, i_to_keep);
	string name_to_keep = *it;

	tile.domain.clear();
	tile.domain.insert(name_to_keep);
}

void TileMapGenerator::recalculate_constraints(TileMap& cells, std::deque<QueueEntry>& tiles_to_update_queue)
{
	std::cout << "starting with " << tiles_to_update_queue.size() << " in queue: ";
	if (tiles_to_update_queue.size() > 0) {
		std::cout << "[";
		for (auto& tile : tiles_to_update_queue)
		{
			std::cout << "(" << tile.position.first << "," << tile.position.second << ")";
		}
		std::cout << "]";
	}
	std::cout << std::endl;

	// propagate constraints: update cell's domain & add neighbors if changed
	while (!tiles_to_update_queue.empty())
	{
		auto [tile, position] = tiles_to_update_queue.front();
		tiles_to_update_queue.pop_front();

		std::deque<QueueEntry> changed_tiles = update_neighbors_domain(position, cells);

		std::cout << "for (" << position.first << "," << position.second << ") adding " << changed_tiles.size() << ": ";

		if (changed_tiles.size() > 0) {
			std::cout << "[";
			for (auto& changed : changed_tiles)
			{
				std::cout << "(" << changed.position.first << "," << changed.position.second << ")";
			}
			std::cout << "]";
		}
		std::cout << std::endl;

		tiles_to_update_queue.insert(tiles_to_update_queue.end(), changed_tiles.begin(), changed_tiles.end());
		changed_tiles.clear();
	}

	std::cout << "step done.\n" << std::endl;
}

std::deque<TileMapGenerator::QueueEntry> TileMapGenerator::update_neighbors_domain(const std::pair<int, int>& position, TileMap& cells)
{
	const int& i = position.first;
	const int& j = position.second;
	Tile& tile = cells[i][j];
	std::deque<QueueEntry> changed_tiles;

	// bottom neighbor
	if (i + 1 < cells.size()
		&& update_neighbor_domain(tile, cells[i+1][j], TileSet::TOP_SIDE_KEY))
	{
		changed_tiles.push_back(QueueEntry{cells[i+1][j], std::make_pair(i+1, j)});
	}

	// top neighbor
	if (i - 1 > 0
		&& update_neighbor_domain(tile, cells[i-1][j], TileSet::BOTTOM_SIDE_KEY))
	{
		changed_tiles.push_back(QueueEntry{cells[i-1][j], std::make_pair(i-1, j)});
	}

	// right neighbor
	if (j + 1 < cells[i].size()
		&& update_neighbor_domain(tile, cells[i][j+1], TileSet::LEFT_SIDE_KEY))
	{
		changed_tiles.push_back(QueueEntry{cells[i][j+1], std::make_pair(i, j+1)});
	}

	// left neighbor
	if (j - 1 > 0
		&& update_neighbor_domain(tile, cells[i][j-1], TileSet::RIGHT_SIDE_KEY))
	{
		changed_tiles.push_back(QueueEntry{cells[i][j-1], std::make_pair(i, j-1)});
	}

	return changed_tiles;
}

bool TileMapGenerator::update_neighbor_domain(const Tile& current_tile, Tile& neighbor, const string& direction_from_neighbor)
{
	if (neighbor.domain.size() == 1)
	{
		return false;
	}

	std::deque<string> unsupported_neighbor_tiles;
	// bool neighbor_started_collapsed = neighbor.domain.size() == 1;

	// // todo: for debugging
	// std::unordered_set<string> neighbor_domain_backup;
	// vector<string> neighbor_adjacency_rules_backup;
	// if (neighbor_started_collapsed) {
	// 	neighbor_domain_backup = neighbor.domain;
	// 	neighbor_adjacency_rules_backup = m_tile_set.adjacency.at(*neighbor.domain.begin()).at(direction_from_neighbor);
	// }

	for (const auto& neighbor_tile_name : neighbor.domain)
	{
		bool is_supported = false;

		for (auto& current_tile_name : current_tile.domain)
		{
			vector<string> neighbor_adjacency_rules = m_tile_set.adjacency.at(neighbor_tile_name).at(direction_from_neighbor);
			for (auto& allowed_name : neighbor_adjacency_rules)
			{
				if (allowed_name == current_tile_name)
				{
					is_supported = true;
					break;
				}
			}
		}

		if (!is_supported)
		{
			unsupported_neighbor_tiles.push_back(neighbor_tile_name);
		}
	}

	for (const auto& unsupported_tile : unsupported_neighbor_tiles)
	{
		neighbor.domain.erase(unsupported_tile);
	}

	// return neighbor.domain.size() == 1 && !neighbor_started_collapsed;
	return neighbor.domain.size() == 1;
}

/**
 * Returns the number of cells with multiple tile possibilities
 * @return The number above or 0 if any cell was found with domain.size() == 0 (invalid output)
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

void TileMapGenerator::draw_tile_map() const
{
	const int mult = 6;  // todo: remove after debugging
	const int tile_width = 10*mult;
	const int tile_height = 10*mult;

	int image_width = tile_width * m_tile_map[0].size();
	int image_height = tile_height * m_tile_map.size();

	int x = 0;
	int y = 0;

	for (const auto& row : m_tile_map) {
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

void TileMapGenerator::draw_tile(const string& tile_name, float x, float y, float tile_width, float tile_height) const {
	m_tile_set.images.at(tile_name).draw(x, y, tile_width, tile_height);
}

/**
 * superimpose all possibilities with transparency
 */
void TileMapGenerator::draw_multiple_possibilities(const Tile& tile, float x, float y, float tile_width, float tile_height) const {
	int number_of_possibilities = tile.domain.size();
	ofSetColor(ofColor::white, 255 / number_of_possibilities);

	for (const string& possibility_name : tile.domain) {
		draw_tile(possibility_name, x, y, tile_width, tile_height);
	}
}
