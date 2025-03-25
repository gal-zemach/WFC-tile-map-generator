# Wave Function Collapse (WFC) in C++

A simple C++ implementation of the Wave Function Collapse algorithm for **procedural tile generation**.  
This project demonstrates how local constraints and probabilistic choices can create unique 2D layouts with minimal hardcoding.  
Drawing to the screen is done using openFrameworks.  

<img src="./assets/DemoGif/KnotsTilesetDemo.gif" alt="WFC Demo Screenshot" width="400"/>

## Table of Contents
1. [Overview](#overview)
2. [Features](#features)
3. [Project Structure](#project-structure)
4. [Future Improvements](#future-improvements)
5. [Credits](#credits)
6. [License](#license)

---

## Overview
Wave Function Collapse (WFC) is a constraint-solving algorithm inspired by quantum mechanics. In game development and other creative coding fields, WFC helps to generate 2D or 3D layouts that respect adjacency rules—producing results that look handcrafted yet are algorithmically generated.

**Key points for this project:**
- Implemented in **C++**.
- Utilizes **tile adjacency rules** to generate valid layouts.
- Demonstrates **randomized** but coherent pattern generation.

---

## Features
- **2D tile map generation based on Edge constraints**
  Automatically generates a tile-based map based on an XML that defines each tile's borders.
- **Symmetry Rules**
  Tiles have several symmetry options reducing redundant XML definitions.
- **Simple & In Development**
  Did this is for my personal learning, so I keep adding optimizations and features as I go.

---

## Project Structure
```
WFC-tile-map-generator/
├── README.md
├── src/
│   └── main.cpp
│   └── ofApp.h
│   └── ofApp.cpp
│   └── TileMapGenerator.h
│   └── TileMapGenerator.cpp
│   └── Data
│       └── Tile.h
│       └── Tile.cpp
│       └── TileSet.h
│       └── Tile.cpp
└── data/
    └── TileSets
        └── Knots.xml
        └── Knots
            └── corner.png
            └── cross.png
            └── empty.png
            └── line.png
            └── t.png
```

- **README.md**: This file, explaining the project.
- **ofApp**: Actual entry point for the tile map generation and drawing.
- **TileMapGenerator**: Holds the current tile map. Allows generating it fully/step-by-step.
- **TileSet**: Holds the parsed tile set and builds the adjacency rules.
- **Tile**: Holds a single tile's data.
- **data/TilSets**: Contains the tile set. Each tile set is comprised of an XML and an images folder.

---

## Credits
**Tile Set**
- This project uses a slightly modified version of the [**Knots Set**](https://github.com/mxgmn/WaveFunctionCollapse/) by Maxim Gumin.

**Algorithm Reference**
- [Wave Function Collapse Main GitHub](https://github.com/mxgmn/WaveFunctionCollapse/) by Maxim Gumin.
- [Simple WFC Explanation](https://www.boristhebrave.com/2020/04/13/wave-function-collapse-explained/) by BorisTheBrave.

---

## Future Improvements
- Performance: Working on improving neighbor-selection performance using a supported neighbors array.
- UI: Add a UI that allows visually selecting the probability for each tile's appearance.
- Generics: Make TileMapGenerator generic to decouple it from the Tile class.

---

## License
MIT License  
Copyright (c) 2025 Gal Zemach  
Permission is hereby granted, free of charge, to any person obtaining a copy  

---
