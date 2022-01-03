#pragma once
#include "errno.h"
namespace smartroutine {

enum CodeVal {
    Success = 0,
    InvalidArgument,
    InsufficientResources,
    OutOfMemory,
    PermissionDenied,
    AddressInUse
};

class ErrorCode {
  public:
    ErrorCode(CodeVal val) : val_(val) {}
    operator bool() const { return fail(); }
    bool fail() const { return val_ != Success; }

  private:
    CodeVal val_;
};
}