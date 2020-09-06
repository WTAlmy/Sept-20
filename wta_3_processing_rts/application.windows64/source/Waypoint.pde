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
    if (progress > 0.5) {
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
      return 0.2;
    } else if (ground > soilLevel) {
      return 1.0;
    } else if (ground > sandLevel) {
      return 0.4;
    } else {
      return 0.2;
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
