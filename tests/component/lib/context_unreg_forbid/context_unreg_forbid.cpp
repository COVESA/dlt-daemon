#include <cstdlib>
#include <stdlib.h>
#include <time.h>
#include "dlt/dlt.h"
#include "dlt/dlt_user_macros.h"

int main()
{
    DLT_DECLARE_CONTEXT(ctx);

    DLT_REGISTER_APP("LOG", "Context unregister forbid");
    DLT_REGISTER_CONTEXT(ctx, "TEST", "Context unregister forbid");

    DLT_LOG(ctx, DLT_LOG_INFO, DLT_STRING("Log message"));
    struct timespec tv = {0, 1000000};
    nanosleep(&tv, NULL);

    DLT_UNREGISTER_APP_FLUSH_BUFFERED_LOGS();
    nanosleep(&tv, NULL);

    DLT_UNREGISTER_CONTEXT(ctx);

    return EXIT_SUCCESS;
}
