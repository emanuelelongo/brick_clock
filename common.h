#define DEBUG

#ifdef DEBUG
 #define DEBUG_PRINT(x) Serial.print(x);
 #define DEBUG_PRINTLN(x) Serial.println(x);
 #define DEBUG_PRINTF(x, ...) Serial.printf(x, __VA_ARGS__);
#else
 #define DEBUG_PRINT(x, ...)
 #define DEBUG_PRINTLN(x, ...)
 #define DEBUG_PRINTF(x, ...)
#endif