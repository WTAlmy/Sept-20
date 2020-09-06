static PImage fortBody;

final float spawnRadius = 50;

public class Base {
  
  // Object Storage
  Tile location;
  PVector position;
  
  // Information
  int team;
  color teamColor;
  
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
    circle(position.x, position.y, spawnRadius * 2.0);
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
    float yPos = random(height / 2.0);
    if (i % 2 == 0) yPos += height / 2.0;
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
