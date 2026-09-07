import carcer.ui.widgets;

int main() {
  ui::QuadProps quad;
  quad.width = 17;
  ui::ChCompactInfoProps character;
  character.hp = 3;
  return quad.width == 17 && character.hp == 3 ? 0 : 1;
}
