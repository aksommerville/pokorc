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
    .song=goatpotion,
    .name="Goat Potion",
    .dancerid=DANCER_ID_RACCOON,
    .songid=2,
  },
  {
    .song=inversegamma,
    .name="Inverse Gamma",
    .dancerid=DANCER_ID_ASTRONAUT,
    .songid=3,
  },
  /* Three slots available. We can't fit more than 6 on the menu.
  {
    .song=0,
    .name="",
    .dancerid=DANCER_ID_NONE,
    .songid=4,
  },
  {
    .song=0,
    .name="",
    .dancerid=DANCER_ID_NONE,
    .songid=5,
  },
  {
    .song=0,
    .name="",
    .dancerid=DANCER_ID_NONE,
    .songid=6,
  },
  /**/
};

const uint8_t songinfoc=sizeof(songinfov)/sizeof(struct songinfo);
