#pragma once

#include <garn/types.h>

typedef struct {
    const char *file;
    uint32_t line;
    uint32_t column;
} source_location_t;
 
typedef struct {
    uint16_t kind;
    uint16_t info;
    char name[];
} type_descriptor_t;
 
typedef struct {
    source_location_t location;
    type_descriptor_t* type;
    uint8_t alignment;
    uint8_t typeCheckKind;
} type_mismatch_info_t;
 
typedef struct {
    source_location_t location;
    type_descriptor_t left_type;
    type_descriptor_t right_type;
} out_of_bounds_info_t;

typedef struct {
  source_location_t location;
  type_descriptor_t* type;
} invalid_value_data_t;

typedef struct {
    source_location_t location;
} pointer_overflow_data_t;

typedef struct {
    source_location_t location;
    type_descriptor_t* arrayType;
    type_descriptor_t* IndexType;
} out_of_bounds_data_t;

typedef struct {
    source_location_t location;
    type_descriptor_t* type;
} overflow_data_t;

typedef struct {
    source_location_t location;
} unreachable_data_t;

typedef struct {
    source_location_t location;
    type_descriptor_t* LHStype;
    type_descriptor_t* RHSType;
} shift_out_of_bounds_data_t;

void __ubsan_handle_type_mismatch_v1(type_mismatch_info_t* typeMismatch, void* pointer);
void __ubsan_handle_type_mismatch_v1_abort(type_mismatch_info_t* typeMismatch, void* pointer);

void __ubsan_handle_load_invalid_value(invalid_value_data_t* data, uint64_t value);
void __ubsan_handle_load_invalid_value_abort(invalid_value_data_t* data, uint64_t value);

void __ubsan_handle_pointer_overflow(pointer_overflow_data_t* data, uint64_t base, uint64_t result);
void __ubsan_handle_pointer_overflow_abort(pointer_overflow_data_t* data, uint64_t base, uint64_t result);

void __ubsan_handle_out_of_bounds(out_of_bounds_data_t* data, uint64_t index, uint64_t opts);
void __ubsan_handle_out_of_bounds_abort(out_of_bounds_data_t* data, uint64_t index, uint64_t opts);

void __ubsan_handle_shift_out_of_bounds(shift_out_of_bounds_data_t* data, uint64_t LHS, uint64_t RHS);
void __ubsan_handle_shift_out_of_bounds_abort(shift_out_of_bounds_data_t* data, uint64_t LHS, uint64_t RHS);

void __ubsan_handle_divrem_overflow(overflow_data_t* data, uint64_t LHS, uint64_t RHS);
void __ubsan_handle_divrem_overflow_abort(overflow_data_t* data, uint64_t LHS, uint64_t RHS);

void __ubsan_handle_negate_overflow(overflow_data_t* data, uint64_t oldValue);
void __ubsan_handle_negate_overflow_abort(overflow_data_t* data, uint64_t oldValue);

void __ubsan_handle_add_overflow(overflow_data_t* data);
void __ubsan_handle_add_overflow_abort(overflow_data_t* data);

void __ubsan_handle_sub_overflow(overflow_data_t* data);
void __ubsan_handle_sub_overflow_abort(overflow_data_t* data);

void __ubsan_handle_mul_overflow(overflow_data_t* data);
void __ubsan_handle_mul_overflow_abort(overflow_data_t* data);

void __ubsan_handle_builtin_unreachable(unreachable_data_t* data);
