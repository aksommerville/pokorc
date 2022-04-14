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
};

const uint8_t songinfoc=sizeof(songinfov)/sizeof(struct songinfo);
