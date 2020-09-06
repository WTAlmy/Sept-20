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
      if (target.waypoints.size() < 1) damage *= 0.7;
      target.health -= damage;
      return true;
    }
    return false;
  }
  
}
