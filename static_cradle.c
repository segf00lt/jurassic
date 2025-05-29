
#include "jurassic.c"

int main(void) {
  Game *gp = game_init();

  while(!gp->quit) {
    game_update_and_draw(gp);
  }

  game_close(gp);

  return 0;
}
