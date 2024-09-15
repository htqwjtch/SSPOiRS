#include "exceptionservice.h"

ExceptionService::ExceptionService(const std::string& exceptionInfo) noexcept {
    this->exceptionInfo = exceptionInfo;
};

const char *ExceptionService::what() const noexcept {
    return exceptionInfo.c_str();
}