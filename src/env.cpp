#include "env.h"

#ifndef DISABLE_LOGGING
static plog::ColorConsoleAppender<plog::TxtFormatter> console_appender;
static int logg = (initLogger(), 0);
#endif

void initLogger()
{
#ifdef LOG_LEVEL_DEBUG
    plog::init(plog::debug);
#else
    plog::init(plog::info);
#endif

    plog::get()->addAppender(&console_appender);
}