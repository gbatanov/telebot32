#ifndef GSB_TLG32_H
#define GSB_TLG32_H

#include <queue>
#include <condition_variable>

typedef struct Tlg32_core_mime_
{
    const char *name;
    char data[4096];
} Tlg32_core_mime;

typedef enum Tlg32_update_type_
{
    TLG32_UPDATE_TYPE_MESSAGE = 0,         /**< Message */
    TLG32_UPDATE_TYPE_EDITED_MESSAGE,      /**< Edited message */
    TLG32_UPDATE_TYPE_CHANNEL_POST,        /**< Channel post */
    TLG32_UPDATE_TYPE_EDITED_CHANNEL_POST, /**< Edited channel post */
} Tlg32_update_type;

typedef struct User_
{
    uint64_t id;
    bool isBot;
    std::string firstName;
    std::string lastName;
    std::string languageCode;
} User;

typedef struct Chat_
{
    uint64_t id;
    std::string firstName;
    std::string lastName;
    std::string type;
} Chat;

typedef struct Message_
{
    uint64_t messageId;
    User from;
    Chat chat;
    unsigned long long date;
    std::string text;
} Message;

typedef void (*handleFunc)(Message);

//------------------------- Tlg32 ----------------------

class Tlg32
{
public:
    Tlg32(std::string botName);
    ~Tlg32();

    std::string get_token();
    bool run(handleFunc handle);
    bool query_to_api(std::string method, std::string *response, Tlg32_core_mime mimes[], uint8_t mime_size);
    bool get_me();
    bool get_updates(std::vector<Message> *msgIn);
    bool send_message(Message msg);
    bool send_message(std::string txt);
    uint64_t bot_id() { return bot_.id; }
    std::string bot_name() { return bot_.lastName; }
    bool add_id(unsigned long long Id)
    {
        valid_ids_.push_back(Id);
        return true;
    }
    bool client_valid(unsigned long long id);

    std::atomic<bool> flag;

private:
    std::string botName_{};
    std::string token_{};
    std::string readBuffer_{};
    bool send_message_real();
    void poll();
    bool parseMe(std::string content);
    bool parseUpdates(std::string content, std::vector<Message> *msgIn);
    bool parseOneUpdate(std::string content, std::vector<Message> *msgIn);

    User bot_{};
    handleFunc handle_;
    uint64_t lastUpdateId_ = 0; // последний полученный update_id, следующий запрос на update должен быть больше этого числа
    std::mutex apiMtx_;
    std::thread pollThread_, sendThread_;
    std::queue<Message> msgQueue_{};
    std::mutex queueMtx_, qcvMtx_;
    std::condition_variable qcv;
    std::vector<unsigned long long> valid_ids_{}; // Список валидных ID клиентов
};


#ifdef USE_UTILS
#ifdef DEBUG
#define ERRLOG(fmt, args...) fprintf(stderr, "[ERROR][%s:%d]" fmt, __func__, __LINE__, ##args)
#define DBGLOG(fmt, args...) fprintf(stdout, "[DEBUG][%s:%d]" fmt, __func__, __LINE__, ##args)
#define INFOLOG(fmt, args...) fprintf(stdout, "[Info]" fmt, ##args)
#else
#define ERRLOG(fmt, args...) syslog(1, fmt, ##args)
#define INFOLOG(fmt, args...) fprintf(stdout, "[Info]" fmt, ##args)
#define DBGLOG(x, ...)
#endif
#endif

#endif