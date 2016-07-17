#pragma once
#include <pebble.h>

typedef enum {
    GSpecialTransformMirrorYR270 = 0b00010100,
    GSpecialTransformRotate90    = 0b00011100,
    GSpecialTransformRotate270   = 0b00110100,
    GSpecialTransformMirrorYR90  = 0b00111100,
    GSpecialTransformIdentity    = 0b01000001,
    GSpecialTransformMirrorX     = 0b01000011,
    GSpecialTransformMirrorY     = 0b11000001,
    GSpecialTransformRotate180   = 0b11000011,
} GSpecialTransformationType;

typedef struct {
    GSpecialTransformationType type;
    GRect area;
} GSpecialSessionTransformModifierData;

GSpecialSessionModifier * graphics_special_draw_create_transform_modifier(
    GRect area, GSpecialTransformationType type);
