#ifndef EXTERNALVAR_H_
#define EXTERNALVAR_H_

#include <stdint.h>
#include "meshStructure.h"

extern uint8_t *depth_mid, *depth_front, *depth_copy;
extern uint16_t  *depth, *depth_mesh;

extern bool got_depth, copyActive;

extern Group grupo;

#endif /* EXTERNALVAR_H_ */
