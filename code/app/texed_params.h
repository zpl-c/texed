#pragma once

typedef enum {
    TPARAM_FLOAT,
    TPARAM_COORD,
    TPARAM_INT,
    TPARAM_COLOR,
    TPARAM_STRING,
    TPARAM_SLIDER,
    
    TPARAM_FORCE_UINT8 = UINT8_MAX
} td_param_kind;

typedef struct {
    td_param_kind kind;
    char const *name;
    char str[1000];
    bool edit_mode;
    
    union {
        struct {
            float flt, old_flt;
        };
        uint32_t u32;
        int32_t i32;
        struct {
            Color color, old_color;
        };
        char copy[4];
    };
} td_param;

typedef enum {
    TCAT_STACK,
    TCAT_GEN,
    TCAT_DRAW,
    TCAT_MOD,
    
    TCAT_FORCE_UINT8 = UINT8_MAX
} tcat_kind;
