#include <syslog.h>
#include <string>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <map>
#include <atomic>
#include <mutex>
#include <thread>
#include <queue>
#include <condition_variable>
#include <curl/curl.h>

#include "version.h"

#ifdef USE_UTILS
#include "gsbutils.h"
#else
#include "../../gsb_utils/gsbutils.h"
using gsb_utils = gsbutils::SString;
#endif
#include "tlg32.h"

#define BOT_API_URL "https://api.telegram.org"
#define BOT_URL_SIZE 1024

// из cUrl приходит в contents, сохраняем ответ в пользовательском буфере userp
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}
// Имя бота необходимо для правильного выбора токена в системе с несколькими ботами
Tlg32::Tlg32(std::string botName) : botName_(botName)
{
    bot_.isBot = true;
    flag.store(true);
}
Tlg32::~Tlg32()
{
    flag.store(false);
    qcv.notify_one();
    if (pollThread_.joinable())
        pollThread_.join();
    if (sendThread_.joinable())
        sendThread_.join();
}

bool Tlg32::run(handleFunc handle)
{
    handle_ = handle;
    token_ = get_token();
    if (token_.empty())
        throw std::runtime_error("Token is empty\n");

    if (!get_me())
    {
        // TODO: рассмотреть варианты ошибок, допускающие перезапуск
        flag.store(false);
        return false;
    }
    DBGLOG("Bot Id: %llu, Bot User Name: %s\n", (unsigned long long)bot_id(), bot_name().c_str());

    pollThread_ = std::thread(&Tlg32::poll, this);
    sendThread_ = std::thread(&Tlg32::send_message_real, this);
    return true;
}

bool Tlg32::query_to_api(std::string method, std::string *response, Tlg32_core_mime mimes[], uint8_t mimeSize)
{
    if (!flag.load())
        return false;
    std::lock_guard<std::mutex> lg(apiMtx_);

    CURL *curl;
    CURLcode res;
    readBuffer_.clear();
    curl_mime *mime = NULL;
    std::string apiUrl = BOT_API_URL;
    bool ret = true;
    bool curl_inited = false;

    try
    {
        apiUrl = apiUrl + "/bot" + token_ + "/" + method;

        //        INFOLOG("%s \n", apiUrl.c_str());

        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();
        if (curl && flag.load())
        {
            curl_inited = true;

            curl_easy_setopt(curl, CURLOPT_URL, apiUrl.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer_);

            if (mimeSize > 0)
            {
                mime = curl_mime_init(curl);
                if (mime == NULL)
                    throw std::runtime_error("Failed to create mime");

                for (unsigned int index = 0; index < mimeSize; index++)
                {
                    curl_mimepart *part = curl_mime_addpart(mime);
                    if (part == NULL)
                        throw std::runtime_error("Failed to create mime part");

                    curl_mime_name(part, mimes[index].name);
                    curl_mime_data(part, mimes[index].data, CURL_ZERO_TERMINATED);
                }

                curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
            }
            if (!flag.load())
                return false;

            res = curl_easy_perform(curl); // CURLE_OK
            if (res == CURLE_OK && flag.load())
            {
                long resp_code = 0;
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &resp_code);
                if (resp_code == 200L)
                {
                    *response = readBuffer_;
                    ret = true;
                    goto finish;
                }
                else
                {
                    ERRLOG("Код ответа: %ld \n", resp_code);
                    ret = false;
                    goto finish;
                }
            }
            else
            {
                ERRLOG("curl not ok\n");
                ret = false;
                goto finish;
            }
            ret = true;
        }
    }
    catch (std::exception &e)
    {
        std::cout << e.what() << std::endl;
        ret = false;
    }

finish:
    if (flag.load())
    {
        if (mime)
            curl_mime_free(mime);
        if (curl_inited)
            curl_easy_cleanup(curl);
    }
    return ret;
}

// Получаем данные бота
bool Tlg32::get_me()
{
    std::string content{};
    if (!query_to_api("getMe", &content, NULL, 0))
        return false;
    try
    {
        return parseMe(content);
    }
    catch (std::exception &e)
    {
        ERRLOG("%s\n", e.what());
    }
    return true;
}

