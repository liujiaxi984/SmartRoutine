#pragma once
#include "errno.h"
#include <string>
namespace smartroutine {

enum CodeVal {
    Success = 0,
    InvalidArgument,
    InsufficientResources,
    OutOfMemory,
    PermissionDenied,
    AddressInUse,
    ConnectionAborted
};

static const char *CodeValStr[] = {
    "Success",           "Invalid Argument",  "Insufficient Resources",
    "Out Of Memory",     "Permission Denied", "Address In Use",
    "Connection Aborted"};

class ErrorCode {
  public:
    ErrorCode() : ErrorCode(Success) {}
    ErrorCode(CodeVal val) : val_(val) {}
    operator bool() const { return fail(); }
    bool fail() const { return val_ != Success; }
    std::string to_string() { return CodeValStr[val_]; }

  private:
    CodeVal val_;
};
}