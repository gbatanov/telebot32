// telega32

#include <thread>
#include <string>
#include <atomic>
#include <unistd.h>
#include <syslog.h>
#include <mutex>
#include <vector>

#include "../src/version.h"
#include "botname.h"
#include "../src/tlg32.h"

std::atomic<bool> flag{true};
std::unique_ptr<Tlg32> tlg32;

// forward declarations
static int cmd_func();
void handle(Message msg);

int main(int argc, char *argv[])
{
    openlog("tlg32", LOG_PID, LOG_LOCAL7); //(local7.log Linux, system.log Mac OS)

    INFOLOG("Welcome to Telega32\n");
    tlg32 = std::make_unique<Tlg32>(BOT_NAME);
    if (!tlg32->run(&handle))
        exit(1);
    cmd_func();
    flag.store(false);
    return 0;
}

// Здесь реализуется вся логика обработки принятых сообщений
// Указатель на функцию передается в класс Tlg32
void handle(Message msg)
{
    if (flag.load() && !msg.text.empty())
    {
        Message answer{};
        answer.chat.id = msg.from.id;

        DBGLOG("%s: %s \n", msg.from.firstName.c_str(), msg.text.c_str());

        if (msg.text.starts_with("/start"))
            answer.text = "Привет, " + msg.from.firstName;
        else
            answer.text = "Я еще в разработке и не понимаю вас.";

        bool ret = tlg32->send_message(answer);

        if (!ret)
            ERRLOG("Failed to send message \n");
    }
}

/**
Обработчик команд с клавиатуры.
*/
static int cmd_func()
{

    int nfds = 1; // Количество файловых дескрипторов, наблюдаемых в select
    char c;
    struct timeval tv;

    while (flag.load())
    {
        tv.tv_sec = (long)1;
        tv.tv_usec = (long)0;

        time_t start = time(NULL);
        time_t waitTime = 1;

        while (flag.load() && (time(NULL) < start + waitTime))
        {
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(0, &readfds);

            int count = select(nfds, &readfds, NULL, NULL, (struct timeval *)&tv);
            if (count > 0)
            {
                if (FD_ISSET(0, &readfds))
                {
                    c = getchar();
                    break;
                }
            }
        }

        switch (c)
        {
        case 'Q':
        case 'q':
            flag.store(false);
            break;

        case '1':
        {
            Message msg;
            msg.chat.id = 836487770;
            msg.text = "Проверка вшивости\n";
            if (tlg32->send_message(msg))
                DBGLOG("Message sent to queue\n");
        }
        break;
        case '0':
        {
        }
        break;
        }
        c = '\0';
    }

    return 0;
}
