#include <cstdlib>
#include <stdlib.h>
#include <time.h>
#include "dlt/dlt.h"
#include "dlt/dlt_user_macros.h"

int main()
{
    int i = 0;
    int num_context = 4;
    int max_msg = 20;

    DLT_DECLARE_CONTEXT(ctx[num_context]);

    DLT_REGISTER_APP("LMAX", "CT: Logstorage max cache size");
    for(i = 0; i < num_context; i++) {
        char ctid[DLT_ID_SIZE + 1], ctdesc[255];
        snprintf(ctid, DLT_ID_SIZE + 1, "CT%02d", i + 1);
        snprintf(ctdesc, 255, "Test Context %02d", i + 1);
        DLT_REGISTER_CONTEXT(ctx[i], ctid, ctdesc);
    }

    for (i = 0; i <= max_msg; i++) {
        for (int j = 0; j < num_context; j++) {
            DLT_LOG(ctx[j], DLT_LOG_INFO, DLT_STRING("Max Cache Size: Log message"), DLT_UINT32(j + 1), DLT_STRING("#"));
        }
        struct timespec tv = {0, 1000000};
        nanosleep(&tv, NULL);
    }

    for (i = 0; i < num_context; i++) {
        DLT_UNREGISTER_CONTEXT(ctx[i]);
    }
    DLT_UNREGISTER_APP_FLUSH_BUFFERED_LOGS();

    sleep(3);

    return EXIT_SUCCESS;
}
