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

void settings () {
  size(windowX, windowY);
  pixelDensity(2);
}

void setup () {
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

void draw () {
  // Frame Direct
  deltaTime = (millis() - lastFrame) / 1000.0;
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
  text("$" + playerWork, windowX - tileSize * 2, windowY - tileSize * 2.5);
  textSize(16);
  text("C: $" + computerWork, windowX - tileSize * 2, windowY - tileSize * 1.5);
}

public void currencyTick () {
  playerWork += numberOwned(PLAYER) * 2;
  computerWork += numberOwned(COMPUTER) * 2;
}
