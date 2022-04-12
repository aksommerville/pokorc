#include "data.h"
#include "dancer.h"

/* Songs.
 */
 
const struct songinfo songinfov[]={
  {
    .song=tinylamb,
    .name="Mary Had a Tiny Lamb",
    .dancerid=DANCER_ID_MARY,
  },
  {
    .song=goatpotion,
    .name="Goat Potion",
    .dancerid=DANCER_ID_RACCOON,
  },
  {
    .song=inversegamma,
    .name="Inverse Gamma",
    .dancerid=DANCER_ID_ASTRONAUT,
  },
};

const uint8_t songinfoc=sizeof(songinfov)/sizeof(struct songinfo);
