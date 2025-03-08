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
	m_output_width = width;
	m_output_height = height;

	m_tile_map = vector(width * height, Tile{m_all_tile_names});
	is_tile_map_finished = false;
}

void TileMapGenerator::generate_single_step()
{
	if (is_tile_map_finished)
	{
		return;
	}

	// pick the lowest entropy cell
	int idx_to_collapse = get_next_cell_to_collapse(m_tile_map);

	// collapse cell
	collapse_cell(idx_to_collapse);

	QueueEntry collapsed_cell = QueueEntry{m_tile_map[idx_to_collapse],idx_to_collapse};

	// propagate constraints
	std::deque<QueueEntry> tiles_to_update_queue{collapsed_cell};
	recalculate_constraints(m_tile_map, tiles_to_update_queue);

	int remaining_cells = count_remaining_cells(m_tile_map);
	is_tile_map_finished = remaining_cells <= 0;
}

int TileMapGenerator::get_next_cell_to_collapse(TileMap& cells) const
{
	float minimal_value = std::numeric_limits<int>::max();
	std::optional<int> minimal_cell_idx;

	for (int i = 0; i < cells.size(); ++i)
	{
		Tile& cell = cells[i];

		if (cell.is_collapsed()) {
			continue;
		}

		float cell_entropy = compute_cell_entropy(cell);
		if (cell_entropy < minimal_value)
		{
			minimal_cell_idx = i;
			minimal_value = cell_entropy;
		}
	}

	if (!minimal_cell_idx.has_value())
	{
		std::cerr << "No cell to collapse" << std::endl;
		throw std::exception();
	}

	return minimal_cell_idx.value();
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

void TileMapGenerator::collapse_cell(const int idx)
{
	Tile& tile = m_tile_map[idx];

	string selected_tile = random_domain_tile(tile);

	tile.domain.clear();
	tile.domain.insert(selected_tile);
}

string TileMapGenerator::random_domain_tile(const Tile& tile) const
{
	vector<std::pair<string, float>> tile_weights;
	float total_weight = 0;
	for (const auto& cell : tile.domain)
	{
		float weight = m_tile_set.get_weight(cell);

		total_weight += weight;
		tile_weights.emplace_back(cell, weight);
	}

	float random_value = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) + 1.0f);
	random_value *= total_weight;

	float cumulative = 0;
	int selected_i = 0;
	for (int i = 0; i < tile_weights.size(); ++i)
	{
		cumulative += tile_weights[i].second / total_weight;
		if (random_value < cumulative)
		{
			selected_i = i;
			break;
		}
	}

	return tile_weights[selected_i].first;
}

void TileMapGenerator::recalculate_constraints(TileMap& cells, std::deque<QueueEntry>& tiles_to_update_queue)
{
	// propagate constraints: update cell's domain & add neighbors if changed
	while (!tiles_to_update_queue.empty())
	{
		auto [tile, idx] = tiles_to_update_queue.front();
		tiles_to_update_queue.pop_front();

		std::deque<QueueEntry> changed_tiles = update_neighbors_domain(idx, cells);

		tiles_to_update_queue.insert(tiles_to_update_queue.end(), changed_tiles.begin(), changed_tiles.end());
		changed_tiles.clear();
	}
}

std::deque<TileMapGenerator::QueueEntry> TileMapGenerator::update_neighbors_domain(const int idx, TileMap& cells)
{
	const Tile& tile = cells[idx];
	std::deque<QueueEntry> changed_tiles;

	const int& row = idx / m_output_width;
	const int& col = idx % m_output_width;

	if (const int bottom_idx = get_idx(row+1, col); row + 1 < m_output_height
		&& update_neighbor_domain(tile, cells[bottom_idx], TileSet::TOP_SIDE_KEY))
	{
		changed_tiles.push_back(QueueEntry{cells[bottom_idx], bottom_idx});
	}

	// top neighbor
	if (const int top_idx = get_idx(row-1, col); row - 1 >= 0
		&& update_neighbor_domain(tile, cells[top_idx], TileSet::BOTTOM_SIDE_KEY))
	{
		changed_tiles.push_back(QueueEntry{cells[top_idx], top_idx});
	}

	// right neighbor
	if (const int right_idx = get_idx(row, col+1); col + 1 < m_output_width
		&& update_neighbor_domain(tile, cells[right_idx], TileSet::LEFT_SIDE_KEY))
	{
		changed_tiles.push_back(QueueEntry{cells[right_idx], right_idx});
	}

	// left neighbor
	if (const int left_idx = get_idx(row, col-1); col - 1 >= 0
		&& update_neighbor_domain(tile, cells[left_idx], TileSet::RIGHT_SIDE_KEY))
	{
		changed_tiles.push_back(QueueEntry{cells[left_idx], left_idx});
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
	for (auto& cell : cells)
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

	return remaining_cells;
}

void TileMapGenerator::draw_tile_map() const
{
	const int mult = 6;  // todo: remove after debugging
	const int tile_width = 10*mult;
	const int tile_height = 10*mult;

	int x = 0;
	int y = 0;

	for (int i = 0; i < m_tile_map.size(); i++)
	{
		const Tile& tile = m_tile_map[i];
		std::optional<string> tile_name = tile.get_collapsed_name();

		ofSetColor(ofColor::white);
		if (tile_name.has_value()) {
			draw_tile(tile_name.value(), x, y, tile_width, tile_height);
		}
		else {
			draw_multiple_possibilities(tile, x, y, tile_width, tile_height);
		}

		x += tile_width;

		if (i % m_output_width == m_output_width - 1) {
			x = 0;
			y += tile_height;
		}
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
