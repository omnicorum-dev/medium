//
// Created by Nico Russo on 4/24/26.
//

#ifndef MEDIUM_EVENTTYPES_H
#define MEDIUM_EVENTTYPES_H

#define MED_RELEASE                0
#define MED_PRESS                  1
#define MED_REPEAT                 2


#define MED_HAT_CENTERED           0
#define MED_HAT_UP                 1
#define MED_HAT_RIGHT              2
#define MED_HAT_DOWN               4
#define MED_HAT_LEFT               8
#define MED_HAT_RIGHT_UP           (MED_HAT_RIGHT | MED_HAT_UP)
#define MED_HAT_RIGHT_DOWN         (MED_HAT_RIGHT | MED_HAT_DOWN)
#define MED_HAT_LEFT_UP            (MED_HAT_LEFT  | MED_HAT_UP)
#define MED_HAT_LEFT_DOWN          (MED_HAT_LEFT  | MED_HAT_DOWN)

#endif //MEDIUM_EVENTTYPES_H
