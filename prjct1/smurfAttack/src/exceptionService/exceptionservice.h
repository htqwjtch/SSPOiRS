#pragma once

#include <iostream>
#include <exception>

class ExceptionService : public std::exception
{
    std::string exceptionInfo = "\nSomething went wrong\n";

public:
    ExceptionService(const std::string&) noexcept;
    const char *what() const noexcept override;
};