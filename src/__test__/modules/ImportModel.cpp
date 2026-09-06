import carcer.model;

int main() {
  model::World world;
  model::Player player;
  return world.combat.active || !player.party.empty();
}
