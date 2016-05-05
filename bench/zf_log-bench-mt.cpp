#include <thread>
#include <vector>
#include <atomic>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <zf_log.c>

const char g_path[] = "logs/zf_log.txt";
int g_fd;

static void output_callback(zf_log_message *msg)
{
    *msg->p = '\n';
    write(g_fd, msg->buf, msg->p - msg->buf + 1);
}

using namespace std;

int main(int argc, char* argv[])
{
    g_fd = open(g_path, O_APPEND|O_CREAT|O_WRONLY);
    if (0 > g_fd)
    {
        ZF_LOGE_AUX(ZF_LOG_STDERR, "Failed to open log file: %s", g_path);
        return -1;
    }
    zf_log_set_output_callback(ZF_LOG_PUT_STD, output_callback);

    int thread_count = 10;
    if(argc > 1)
        thread_count = std::atoi(argv[1]);
    int howmany = 1000000;
    std::atomic<int > msg_counter {0};
    vector<thread> threads;

    for (int t = 0; t < thread_count; ++t)
    {
        threads.push_back(std::thread([&]()
        {
            while (true)
            {
                int counter = ++msg_counter;
                if (counter > howmany) break;
                ZF_LOGI("zf_log message #%i: This is some text for your pleasure", counter);
            }
        }));
    }

    for (auto &t:threads)
    {
        t.join();
    };
    close(g_fd);
    return 0;
}
