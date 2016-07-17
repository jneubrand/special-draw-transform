#include <pebble.h>
#include <special-draw/special-draw.h>
#include "special-draw-transform.h"

#if defined(PBL_RECT) && !defined(SPECIAL_DRAW_SCREEN_SIZE)
#define SPECIAL_DRAW_SCREEN_SIZE (GSize(144, 168))
#elif defined(PBL_ROUND) && !defined(SPECIAL_DRAW_SCREEN_SIZE)
#define SPECIAL_DRAW_SCREEN_SIZE (GSize(180, 180))
#endif

static void prv_run_modifier(GSpecialSessionModifier * modifier,
        GBitmap * bitmap) {
    GRect bounds =
        ((GSpecialSessionTransformModifierData *)modifier->context)->area;
    GSpecialTransformationType type =
        ((GSpecialSessionTransformModifierData *)modifier->context)->type;

    int8_t x_to_x = (type >> 6) & 0b11;
    if (x_to_x == 3) x_to_x = -1;
    int8_t y_to_x = (type >> 4) & 0b11;
    if (y_to_x == 3) y_to_x = -1;
    int8_t x_to_y = (type >> 2) & 0b11;
    if (x_to_y == 3) x_to_y = -1;
    int8_t y_to_y = (type >> 0) & 0b11;
    if (y_to_y == 3) y_to_y = -1;

    GBitmap * saved_bitmap = gbitmap_create_blank(bounds.size,
        (gbitmap_get_format(bitmap) == GBitmapFormat1Bit ? GBitmapFormat1Bit : GBitmapFormat8Bit));

    if (!saved_bitmap) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Couldn't allocate temporary bitmap in "
                                     "special-draw-transform.");
        return;
    }

    for (int y = 0; y < bounds.size.h; y++) {
        GBitmapDataRowInfo row_info =
            gbitmap_get_data_row_info(bitmap, y + bounds.origin.y);
        GBitmapDataRowInfo row_info_temp =
            gbitmap_get_data_row_info(saved_bitmap, y);

        for (int x = 0; x < bounds.size.w; x++) {
#ifdef PBL_BW
            if (x + bounds.origin.x < row_info.min_x ||
                x + bounds.origin.x > row_info.max_x) {
                continue;
            } else {
                bool pixel = row_info.data[(x + bounds.origin.x)/8] &
                    (1 << ((x + bounds.origin.x) % 8));
                APP_LOG(200, "%i", row_info.data[(x + bounds.origin.x)/8]);
                if (pixel) {
                    APP_LOG(200, "x %i y %i", x, y);
                }
                row_info_temp.data[x/8] |= pixel << (x % 8);
            }
#else
            if (x + bounds.origin.x < row_info.min_x ||
                x + bounds.origin.x > row_info.max_x) {
                row_info_temp.data[x] = GColorClearARGB8;
            } else {
                row_info_temp.data[x] = row_info.data[x + bounds.origin.x];
            }
#endif
        }
    }
    for (int y = 0; y < bounds.size.h; y++) {
        GBitmapDataRowInfo row_info =
            gbitmap_get_data_row_info(bitmap, y + bounds.origin.y);
        for (int x = 0; x < bounds.size.w; x++) {
            if (x + bounds.origin.x < row_info.min_x ||
                x + bounds.origin.x > row_info.max_x) {
                continue;
            }
            int16_t x_transposed = x * x_to_x + y * y_to_x;
            if (x_to_x + y_to_x < 0) x_transposed += bounds.size.w - 1;
            int16_t y_transposed = y * y_to_y + x * x_to_y;
            if (x_to_y + y_to_y < 0) y_transposed += bounds.size.h - 1;

            GBitmapDataRowInfo row_info_saved =
                gbitmap_get_data_row_info(saved_bitmap,
                     y_transposed);
#ifdef PBL_BW
            bool pixel = row_info_saved.data[x_transposed/8] &
                (1 << (x_transposed % 8));
            if (pixel) {
                row_info.data[(x + bounds.origin.x)/8] |=
                    1<<((x+bounds.origin.x) % 8);
            } else {
                row_info.data[(x + bounds.origin.x)/8] &=
                    ~(1<<((x+bounds.origin.x) % 8));
            }
#else
            row_info.data[x + bounds.origin.x] =
                row_info_saved.data[x_transposed];
#endif
        }
    }
    gbitmap_destroy(saved_bitmap);
}

static void prv_destroy_modifier(GSpecialSessionModifier * modifier) {
    free(modifier->context);
    free(modifier);
}

GSpecialSessionModifier * graphics_special_draw_create_transform_modifier(
        GRect area, GSpecialTransformationType type) {
    // Create the modifier.
    GSpecialSessionModifier * mod = malloc(sizeof(GSpecialSessionModifier));
    mod->overrides_draw = false;
    mod->action.modifier_run = prv_run_modifier;
    mod->destroy = prv_destroy_modifier;
    mod->context = malloc(sizeof(GSpecialSessionTransformModifierData));
    ((GSpecialSessionTransformModifierData *)mod->context)->type = type;
    ((GSpecialSessionTransformModifierData *)mod->context)->area = area;
    return mod;
}
