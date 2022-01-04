#pragma once
#include "const_buffer.h"
#include "mutable_buffer.h"

namespace smartroutine {
MutableBuffer buffer(void *data, size_t size_in_bytes);

ConstBuffer buffer(const void *data, size_t size_in_bytes);
}