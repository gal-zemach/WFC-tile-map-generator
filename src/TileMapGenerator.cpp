#include "TileMapGenerator.h"

#include "ofMain.h"

TileMapGenerator::TileMapGenerator(const TileSet& tile_set) : m_tile_set{tile_set}
{
	std::srand(static_cast<unsigned int>(std::time(nullptr)));

	for (const auto& key : m_tile_set.adjacency | std::views::keys)
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
	float minimal_value = std::numeric_limits<int>::max();
	std::optional<std::pair<int, int>> minimal_cell_position;

	for (int i = 0; i < cells.size(); ++i)
	{
		for (int j = 0; j < cells[i].size(); ++j)
		{
			Tile& cell = cells[i][j];

			if (cell.is_collapsed()) {
				continue;
			}

			float cell_entropy = compute_cell_entropy(cell);
			if (cell_entropy < minimal_value)
			{
				minimal_cell_position = std::make_pair(i, j);
				minimal_value = cell_entropy;
			}
		}
	}

	if (!minimal_cell_position.has_value())
	{
		std::cerr << "No cell to collapse" << std::endl;
		throw std::exception();
	}

	return minimal_cell_position.value();
}

float TileMapGenerator::compute_cell_entropy(const Tile& cell) const
{
	float entropy = 0;
	for (const auto& tile_name : cell.domain)
	{
		float w = m_tile_set.get_weight(tile_name);
		entropy -= w * log2(w);
	}

	return entropy;
}

void TileMapGenerator::collapse_cell(const std::pair<int, int>& position)
{
	Tile& tile = m_tile_map[position.first][position.second];

	int i_to_keep = rand() % tile.domain.size();

	auto it = tile.domain.begin();
	std::advance(it, i_to_keep);
	string name_to_keep = *it;

	tile.domain.clear();
	tile.domain.insert(name_to_keep);
}

void TileMapGenerator::recalculate_constraints(TileMap& cells, std::deque<QueueEntry>& tiles_to_update_queue)
{
	// propagate constraints: update cell's domain & add neighbors if changed
	while (!tiles_to_update_queue.empty())
	{
		auto [tile, position] = tiles_to_update_queue.front();
		tiles_to_update_queue.pop_front();

		std::deque<QueueEntry> changed_tiles = update_neighbors_domain(position, cells);

		tiles_to_update_queue.insert(tiles_to_update_queue.end(), changed_tiles.begin(), changed_tiles.end());
		changed_tiles.clear();
	}
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
	if (i - 1 >= 0
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
	if (j - 1 >= 0
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
	for (const auto& neighbor_tile_name : neighbor.domain)
	{
		vector<string> neighbor_adjacency_rules = m_tile_set.adjacency.at(neighbor_tile_name).at(direction_from_neighbor);
		bool is_supported = false;

		for (auto& allowed_neighbor : neighbor_adjacency_rules)
		{
			if (current_tile.domain.contains(allowed_neighbor))
			{
				is_supported = true;
				break;
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

			x = (x + tile_width) % image_width;
		}

		y = y + tile_height;
	}
}

void TileMapGenerator::draw_tile(const string& tile_name, float x, float y, float tile_width, float tile_height) const {

	const int delim_pos = tile_name.find("_");
	string base_name = tile_name.substr(0, delim_pos);
	const string rotation_str = tile_name.substr(delim_pos + 1, tile_name.length() - delim_pos - 1);
	int rotation = rotation_str.empty() ? 0 : atoi(rotation_str.c_str());

	const ofImage& image = m_tile_set.images.at(base_name);
	draw_image(image, x, y, tile_width, tile_height, rotation);
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

void TileMapGenerator::draw_image(const ofImage& image, float x, float y, float width, float height, int rotation) {
	ofPushMatrix();
	// Move the origin to the center of the destination rectangle.
	ofTranslate(x + width / 2, y + height / 2);

	// Apply the rotation (in degrees).
	ofRotateDeg(rotation);

	// Draw the image centered at the origin.
	image.draw(-width / 2, -height / 2, width, height);
	ofPopMatrix();
}
