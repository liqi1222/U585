#ifndef U585_LOG_H
#define U585_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void U585_Log_WriteString(const char *text);
void U585_Log_WriteLine(const char *text);
void U585_Log_WriteU32(const char *prefix, uint32_t value);

#ifdef __cplusplus
}
#endif

#endif /* U585_LOG_H */
