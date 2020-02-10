
#include <spdlog/spdlog.h>
#include <spdlog/sinks/base_sink.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
using namespace std;
template<typename Mutex>
class tcp_sink : public spdlog::sinks::base_sink <Mutex>
{
public:
    tcp_sink(std::string address,int port)
    {
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            SPDLOG_THROW(spdlog_ex("Socket creation error", errno));
        }
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);
        if(inet_pton(AF_INET, address.c_str(), &serv_addr.sin_addr)<=0)
        {
            SPDLOG_THROW(spdlog_ex("Invalid address/ Address not supported", errno));
        }
        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            SPDLOG_THROW(spdlog_ex("Connection Failed", errno));
        }

    }
protected:
    void sink_it_(const spdlog::details::log_msg& msg) override
    {
        spdlog::memory_buf_t formatted;
        spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
        send(sock , formatted.data() , formatted.size() , 0 );
    }

    void flush_() override
    {
    }
private:
    int sock;
    struct sockaddr_in serv_addr;
};
using tcp_sink_mt = tcp_sink<std::mutex>;
using tcp_sink_st = tcp_sink<spdlog::details::null_mutex>;
