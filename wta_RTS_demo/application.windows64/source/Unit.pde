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

final color PLAYER_COLOR = color(90, 120, 255);
final color COMPUTER_COLOR = color(255, 100, 100);

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
  color teamColor;
  
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
      heading = lerp(heading, current.heading, 0.1);
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
      gunHeading = lerp(gunHeading, heading, 0.15);
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
      adjustedRange *= 0.8;
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
