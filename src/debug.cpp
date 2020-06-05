#include <stdio.h>
#include <stdarg.h>
#include <sys/time.h>

#include "debug.h"
#include "filehandling.h"

// global debug variables
const char *debugstr[] = { "CRI", "WRN", "GEN", "NFO", "DBG" };
enum debuglvl dbglvl = GENERAL;
const char *logfile = GAMENAME".log";
FILE *debugfp = NULL;

// set a new debug level
void debug(enum debuglvl lvl) {
  dbglvl = lvl;
  return;
}

// log information to a file
void log(const enum debuglvl msglvl, const char *fmt, ...) {
#ifndef DEBUG_DISABLE
  char str[STRLEN+1];
  va_list ap;
  struct timeval tv;

  // do nothing on an empty message
  if (fmt == NULL) return;

  // open logfile if it isn't already
  if (debugfp == NULL)
    debugfp = f1open(logfile, "w", USERDATA);

  if ( dbglvl >= msglvl ) {
    // format input to string
    va_start(ap, fmt);
    vsprintf(str, fmt, ap);
    va_end(ap);

    // get time of day
    gettimeofday(&tv, NULL);

    // do the logging
    fprintf(debugfp, "%02d:%02d:%02d.%03d %s: %s\n", (int)tv.tv_sec/3600%24, (int)tv.tv_sec/60%60, (int)tv.tv_sec%60, (int)tv.tv_usec/1000, debugstr[msglvl], str);
    fflush(debugfp);
  }
#endif // DEBUG_DISABLE
  return;
}

// backwards compatibility
void debug_output_message(const char *fmt, ...) {
#ifndef DEBUG_DISABLE
  char str[STRLEN+1];
  va_list ap;

  va_start(ap, fmt);
  vsprintf(str, fmt, ap);
  va_end(ap);

  log(DEBUG, "%s", str);
#endif // DEBUG_DISABLE
  return;
}


// close the logfile
void close_log() {
#ifndef DEBUG_DISABLE
  if (debugfp != NULL)
    fclose(debugfp);
  debugfp = NULL;
#endif // DEBUG_DISABLE
  return;
}

