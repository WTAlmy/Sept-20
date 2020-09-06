import processing.core.*; 
import processing.data.*; 
import processing.event.*; 
import processing.opengl.*; 

import java.util.HashMap; 
import java.util.ArrayList; 
import java.io.File; 
import java.io.BufferedReader; 
import java.io.PrintWriter; 
import java.io.InputStream; 
import java.io.OutputStream; 
import java.io.IOException; 

public class wta_RTS_demo extends PApplet {

final int windowX = 900;
final int windowY = 600;

final int tileSize = 30;
final int gridSizeX = windowX / tileSize;
final int gridSizeY = windowY / tileSize;

/* CONTROLS
  1 - creates friendly tank at price of $100 at tile within radius of friendly base
  2 - creates hostile tank at price of $100 at tile within radius of enemy base
  3 - DEBUG: creates friendly base at mouse
  4 - DEBUG: creates hostile base at mouse
  R: Reset
  LMB: Click and Drag to move units: speed based on terrain, movement reduces damage
*/

// Object storage
Tile[][] tiles;
PVector center;
ArrayList<Base> bases;
ArrayList<Unit> units;
ArrayList<Bullet> bullets;

// Logic variables
int lastTick;
int playerWork;
int computerWork;
float lastFrame;
float deltaTime;

public void settings () {
  size(windowX, windowY);
  pixelDensity(2);
}

public void setup () {
  smooth();
  rectMode(CENTER);
  imageMode(CENTER);
  frameRate(60);
  noiseSeed(millis());
  center = new PVector(width/2, height/2);
  bases = new ArrayList<Base>();
  units = new ArrayList<Unit>();
  bullets = new ArrayList<Bullet>();
  // Images
  fortBody = loadImage("fortBody.png");
  tankBody = loadImage("tankBody.png");
  tankTurret = loadImage("tankTurret.png");
  bulletBody = loadImage("bullet.png");
  // Creation Methods
  generateTiles();
  generateBases();
  generateTerrain();
  lastTick = millis();
  lastFrame = millis();
  // Values
  playerWork = 300;
  computerWork = 300;
}

public void draw () {
  // Frame Direct
  deltaTime = (millis() - lastFrame) / 1000.0f;
  lastFrame = millis();
  // Frame Methods
  drawTiles();
  drawBullets();
  drawBases();
  drawUnits();
  drawSelection();
  drawInformationGUI();
  // Currency
  if (millis() - lastTick >= 1000) {
    lastTick += 1000;
    currencyTick();
  }
  // Exits draw loop if all bases captured
  int winner = 1;
  for (Base base : bases) {
    if (base.team != winner) {
      return;
    }
  }
  delay(2000);
  exit();
}

public void drawTiles () {
  background(water);
  for (Tile[] row : tiles) {
    for (Tile tile : row) {
      tile.show();
    }
  }
}

public void drawBullets () {
  for (int i = 0; i < bullets.size(); i++) {
    Bullet bullet = bullets.get(i);
    if (bullet.checkHit()) {
      bullets.remove(bullet);
      i--;
    } else {
      bullet.show();
    }
  }
}

public void drawUnits () {
  for (int i = 0; i < units.size(); i++) {
    Unit unit = units.get(i);
    unit.update();
    if (unit.dead()) {
      units.remove(i);
      unit.location.occupant = null;
      i--;
    } else {
      unit.show();
    }
  }
}

public void drawBases () {
  for (Base base : bases) {
    base.show();
  }
}

public void drawInformationGUI () {
  fill(150);
  stroke(255);
  rect(windowX - tileSize * 2, windowY - tileSize * 2, tileSize * 3, tileSize * 3);
  fill(255);
  textSize(32);
  textAlign(CENTER, CENTER);
  text("$" + playerWork, windowX - tileSize * 2, windowY - tileSize * 2.5f);
  textSize(16);
  text("C: $" + computerWork, windowX - tileSize * 2, windowY - tileSize * 1.5f);
}

public void currencyTick () {
  playerWork += numberOwned(PLAYER) * 2;
  computerWork += numberOwned(COMPUTER) * 2;
}
static PImage fortBody;

final float spawnRadius = 50;

public class Base {
  
