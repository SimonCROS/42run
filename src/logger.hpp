#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>

#define FTRUN_SHADER "SHADER"
#define FTRUN_SHADER_PROGRAM "SHADER_PROGRAM"

#define FTRUN_FILE_NOT_SUCCESSFULLY_READ "FILE_NOT_SUCCESSFULLY_READ"
#define FTRUN_COMPILATION_FAILED "COMPILATION_FAILED"

#define logError(subject, error) (std::cerr << "ERROR::" << subject << "::" << error << std::endl)
#define logError2(subject, error, additional) (std::cerr << "ERROR::" << subject << "::" << error << ": " << additional << std::endl)

#endif
