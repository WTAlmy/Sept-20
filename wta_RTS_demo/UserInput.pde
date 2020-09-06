static Unit selectedUnit;
static ArrayList<Tile> selectedTiles;

void mousePressed () {
  
  Tile location = mouseTile();
  selectedUnit = location.occupant;
  
  selectedTiles = new ArrayList<Tile>();
  selectedTiles.add(location);
  
}

void mouseDragged () {
  if (selectedUnit != null) {
    Tile location = mouseTile();
    if (selectedTiles.contains(location) == false) {
      selectedTiles.add(location);
      // Creates diagonals
      findShortcuts();
    }
  }
}

void mouseReleased () {
  if (selectedTiles.size() > 1) {
    selectedUnit.move(selectedTiles);
    selectedTiles.clear();
  }
}

void keyPressed () {
  Tile hover = mouseTile();
  if (key == 'r') {
    setup();
  } else if (hover.base == null && hover.occupant == null) {
    // Create unit if player has cash and is in range of friendly base
    if (key == '1' && distanceFromBase(hover.position, PLAYER) <= spawnRadius) {
      if (playerWork >= 100) {
        hover.occupant = new Unit(hover, PLAYER, TANK);
        playerWork -= 100;
      }
    } else if (key == '2' && distanceFromBase(hover.position, COMPUTER) <= spawnRadius) {
      if (computerWork >= 100) {
        hover.occupant = new Unit(hover, COMPUTER, TANK);
        computerWork -= 100;
      }
    } else if (key == '3') {
      hover.base = new Base(hover, PLAYER);
    } else if (key == '4') {
      hover.base = new Base(hover, COMPUTER);
    }
  }
}

public Tile mouseTile () {
  // Get the grid square closest to the cursor's location
  int tileX = constrain(mouseX / tileSize, 0, gridSizeX - 1);
  int tileY = constrain(mouseY / tileSize, 0, gridSizeY - 1);
  Tile location = tiles[tileX][tileY];
  return location;
}

// Draws the coordinate path the user is creating in progress
public void drawSelection () {
  noFill();
  stroke(0, 255, 0);
  if (selectedTiles != null && selectedTiles.size() > 1) {
    for (int i = 0; i < selectedTiles.size() - 1; i++) {
      Tile t1 = selectedTiles.get(i);
      Tile t2 = selectedTiles.get(i+1);
      if (t1.occupant == selectedUnit) {
        line(selectedUnit.position.x, selectedUnit.position.y, t2.position.x, t2.position.y);
      } else {
        circle(t1.position.x, t1.position.y, 10);
        line(t1.position.x, t1.position.y, t2.position.x, t2.position.y);
      }
    }
  }
}

public void findShortcuts () {
  // Simplifies (straight right, straight up) to diagonal right-up
  if (selectedTiles.size() > 2) {
    for (int i = 0; i < selectedTiles.size() - 2; i++) {
      Tile tile0 = selectedTiles.get(i);
      Tile tile1 = selectedTiles.get(i+1);
      Tile tile2 = selectedTiles.get(i+2);
      int xDiff = tile0.gridX - tile2.gridX;
      int yDiff = tile0.gridY - tile2.gridY;
      if ( (xDiff == 1 || xDiff == -1) && (yDiff == 1 || yDiff == -1) ) {
        selectedTiles.remove(tile1);
        i--;
      }
    }
  }
}
