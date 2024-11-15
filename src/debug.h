#ifndef DEBUG_H
#define DEBUG_H

#define DEBUG 0

#if DEBUG
#define DEBUG_ONLY(x) x
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)
#define DEBUG_PRINTLN(x) Serial.println(x)

#else
#define DEBUG_ONLY(x)
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINTF(...)
#endif

#endif // DEBUG_H