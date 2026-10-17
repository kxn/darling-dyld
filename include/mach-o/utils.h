#ifndef DARLING_MACH_O_UTILS_H
#define DARLING_MACH_O_UTILS_H
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
#ifdef __BLOCKS__
void macho_for_each_runnable_arch_name(void (^callback)(const char *archName, bool *stop));
#endif
#ifdef __cplusplus
}
#endif
#endif
