#include "data.h"

/* Songs.
 */
 
const struct songinfo songinfov[]={
  {
    .song=inversegamma,
    .name="Inverse Gamma",
  },
  {
    .song=goatpotion,
    .name="Goat Potion",
  },
  {
    .song=tinylamb,
    .name="Mary Had a Tiny Lamb",
  },
};

const uint8_t songinfoc=sizeof(songinfov)/sizeof(struct songinfo);