  // Object Storage
  Tile location;
  PVector position;
  
  // Information
  int team;
  int teamColor;
  
  public Base (Tile initial, int team) {
    this.team = team;
    location = initial;
    location.base = this;
    position = location.position.copy();
    teamColor = color(255);
    if (team == PLAYER) teamColor = PLAYER_COLOR;
    if (team == COMPUTER) teamColor = COMPUTER_COLOR;
    bases.add(this);
  }
  
  public void show () {
    tint(teamColor);
    image(fortBody, position.x, position.y, tileSize, tileSize);
    fill(teamColor, 75);
    stroke(255);
    circle(position.x, position.y, spawnRadius * 2.0f);
  }
  
  public void capture (int owner) {
    team = owner;
    if (team == PLAYER) teamColor = PLAYER_COLOR;
    if (team == COMPUTER) teamColor = COMPUTER_COLOR;
  }
  
}

public void generateBases () {
  int numBases = 7;
  for (int i = 1; i <= numBases; i++) {
    float xPos = width * (float)i / ((float)numBases + 1);
    float yPos = random(height / 2.0f);
    if (i % 2 == 0) yPos += height / 2.0f;
    Tile baseLocation = positionTile(xPos, yPos);
    Base base;
    if (i == 1) {
      base = new Base(baseLocation, COMPUTER);
      base.show();
    } else if (i < numBases) {
      base = new Base(baseLocation, 0);
      base.show();
    } else if (i >= numBases) {
      base = new Base(baseLocation, PLAYER);
      base.show();
    }
  }
}

public int numberOwned (int team) {
  int owned = 0;
  for (Base base : bases) {
    if (base.team == team) {
      owned++;
    }
  }
  return owned;
}

public float distanceFromBase (PVector position, int team) {
  float minimumDist = windowX * windowY;
  for (Base base : bases) {
    if (base.team == team) {
      float dist = base.position.dist(position);
      if (dist < minimumDist) {
        minimumDist = dist;
      }
    }
  }
  return minimumDist;
}
static PImage bulletBody;

public class Bullet {
  
  Unit owner, target;
  
  float heading;
  PVector position;
  
  float size = 6;
  float speed = 400;
  float damage = 20;
  
  public Bullet (Unit owner, Unit target) {
    this.owner = owner;
    this.target = target;
    heading = PVector.sub(target.position, owner.position).heading();
    position = owner.position.copy();
  }
  
  public void show () {
    tint(owner.teamColor);
    image(bulletBody, position.x, position.y, size, size);
  }
  
