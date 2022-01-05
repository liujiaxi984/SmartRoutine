#include "common/buffer.h"

namespace smartroutine {
MutableBuffer buffer(void *data, size_t size_in_bytes) {
    MutableBuffer mutable_buffer(data, size_in_bytes);
    return mutable_buffer;
}

ConstBuffer buffer(const void *data, size_t size_in_bytes) {
    ConstBuffer const_buffer(data, size_in_bytes);
    return const_buffer;
}
}