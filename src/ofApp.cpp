#include "ofApp.h"

#include <string>

using std::string;

const string SET_TILES_FOLDER_PATH = "Tilesets/Knots";
const string SET_XML_PATH = "Tilesets/Knots.xml";

//--------------------------------------------------------------
void ofApp::setup(){
	std::string xml_path = ofToDataPath(SET_XML_PATH, true);
	std::string images_folder_path = ofToDataPath(SET_TILES_FOLDER_PATH, true);
	
	m_tile_set = std::make_unique<TileSet>(xml_path, images_folder_path);
	m_tile_map_generator = std::make_unique<TileMapGenerator>(*m_tile_set);

	// m_current_tile_map = std::make_unique<TileMapGenerator::TileMap>(m_tile_map_generator->generate_tile_map(12, 9));
	m_current_tile_map = std::make_unique<TileMapGenerator::TileMap>(m_tile_map_generator->init_tile_map(12, 9));
	ofSetFrameRate(12);
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
	ofBackground(ofColor::black);

	m_tile_map_generator->generate_single_step(*m_current_tile_map);
	m_tile_map_generator->DrawTileMap(*m_current_tile_map);
}

//--------------------------------------------------------------
void ofApp::exit(){

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseScrolled(int x, int y, float scrollX, float scrollY){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
