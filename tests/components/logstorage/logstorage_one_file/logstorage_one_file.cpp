#include <stdlib.h>
#include <time.h>
#include "dlt/dlt.h"
#include "dlt/dlt_user_macros.h"

int main(int argc, char *argv[])
{
    int i = 0;
    int c = 0;
    int num_context = 4;
    int max_msg = 200;

    while ((c = getopt(argc, argv, "c:n:")) != -1) {
        switch (c) {
            case 'c':
                num_context = atoi(optarg);
                break;
            case 'n':
                max_msg = atoi(optarg);
                break;
            default:
                break;
        }
    }

    DLT_DECLARE_CONTEXT(ctx[num_context]);

    DLT_REGISTER_APP("LONE", "CT: Logstorage one file");
    for (i = 0; i < num_context; i++) {
        char ctid[DLT_ID_SIZE], ctdesc[255];

        // Ensure DLT_ID_SIZE is enough for "CTxx" (i.e., 4 characters)
        if (DLT_ID_SIZE >= 4) {
            int str_ret = snprintf(ctid, DLT_ID_SIZE, "CT%d", i + 1);
            if (str_ret < 0 || str_ret > DLT_ID_SIZE) {
                fprintf(stderr, "Error: String truncated or snprintf failed.\n");
                return -1;
            }
        } else {
            // Handle error if DLT_ID_SIZE is too small
            fprintf(stderr, "Error: DLT_ID_SIZE is too small to store the context ID\n");
            return EXIT_FAILURE;
        }

        snprintf(ctdesc, 255, "Test Context %02d", i + 1);
        DLT_REGISTER_CONTEXT(ctx[i], ctid, ctdesc);
    }

    for (i = 0; i <= max_msg; i++) {
        for (int j = 0; j < num_context; j++) {
            DLT_LOG(ctx[j], DLT_LOG_INFO, DLT_STRING("Log message"), DLT_UINT32(j + 1), DLT_STRING("#"), DLT_UINT32(i));
        }
        struct timespec tv = {0, 1000000};
        nanosleep(&tv, NULL);
    }

    for (i = 0; i < num_context; i++) {
        DLT_UNREGISTER_CONTEXT(ctx[i]);
    }
    DLT_UNREGISTER_APP_FLUSH_BUFFERED_LOGS();

    return EXIT_SUCCESS;
}