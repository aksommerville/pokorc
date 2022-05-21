#include "data.h"
#include "dancer.h"

/* Songs.
 */
 
const struct songinfo songinfov[]={
  {
    .song=tinylamb,
    .name="Mary Had a Tiny Lamb",
    .dancerid=DANCER_ID_MARY,
    .songid=1,
  },
  {
    .song=cabbages,
    .name="Cabbages",
    .dancerid=DANCER_ID_RACCOON,//TODO
    .songid=4,
  },
  {
    .song=goatpotion,
    .name="Goat Potion",
    .dancerid=DANCER_ID_RACCOON,
    .songid=2,
  },
  {
    .song=elfonahill,
    .name="Elf on a Hill",
    .dancerid=DANCER_ID_ELF,
    .songid=6,
  },
  {
    .song=inversegamma,
    .name="Inverse Gamma",
    .dancerid=DANCER_ID_ASTRONAUT,
    .songid=3,
  },
  {
    .song=cobweb,
    .name="Cobweb",
    .dancerid=DANCER_ID_RACCOON,//TODO
    .songid=5,
  },
};

const uint8_t songinfoc=sizeof(songinfov)/sizeof(struct songinfo);
