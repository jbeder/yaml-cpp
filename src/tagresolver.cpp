#include "../include/yaml-cpp/tagresolver.h"

#include <vector>
#include <algorithm>

namespace YAML {

bool CoreTagResolver::IsBase8Int(const string & text)
{
    if (text.empty())
        return false;
    for (string::size_type i = 0; i < text.size(); ++i)
    {
        if (!IsBase8Digit(text[i]))
            return false;
    }
    return true;
}

bool CoreTagResolver::IsBase16Int(const string & text)
{
    if (text.empty())
        return false;
    for (string::size_type i = 0; i < text.size(); ++i)
    {
        if (!IsBase16Digit(text[i]))
            return false;
    }
    return true;
}

bool CoreTagResolver::isBase10Int(const string & text)
{
    using namespace std;

    if (text.empty())
        return false;
    string::size_type pos = 0;
    if ( text[pos] == '-' ||
         text[pos] == '+' )
        pos += 1;
    if (pos == text.size())
        return false;
    for (; pos < text.size(); ++pos)
    {
        if (!IsBase10Digit(text[pos]))
            return false;
    }
    return true;
}

bool CoreTagResolver::ScalarIsInt(const string & text)
{
    if (text.substr(0,2) == "0o")
    {
        return IsBase8Int(text.substr(2));
    }
    else if (text.substr(0,2) == "0x")
    {
        return IsBase16Int(text.substr(2));
    }
    else
    {
        return isBase10Int(text);
    }
}

bool CoreTagResolver::ScalarIsFloat(const string & text)
{
    static std::vector<string> inf = { ".inf", ".Inf", ".INF" };
    static std::vector<string> nan = { ".nan", ".NaN", ".NAN" };

    if (text.empty())
        return false;

    if (std::find(nan.begin(), nan.end(), text) != nan.end())
        return true;

    string::size_type pos = 0;

    if ( text[0] == '-' ||
         text[0] == '+' )
    {
        pos += 1;
    }

    if (pos == text.size())
        return false;

    if (std::find(inf.begin(), inf.end(), text.substr(pos)) != inf.end())
        return true;

    string::size_type intStart = pos;
    while(pos < text.size() && IsBase10Digit(text[pos]))
    {
        ++pos;
    }
    bool hasInt = pos > intStart;

    if (pos == text.size())
        return false;

    if (text[pos] != '.')
        return false;
    ++pos;

    string::size_type fracStart = pos;
    while(pos < text.size() && IsBase10Digit(text[pos]))
    {
        ++pos;
    }
    bool hasFrac = pos > fracStart;

    if (!hasInt && !hasFrac)
        return false;

    if (pos == text.size())
        return true;

    if (text[pos] != 'e' && text[pos] != 'E')
        return false;
    ++pos;

    if ( text[pos] == '-' || text[pos] == '+' )
        ++pos;

    string::size_type expStart = pos;
    while(pos < text.size() && IsBase10Digit(text[pos]))
    {
        ++pos;
    }
    if (pos == expStart)
        return false;
    if (pos != text.size())
        return false;
    return true;
}

void CoreTagResolver::OnScalar(const Mark& mark, const std::string& tag,
                      anchor_t anchor, const std::string& value)
{
    if (tag == "!")
    {
        TagResolver::OnScalar(mark, "tag:yaml.org,2002:str", anchor, value);
    }
    else if (tag == "?")
    {
        string resolved_tag;

        if (ScalarIsNull(value))
            resolved_tag = "tag:yaml.org,2002:null";
        else if (ScalarIsBool(value))
            resolved_tag = "tag:yaml.org,2002:bool";
        else if (ScalarIsInt(value))
            resolved_tag = "tag:yaml.org,2002:int";
        else if (ScalarIsFloat(value))
            resolved_tag = "tag:yaml.org,2002:float";
        else
            resolved_tag = "tag:yaml.org,2002:str";

        TagResolver::OnScalar(mark, resolved_tag, anchor, value);
    }
    else
    {
        TagResolver::OnScalar(mark, tag, anchor, value);
    }
}

void CoreTagResolver::OnSequenceStart(const Mark& mark, const std::string& tag,
                             anchor_t anchor, EmitterStyle::value style)
{
    if (TagIsNonSpecific(tag))
        TagResolver::OnSequenceStart(mark, "tag:yaml.org,2002:seq", anchor, style);
    else
        TagResolver::OnSequenceStart(mark, tag, anchor, style);
}

void CoreTagResolver::OnMapStart(const Mark& mark, const std::string& tag,
                        anchor_t anchor, EmitterStyle::value style)
{
    if (TagIsNonSpecific(tag))
        TagResolver::OnMapStart(mark, "tag:yaml.org,2002:map", anchor, style);
    else
        TagResolver::OnMapStart(mark, tag, anchor, style);
}

}
