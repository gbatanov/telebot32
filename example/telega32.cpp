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
#include <gsbutils/gsbutils.h>
#include "../src/tlg32.h"

std::atomic<bool> Flag{true};
std::shared_ptr<Tlg32> tlg32;

// forward declarations
static int cmd_func();
void handle();

std::shared_ptr<gsbutils::Channel<TlgMessage>> tlgIn, tlgOut; // каналы обмена с телеграм
std::thread *tlgInThread;                                     // поток приема команд с телеграм
int64_t MyId = 836487770;

int main(int argc, char *argv[])
{
    gsbutils::init(0, (const char *)"tlg");
    gsbutils::set_debug_level(4);

    INFOLOG("Welcome to Telega32\n");

    // канал приема команд из телеграм
    tlgIn = std::make_shared<gsbutils::Channel<TlgMessage>>(2);
    // канал отправки сообщений в телеграм
    tlgOut = std::make_shared<gsbutils::Channel<TlgMessage>>(2);
    tlg32 = std::make_shared<Tlg32>(BOT_NAME, tlgIn, tlgOut);
    tlg32->add_id(MyId);
    tlgInThread = new std::thread(handle);

    if (!tlg32->run())
        exit(1);
    cmd_func();
    INFOLOG("Stop Telega32\n");
    Flag.store(false);
    tlg32->stop();
    tlgIn->stop();
    tlgOut->stop();
    if (tlgInThread->joinable())
        tlgInThread->join();

    gsbutils::stop();
    return 0;
}

// Здесь реализуется вся логика обработки принятых сообщений
// Указатель на функцию передается в класс Tlg32
void handle()
{
    while (Flag.load())
    {
        TlgMessage msg = tlgIn->read();
        TlgMessage answer{};
        answer.chat.id = msg.from.id;

        DBGLOG("%s: %s \n", msg.from.firstName.c_str(), msg.text.c_str());

        if (msg.text.starts_with("/start"))
            answer.text = "Привет, " + msg.from.firstName;
        else
            answer.text = "Я еще в разработке и не понимаю вас.";

        bool ret = tlg32->send_message(answer.text);

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

    while (Flag.load())
    {
        tv.tv_sec = (long)1;
        tv.tv_usec = (long)0;

        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(0, &readfds);

        int count = select(nfds, &readfds, NULL, NULL, (struct timeval *)&tv);
        if (count > 0)
        {
            if (FD_ISSET(0, &readfds))
            {
                c = getchar();
            }
        }

        switch (c)
        {
        case 'Q':
        case 'q':
            Flag.store(false);
            break;

        case '1':
        {
            TlgMessage msg;
            msg.chat.id = MyId;
            msg.text = "Проверка работы\n";
            if (tlg32->send_message(msg.text))
                DBGLOG("Message sent to queue\n");
        }
        break;
        case '0':
        {
        }
        break;
        } // switch
        c = '\0';
    }

    return 0;
}
