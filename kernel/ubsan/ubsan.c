#include "ubsan.h"
#include <garn/panic.h>
#include <garn/mm.h>

const char* typeStrings[] = {
    "load of",
    "store to",
    "reference binding to",
    "member access within",
    "member call on",
    "constructor call on",
    "downcast of",
    "downcast of",
    "upcast of",
    "cast to virtual base of",
};
 
void __ubsan_handle_type_mismatch_v1(type_mismatch_info_t* typeMismatch, void* pointer){
    source_location_t* location = &typeMismatch->location;
    if(!pointer) {
        panic("NULL pointer access (file: %s, line: %u, column: %u)", "ubsan", location->file, location->line, location->column);
    } else if (typeMismatch->alignment != 0 && ((uint64_t)pointer % typeMismatch->alignment) != 0) {
        klog("Unaligned memory access (file: %s, line: %u, column: %u)\n", KLOG_WARNING, "ubsan", location->file, location->line, location->column);
    } else {
        klog("Insufficient size: %s address %p with insufficient space for object of type %s (file: %s, line: %u, column: %u)\n", KLOG_WARNING, "ubsan", typeStrings[typeMismatch->typeCheckKind], pointer, typeMismatch->type->name, location->file, location->line, location->column);
    }
}

void __ubsan_handle_type_mismatch_v1_abort(type_mismatch_info_t* typeMismatch, void* pointer){
    __ubsan_handle_type_mismatch_v1(typeMismatch, pointer);
}

void __ubsan_handle_load_invalid_value(invalid_value_data_t* data, uint64_t value){
    source_location_t* location = &data->location;

    klog("Load of value 0x%x, which is not a valid value for type %s (file: %s, line: %u, column: %u)\n", KLOG_WARNING, "ubsan", value, data->type->name, location->file, location->line, location->column);
}

void __ubsan_handle_load_invalid_value_abort(invalid_value_data_t* data, uint64_t value){
    __ubsan_handle_load_invalid_value(data, value);
}

void __ubsan_handle_pointer_overflow(pointer_overflow_data_t* data, uint64_t base, uint64_t result){
    source_location_t* location = &data->location;

    if(base == 0 && result == 0)
        panic("Applying zero offset to null pointer (file: %s, line: %u, column: %u)", "ubsan", location->file, location->line, location->column);
    else if (base == 0 && result != 0)
        panic("Applying non-zero offset 0x%x to null pointer (file: %s, line: %u, column: %u)", "ubsan", result, location->file, location->line, location->column);
    else if (base != 0 && result == 0)
        panic("Applying non-zero offset to non-null pointer 0x%x produced null pointer (file: %s, line: %u, column: %u)", "ubsan", base, location->file, location->line, location->column);
    else
        panic("Pointer 0x%x overflowed to 0x%x (file: %s, line: %u, column: %u)", "ubsan", base, result, location->file, location->line, location->column);
}

void __ubsan_handle_pointer_overflow_abort(pointer_overflow_data_t* data, uint64_t base, uint64_t result){
    __ubsan_handle_pointer_overflow(data, base, result);
}

void __ubsan_handle_out_of_bounds(out_of_bounds_data_t* data, uint64_t index, uint64_t opts){
    source_location_t* location = &data->location;

    panic("Index 0x%x out of bounds for type %s (file: %s, line: %u, column: %u)", "ubsan", index, data->arrayType->name, location->file, location->line, location->column);
}

void __ubsan_handle_out_of_bounds_abort(out_of_bounds_data_t* data, uint64_t index, uint64_t opts){
    __ubsan_handle_out_of_bounds(data, index, opts);
}

void __ubsan_handle_shift_out_of_bounds(shift_out_of_bounds_data_t* data, uint64_t LHS, uint64_t RHS){
    source_location_t* location = &data->location;

    panic("Shift out of bounds (file: %s, line: %u, column: %u)", "ubsan", location->file, location->line, location->column);
}

void __ubsan_handle_shift_out_of_bounds_abort(shift_out_of_bounds_data_t* data, uint64_t LHS, uint64_t RHS){
    __ubsan_handle_shift_out_of_bounds(data, LHS, RHS);
}


void __ubsan_handle_divrem_overflow(overflow_data_t* data, uint64_t LHS, uint64_t RHS){
    source_location_t* location = &data->location;

    panic("Invalid division or divide by zero (file: %s, line: %u, column: %u)", "ubsan", location->file, location->line, location->column);
}

void __ubsan_handle_divrem_overflow_abort(overflow_data_t* data, uint64_t LHS, uint64_t RHS){
    __ubsan_handle_divrem_overflow(data, LHS, RHS);
}

void __ubsan_handle_negate_overflow(overflow_data_t* data, uint64_t oldValue){
    source_location_t* location = &data->location;

    panic("Negation of %u cannot be represented in type %s (file: %s, line: %u, column: %u)", "ubsan", oldValue, data->type->name, location->file, location->line, location->column);
}

void __ubsan_handle_negate_overflow_abort(overflow_data_t* data, uint64_t oldValue){
    __ubsan_handle_negate_overflow(data, oldValue);
}

void __ubsan_handle_add_overflow(overflow_data_t* data){
    source_location_t* location = &data->location;

    panic("Add overflow (file: %s, line: %u, column: %u)", "ubsan", location->file, location->line, location->column);
}

void __ubsan_handle_add_overflow_abort(overflow_data_t* data){
    __ubsan_handle_add_overflow(data);
}

void __ubsan_handle_sub_overflow(overflow_data_t* data){
    source_location_t* location = &data->location;

    panic("Subtract overflow (file: %s, line: %u, column: %u)", "ubsan", location->file, location->line, location->column);
}

void __ubsan_handle_sub_overflow_abort(overflow_data_t* data){
    __ubsan_handle_sub_overflow(data);
}

void __ubsan_handle_mul_overflow(overflow_data_t* data){
    source_location_t* location = &data->location;

    panic("Multiply overflow (file: %s, line: %u, column: %u)", "ubsan", location->file, location->line, location->column);
}

void __ubsan_handle_mul_overflow_abort(overflow_data_t* data){
    __ubsan_handle_mul_overflow(data);
}

void __ubsan_handle_builtin_unreachable(unreachable_data_t* data){
    source_location_t* location = &data->location;
    
    panic("__builtin_unreachable() reached! (file: %s, line: %u, column: %u)", "ubsan", location->file, location->line, location->column);
}