  public boolean checkHit () {
    
    // Movement
    PVector bearing = PVector.sub(target.position, position);
    heading = bearing.heading();
    bearing.normalize();
    bearing.mult(deltaTime * speed);
    position.add(bearing);
    
    // Collision
    if (position.dist(target.position) < size) {
      // Reduces incoming damage if not moving
      if (target.waypoints.size() < 1) damage *= 0.7f;
      target.health -= damage;
      return true;
    }
    return false;
  }
  
}
final int sand = color(194, 178, 128);
final int water = color(0, 0, 128);
final int soil = color(0, 170, 0);
final int forest = color(0, 120, 0);

final static float waterSmooth = 0.10f;

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

public void generateTerrain () {
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
  sandLevel = mean - range / 10.0f;
  soilLevel = mean;
  forestLevel = mean + range / 6.0f;
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
// Team constants
final static int NEUTRAL = 0;
final static int PLAYER = 1;
final static int COMPUTER = 2;

// Not very proper
final static int SOLDIER = 1;
final static int SNIPER = 2;
final static int TANK = 3;
final static int SHIP = 4;
final static int PLANE = 5;

final int PLAYER_COLOR = color(90, 120, 255);
final int COMPUTER_COLOR = color(255, 100, 100);

static PImage tankBody;
static PImage tankTurret;

public class Unit {
  
  // Object Storage
  Tile location;
  PVector position;
  ArrayList<Waypoint> waypoints;
  
  // Set once
  float speed;
  float range;
  float maxHealth;
  float reloadTime;
  int teamColor;
  
  // Information
  int type;
  int team;
  
  // Logic
  int lastFired;
  float health;
  float heading;
  float gunHeading;
  
  public Unit (Tile initial, int team, int type) {
    // Statistics
    this.type = type;
    this.team = team;
    this.location = initial;
    this.position = location.position.copy();
    this.waypoints = new ArrayList<Waypoint>();
    buildStats();
    units.add(this);
  }
  
  public boolean dead () {
    return health <= 0;
  }
  
  public void show () {
    
    // Character
    if (type == TANK) {
      tint(teamColor);
      pushMatrix();
      translate(position.x, position.y);
      rotate(heading + HALF_PI);
      image(tankBody, 0, 0, 14, 20);
      translate(0, 0);
      rotate(gunHeading - heading);
      image(tankTurret, 0, -4, 10, 18);
      popMatrix();
    }
    
    // Health bar
    fill(255, 127);
    noStroke();
    rect(position.x, position.y - 10, 16, 4);
    fill(0, 255, 0);
    float hp = health / maxHealth;
    rect(position.x, position.y - 10, 14 * hp, 2);
    
  }
  
  public void move (ArrayList<Tile> path) {
    // Checks over all tiles to prevent duplication errors
    for (Tile[] row : tiles) {
      for (Tile tile : row) {
        if (tile.occupant == this && tile != location) {
          tile.occupant = null;
        } else if (tile.occupant != null && tile.occupant.dead()) {
          tile.occupant = null;
        }
      }
    }
    // If unit currently moving, don't interrupt until it finishes
    if (waypoints.size() >= 1) {
      waypoints.subList(0, waypoints.size()).clear();
    }
    // Generate a list of waypoints given coordinates of tiles
    for (int i = 0; i < path.size() - 1; i++) {
      Tile to = path.get(i+1);
      Tile from = path.get(i);
      Waypoint segment = new Waypoint(to, from, this);
      waypoints.add(segment);
    }
  }
  
  public void update () {
    // If it can move, it moves
    if (waypoints.size() > 0) {
      Waypoint current = waypoints.get(0);
      boolean finished = current.update();
      // Smooth rotation
      heading = lerp(heading, current.heading, 0.1f);
      if (finished) waypoints.remove(current);
    } 
    // If there are enemies in range, check if it can fire
    Unit closestEnemy = closestEnemy();
    if (millis() - lastFired > reloadTime) {
      if (closestEnemy != null) {
        lastFired = millis();
        Bullet bullet = new Bullet(this, closestEnemy);
        bullets.add(bullet);
        gunHeading = bullet.heading;
      }
    }
    // Reset turret to center if no enemies in range
    if (closestEnemy == null) {
      // Smooth
      gunHeading = lerp(gunHeading, heading, 0.15f);
    }
  }
  
  public Unit closestEnemy () {
    
    // Initialize
    Unit closest;
    float closestDistance;
    ArrayList<Unit> enemyUnits = new ArrayList<Unit>();
    
    // Find qualified units
    float adjustedRange = range;
    if (waypoints.size() > 0) {
      adjustedRange *= 0.8f;
    }
    for (Unit unit : units) {
      float localDistance = position.dist(unit.position);
      if (team != unit.team && localDistance <= adjustedRange) {
        enemyUnits.add(unit);
      }
    }
    if (enemyUnits.size() < 1) return null;
    
    // Find closest unit
    closest = enemyUnits.get(0);
    closestDistance = position.dist(closest.position);
    for (Unit unit : enemyUnits) {
      float localDistance = position.dist(unit.position);
      if (localDistance < closestDistance) {
        closest = unit;
        closestDistance = localDistance;
      }
    }
    return closest;
    
  }
  
  public void buildStats () {
    if (type == TANK) {
      speed = 55;
      range = 200;
      maxHealth = 200;
      reloadTime = 800;
    }
    heading = gunHeading = PVector.sub(center, position).heading();
    teamColor = color(255);
    if (team == PLAYER) teamColor = PLAYER_COLOR;
    if (team == COMPUTER) teamColor = COMPUTER_COLOR;
    health = maxHealth;
    lastFired = millis();
  }
  
}
static Unit selectedUnit;
static ArrayList<Tile> selectedTiles;

public void mousePressed () {
  
  Tile location = mouseTile();
  selectedUnit = location.occupant;
  
  selectedTiles = new ArrayList<Tile>();
  selectedTiles.add(location);
  
}

public void mouseDragged () {
  if (selectedUnit != null) {
    Tile location = mouseTile();
    if (selectedTiles.contains(location) == false) {
      selectedTiles.add(location);
      // Creates diagonals
      findShortcuts();
    }
  }
}

public void mouseReleased () {
  if (selectedTiles.size() > 1) {
    selectedUnit.move(selectedTiles);
    selectedTiles.clear();
  }
}

public void keyPressed () {
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
public class Waypoint {
  
  // Object storage
  Unit unit;
  Tile next, from;
  PVector startPosition;
  
  // Logic stuff
  float heading;
  float distance;
  float startTime;
  float finishTime;
  float completionTime;
  boolean activated;
  
  public Waypoint (Tile to, Tile from, Unit unit) {
    activated = false;
    this.next = to;
    this.from = from;
    this.unit = unit;
  }
  
  public void show () {
    stroke(0, 255, 0);
    if (this == unit.waypoints.get(0)) {
      line(unit.position.x, unit.position.y, next.position.x, next.position.y);
    } else {
      line(from.position.x, from.position.y, next.position.x, next.position.y);
    }
  }
  
  private boolean activate () {
    // Checks if the unit can move to the next tile or not
    if (next.occupant == null) {
      
      // Set Variables
      activated = true;
      startTime = millis();
      startPosition = unit.position;
      heading = PVector.sub(next.position, startPosition).heading();
      distance = next.position.dist(startPosition);
      
      // Accounts for terrain vs. unit speed
      float adjustedSpeed = unit.speed * speedMultiplier(unit, from, next);
      finishTime = startTime + 1000 * (distance / adjustedSpeed);
      completionTime = finishTime - startTime;
      return true;
      
    }
    unit.waypoints.clear();
    return false;
  }
  
  public boolean update () {
    
    // Check if Unit can proceed
    if (activated == false) activate();
    if (activated == false) return false;
    
    float currentTime = millis();
    float timeElapsed = currentTime - startTime;
    float progress = constrain(timeElapsed / completionTime, 0, 1);
    
    // Move Unit
    PVector position = PVector.lerp(startPosition, next.position, progress);
    unit.position = position;
    
    // Check if Unit in new tile
    if (progress > 0.5f) {
      from.occupant = null;
      next.occupant = unit;
      unit.location = next;
      if (next.base != null) {
        next.base.capture(unit.team);
      }
    }
    
    // Check if Unit reached destination
    if (currentTime > finishTime) return true;
    return false;
    
  }
  
}

public float tileMultiplier (int type, float ground) {
  // Soldiers are not impeded by tile type
  if (type == TANK) {
    if (ground > forestLevel) {
      return 0.2f;
    } else if (ground > soilLevel) {
      return 1.0f;
    } else if (ground > sandLevel) {
      return 0.4f;
    } else {
      return 0.2f;
    }
  }
  return 1;
}

public float speedMultiplier (Unit unit, Tile tile1, Tile tile2) {
  // Provides an adjusted speed based on unit type and terrain
  int type = unit.type;
  float level1 = tile1.groundLevel;
  float level2 = tile2.groundLevel;
  float mult1 = tileMultiplier(type, level1);
  float mult2 = tileMultiplier(type, level2);
  return max(mult1, mult2);
}
  static public void main(String[] passedArgs) {
    String[] appletArgs = new String[] { "wta_RTS_demo" };
    if (passedArgs != null) {
      PApplet.main(concat(appletArgs, passedArgs));
    } else {
      PApplet.main(appletArgs);
    }
  }
}
