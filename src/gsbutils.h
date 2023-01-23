#ifndef GSB_UTILS_HPP
#define GSB_UTILS_HPP

// Набор утилит, требующихся мне регулярно ))
#include <cstring>
#include <string>
#include <string_view>
#include <vector>
#include <map>

namespace gsb_utils
{

    // меняем первое вхождение
    std::size_t replace_first(std::string &inout, std::string_view what, std::string_view with)
    {
        std::size_t count = 0;
        std::string::size_type pos{};
        if (inout.npos != (pos = inout.find(what.data(), 0, what.length())))
        {
            inout.replace(pos, what.length(), with.data(), with.length());
            count = 1;
        }
        return count;
    }
    // меняем все вхождения
    std::size_t replace_all(std::string &inout, std::string_view what, std::string_view with)
    {
        std::size_t count{};
        for (std::string::size_type pos{};
             inout.npos != (pos = inout.find(what.data(), pos, what.length()));
             pos += with.length(), ++count)
        {
            inout.replace(pos, what.length(), with.data(), with.length());
        }
        return count;
    }
    // удаляем все вхождения
    std::size_t remove_all(std::string &inout, std::string_view what)
    {
        return replace_all(inout, what, "");
    }
    // удаляем до первого вхождения
    std::string remove_before(std::string inout, std::string_view delimiter)
    {
        std::string result = (inout);
        std::string::size_type pos{};
        pos = result.find(delimiter.data(), 0, delimiter.length());
        if (pos == inout.npos)
            return result;
        result.replace(0, pos + delimiter.length(), "", 0);
        return result;
    }
    // удаляем после первого вхождения
    std::string remove_after(std::string inout, std::string_view delimiter)
    {
        std::string result = (inout);
        std::string::size_type pos{};
        pos = result.find(delimiter.data(), 0, delimiter.length());
        if (pos == inout.npos)
            return result;
        result.replace(pos, result.length() - pos, "");
        return result;
    }

}

#endif
