#include <stdio.h>
#include <zf_log.c>

const char g_path[] = "logs/zf_log.txt";
static FILE *g_f;

static void output_callback(zf_log_message *msg)
{
    *msg->p = '\n';
    fwrite(msg->buf, msg->p - msg->buf + 1, 1, g_f);
}

int main(int, char* [])
{
    g_f = fopen(g_path, "wb");
    if (!g_f)
    {
        ZF_LOGE_AUX(ZF_LOG_STDERR, "Failed to open log file: %s", g_path);
        return -1;
    }
    zf_log_set_output_callback(ZF_LOG_PUT_STD, output_callback);

    const int howmany = 1000000;
    for(int i  = 0 ; i < howmany; ++i)
        ZF_LOGI("zf_log message #%i: This is some text for your pleasure", i);
    fclose(g_f);
    return 0;
}
