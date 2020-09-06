final color sand = color(194, 178, 128);
final color water = color(0, 0, 128);
final color soil = color(0, 170, 0);
final color forest = color(0, 120, 0);

final static float waterSmooth = 0.10;

static float sandLevel;
static float soilLevel;
static float forestLevel;

public class Tile {
  
  int gridX, gridY;
  PVector position;
  
  Base base;
  Unit occupant;
  float groundLevel;
  
  public Tile (int x, int y) {
    gridX = x;
    gridY = y;
    float pX = gridX * tileSize + tileSize / 2;
    float pY = gridY * tileSize + tileSize / 2;
    position = new PVector(pX, pY);
    groundLevel = noise(x * waterSmooth, y * waterSmooth);
  }
  
  public void show () {
    if (groundLevel > forestLevel) {
      fill(forest);
    } else if (groundLevel > soilLevel) {
      fill(soil);
    } else if (groundLevel > sandLevel) {
      fill(sand);
    } else if (occupant != null) {
      // Assigns an outline to units in the water
      fill(water);
    }
    stroke(255, 255, 255, 100);
    if (groundLevel > sandLevel) {
      rect(position.x, position.y, tileSize, tileSize);
    }
  }
  
}

void generateTerrain () {
  // Generates water, forest, sand assignments GIVEN ground level
  float totalTiles = gridSizeX * gridSizeY;
  float totalHeight = 0;
  float minimumHeight = 0;
  float maximumHeight = 0;
  for (Tile[] tileRow : tiles) {
    for (Tile tile : tileRow) {
      float g = tile.groundLevel;
      totalHeight += g;
      if (g > maximumHeight) maximumHeight = g;
      if (g < minimumHeight) minimumHeight = g;
    }
  }
  // Straddles the terrain generation to prevent all-water/all-forest maps
  float range = maximumHeight - minimumHeight;
  float mean = totalHeight / totalTiles;
  sandLevel = mean - range / 10.0;
  soilLevel = mean;
  forestLevel = mean + range / 6.0;
}

public Tile positionTile (float x, float y) {
  int tileX = (int) constrain(x / tileSize, 0, gridSizeX - 1);
  int tileY = (int) constrain(y / tileSize, 0, gridSizeY - 1);
  Tile location = tiles[tileX][tileY];
  return location;
}

public void generateTiles () {
  tiles = new Tile[gridSizeX][gridSizeY];
  for (int x = 0; x < gridSizeX; x++) {
    for (int y = 0; y < gridSizeY; y++) {
      Tile tile = new Tile(x, y);
      tiles[x][y] = tile;
      // Prevents tiles from being drawn over currency GUI
      if (x > gridSizeX - 5 && y > gridSizeY - 5) {
        tile.groundLevel = 0;
      }
    }
  }
}