bool Tlg32::get_updates(std::vector<Message> *msgIn)
{
    std::string content{};
    User from;

    int64_t offset = (int64_t)lastUpdateId_ + 1;
    uint8_t limit = 20;  // максимум 20 записей
    uint8_t timeout = 5; // тайм-аут -  5 секунд

    int index = 0;
    Tlg32_core_mime mimes[4]; // number of arguments
    mimes[index].name = "offset";
    snprintf(mimes[index].data, sizeof(mimes[index].data), "%lld", (long long int)offset);
    ++index;

    mimes[index].name = "limit";
    snprintf(mimes[index].data, sizeof(mimes[index].data), "%d", limit);
    ++index;

    mimes[index].name = "timeout";
    snprintf(mimes[index].data, sizeof(mimes[index].data), "%d", timeout);
    ++index;

    mimes[index].name = "allowed_updates";
    snprintf(mimes[index].data, sizeof(mimes[index].data), "%s", "message");
    ++index;

    if (!query_to_api("getUpdates", &content, mimes, index))
        return false;

    uint64_t tmpUpdateId = lastUpdateId_; // сохраняю последний обработанный ID сообщения

    try
    {
        //        DBGLOG("getUpdates body: %s \n", content.c_str());
        return parseUpdates(content, msgIn);
    }
    catch (std::exception &e)
    {
        ERRLOG("%s \n", e.what());
        msgIn->clear();
        lastUpdateId_ = tmpUpdateId; // восстанавливаю последний обработанный ID сообщения
        return false;
    }
    return true;
}

