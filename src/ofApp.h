#pragma once

#include "Data/TileSet.h"
#include <TileMapGenerator.h>

#include "ofMain.h"

class ofApp : public ofBaseApp{

	public:
		void setup() override;
		void update() override;
		void draw() override;
		void exit() override;

		void keyPressed(int key) override;
		void keyReleased(int key) override;
		void mouseMoved(int x, int y ) override;
		void mouseDragged(int x, int y, int button) override;
		void mousePressed(int x, int y, int button) override;
		void mouseReleased(int x, int y, int button) override;
		void mouseScrolled(int x, int y, float scrollX, float scrollY) override;
		void mouseEntered(int x, int y) override;
		void mouseExited(int x, int y) override;
		void windowResized(int w, int h) override;
		void dragEvent(ofDragInfo dragInfo) override;
		void gotMessage(ofMessage msg) override;

	private:
		const int TILE_MAP_WIDTH = 12;
		const int TILE_MAP_HEIGHT = 9;
		const int ANIMATION_FRAME_RATE = 60;

		std::unique_ptr<TileSet> m_tile_set;
		std::unique_ptr<TileMapGenerator> m_tile_map_generator;

		bool m_start_animation_pressed = false;
		bool m_erase_map_pressed = false;
		bool m_reset_pressed = false;
};
