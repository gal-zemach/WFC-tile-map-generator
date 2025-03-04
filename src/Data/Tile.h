#pragma once

#include <string>
#include <vector>
#include <unordered_set>
#include <optional>

using std::string;
using std::vector;
using std::unordered_set;

/**
 * @class Tile
 * @brief Represents a tile in the wave function collapse (wfc) algorithm
 */
class Tile
{
public:
	unordered_set<string> m_possible_tiles;
	
	Tile(const unordered_set<string>& possible_tiles) : m_possible_tiles(possible_tiles){}
	
	//todo: choose approach here
	void remove_possible_tile();
	void update_constraints();
	
	/**
	 * @brief Returns true iff one possible tile remains for this tile
	 */
	bool is_collapsed() const { return m_possible_tiles.size() == 1; }
	
	/**
	 * @brief Returns true if no possible tiles remain for this tile and false otherwise
	 */
	bool is_domain_empty() const { return m_possible_tiles.empty(); }
	
	/**
	 * @brief returns the name of the collapsed tile iff the tile is collapsed
	 * @return string only if the tile collapsed
	 */
	std::optional<string> get_collapsed_name() const { return is_collapsed() ? std::make_optional(*m_possible_tiles.begin()) : std::nullopt; }
};