// Ставим сообщение в очередь
bool Tlg32::send_message(Message msg)
{
    std::lock_guard<std::mutex> lg(queueMtx_);
    msgQueue_.push(msg);
    qcv.notify_one();
    return true;
}
// Сообщения из модулей программы, отправляются по списку валидных ID
bool Tlg32::send_message(std::string txt)
{
    if (txt.empty())
        return false;
    if (txt.size() > 2048)
        return false;
    for (auto &vid : valid_ids_)
    {
        Message msg;
        msg.chat.id = vid;
        msg.text = txt;
        std::lock_guard<std::mutex> lg(queueMtx_);
        msgQueue_.push(msg);
    }
    qcv.notify_one();
    return true;
}
bool Tlg32::send_message_real()
{
    while (flag.load())
    {
        std::unique_lock<std::mutex> ul(qcvMtx_);
        qcv.wait(ul, [this]()
                 { return (this->msgQueue_.size() > 0) || !flag.load(); });
        if (this->msgQueue_.size() > 0)
        {
            Message msg = this->msgQueue_.front();

            std::string content{};
            int index = 0;
            Tlg32_core_mime mimes[2];
            mimes[index].name = "chat_id";
            snprintf(mimes[index].data, sizeof(mimes[index].data), "%lld", (long long int)msg.chat.id);
            ++index;
            mimes[index].name = "text";
            snprintf(mimes[index].data, sizeof(mimes[index].data), "%s", msg.text.c_str());
            ++index;

            bool res = query_to_api("sendMessage", &content, mimes, index);
            // Сообщение удаляем только в случае его удачной отправки
            if (res)
                this->msgQueue_.pop();
        }
        ul.unlock();
    }

    return true;
}
std::string Tlg32::get_token()
{
    char c_token[1024]{0};
    snprintf(c_token, 1024, "/usr/local/etc/telebot32/.token%s/.token", botName_.c_str());
    FILE *fp = fopen(c_token, "r");
    if (fp == NULL)
    {
        ERRLOG("Failed to open .token file\n");
        return "";
    }

    if (fscanf(fp, "%s", c_token) == 0)
    {
        ERRLOG("Failed to read token\n");
        fclose(fp);
        return "";
    }

    fclose(fp);
    std::string token(c_token);
    return token;
}
// функция опроса сервера телеграма
void Tlg32::poll()
{
    try
    {
        while (flag.load())
        {
            std::vector<Message> msgIn{};

            if (flag.load() && get_updates(&msgIn))
                if (msgIn.size())
                    for (Message msg : msgIn)
                        handle_(msg);
            if (flag.load())
                std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }
    catch (std::exception &e)
    {
        ERRLOG("%s \n", e.what());
    }
}

// парсинг поля result в ответе getMe
bool Tlg32::parseMe(std::string content)
{
    if (!content.starts_with("{\"ok\":true,\"result\""))
        return false;
    gsb_utils::replace_first(content, "{\"ok\":true,\"result\":{", "");
    gsb_utils::remove_all(content, "}}");
    int countParams = 3; // мне нужны только три параметра из этого ответа

    while (flag.load() && countParams > 0)
    {
        std::string para = gsb_utils::remove_after(content, ","); // очередной параметр "key":value или "key":"value"
        content = gsb_utils::remove_before(content, ",");         // оставшаяся часть строки ответа
        if (para.starts_with("\"id\""))
        {
            countParams--;
            para = gsb_utils::remove_before(para, ":");
            bot_.id = std::stoull(para.c_str());
        }
        else if (para.starts_with("\"first_name\""))
        {
            countParams--;
            para = gsb_utils::remove_before(para, ":");
            bot_.firstName = para;
        }
        else if (para.starts_with("\"username\""))
        {
            countParams--;
            para = gsb_utils::remove_before(para, ":");
            bot_.lastName = para;
        }
    }
    return true;
}
// парсинг поля result в ответе getUpdates
bool Tlg32::parseUpdates(std::string content, std::vector<Message> *msgIn)
{
    if (!content.starts_with("{\"ok\":true,\"result\""))
        return false;
    if (content == "{\"ok\":true,\"result\":[]}")
        return true;
    gsb_utils::replace_first(content, "{\"ok\":true,\"result\":[", "");
    gsb_utils::remove_all(content, "]}");
    //   DBGLOG("%s \n", content.c_str());
    std::string::size_type n;
    std::vector<std::string::size_type> positions{};
    n = content.find("{\"update_id");
    while (n != std::string::npos)
    {
        //       DBGLOG("%lu \n", n);
        positions.push_back(n);
        n = content.find("{\"update_id", n + strlen("{\"update_id"));
    }
    int msgCount = positions.size();
    //    DBGLOG("Messages count %d \n", msgCount);
    if (msgCount == 1)
    {
        return parseOneUpdate(content, msgIn);
    }
    else
    {
        for (uint8_t i = 0; i < msgCount - 1; i++)
        {
            std::string update = content.substr(positions[i], positions[i + 1] - positions[i]);
            parseOneUpdate(update, msgIn);
        }
        std::string update = content.substr(positions[msgCount - 1]);
        return parseOneUpdate(update, msgIn);
    }

    return false;
}

bool Tlg32::parseOneUpdate(std::string content, std::vector<Message> *msgIn)
{
    Message msg{};
    //   DBGLOG("\n %s \n", content.c_str());
    std::string para = gsb_utils::remove_after(content, ","); // "update_id":ID
    content = gsb_utils::remove_before(content, ",");         // оставшаяся часть строки ответа - message

    para = gsb_utils::remove_before(para, ":");
    uint64_t lastUpdateId = (uint64_t)std::stoull(para.c_str());
    if (lastUpdateId > lastUpdateId_)
        lastUpdateId_ = lastUpdateId;

    content = gsb_utils::remove_before(content, "\"message\":{\"message_id\":");
    para = gsb_utils::remove_after(content, ",");
    msg.messageId = (uint64_t)std::stoull(para.c_str());

    content = gsb_utils::remove_before(content, "\"from\":{"); // начинается с from "id":836487770,"is_bot":false,"first_name":"Georgii","last_name":"Batanov","language_code":"ru"},"chat":{"id":836487770,"first_name":"Georgii","last_name":"Batanov","type":"private"},"date":1672082654,"text":"test9"}}
    para = gsb_utils::remove_before(content, "date\":");       // 1672082654,"text":"test9"}}  // начинается с date
    std::string txt = gsb_utils::remove_before(para, "\"text\":");
    gsb_utils::remove_all(txt, "\"}}");
    txt = gsb_utils::remove_after(txt, ",\"entities");
    txt = gsb_utils::remove_before(txt, "\"");
    txt = gsb_utils::remove_after(txt, "\"");
    DBGLOG("%s \n", txt.c_str());
    msg.text = txt;

    para = gsb_utils::remove_after(para, ",");
    msg.date = (uint64_t)std::stoull(para.c_str());

    content = gsb_utils::remove_after(content, "},\"chat");

    //   DBGLOG("\n %s \n", para.c_str());
    //    DBGLOG("\n %s \n", content.c_str());
    int countParams = 3; // мне нужны только три параметра из этого ответа

    while (flag.load() && countParams > 0)
    {
        std::string para = gsb_utils::remove_after(content, ","); // очередной параметр "key":value или "key":"value"
        content = gsb_utils::remove_before(content, ",");         // оставшаяся часть строки ответа
        if (para.starts_with("\"id\""))
        {
            countParams--;
            para = gsb_utils::remove_before(para, ":");
            msg.from.id = std::stoull(para.c_str());
        }
        else if (para.starts_with("\"first_name\""))
        {
            countParams--;
            para = gsb_utils::remove_before(para, ":\"");
            para = gsb_utils::remove_after(para, "\"");
            msg.from.firstName = para;
        }
        else if (para.starts_with("\"last_name\""))
        {
            countParams--;
            para = gsb_utils::remove_before(para, ":\"");
            para = gsb_utils::remove_after(para, "\"");
            msg.from.lastName = para;
        }
    }
    msgIn->push_back(msg);
    return true;
}

bool Tlg32::client_valid(unsigned long long id)
{
    for (auto &elem : valid_ids_)
    {
        if (id == elem)
            return true;
    }
    return false;
}