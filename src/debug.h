
enum debuglvl { CRITICAL, WARNING, GENERAL, INFO, DEBUG };
// CRITICAL: Critical application failure and has to shut down
// WARNING: A feature isn't working properly, but we got a workaround
// GENERAL: General applicative changes the user should experience
// INFO: Information the user isn't aware of happening in the application
// DEBUG: Useful for debugging programming problems, causes extreme logging

void log(const enum debuglvl lvl, const char *fmt, ...);
void debug_output_message(const char *fmt, ...);

void close_log();
