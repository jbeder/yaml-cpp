
#include "yaml-cpp/dll.h"
#include "yaml-cpp/eventhandler.h"

#include <string>

namespace YAML {
struct Mark;

/**
 * TagResolver is a base class for tag resolution.
 * It acts as an event filter: it is an EventHandler itself,
 * and it passes events on to a client EventHandler
 * while resolving tags in the process.

 * This base class passes events unmodified.
 * A subclass is supposed to implement tag resolution
 * according to a particular schema,
 * as recommended by the YAML 1.2 spec:
 * http://yaml.org/spec/1.2/spec.html#Schema

 * Note that each tag in a schema also needs to provide
 * a canonical representation of its values.
 * Mapping values into canonical representations
 * is not the responsibility of TagResolver though.
 */

class YAML_CPP_API TagResolver : public EventHandler
{
public:
    using string = std::string;

    TagResolver(EventHandler & client): m_client(client) {}

    virtual void OnDocumentStart(const Mark& mark)
    {
        m_client.OnDocumentStart(mark);
    }

    virtual void OnDocumentEnd()
    {
        m_client.OnDocumentEnd();
    }

    virtual void OnNull(const Mark& mark, anchor_t anchor)
    {
        m_client.OnNull(mark, anchor);
    }

    virtual void OnAlias(const Mark& mark, anchor_t anchor)
    {
        m_client.OnAlias(mark, anchor);
    }

    virtual void OnScalar(const Mark& mark, const std::string& tag,
                          anchor_t anchor, const std::string& value)
    {
        m_client.OnScalar(mark, tag, anchor, value);
    }

    virtual void OnSequenceStart(const Mark& mark, const std::string& tag,
                                 anchor_t anchor, EmitterStyle::value style)
    {
        m_client.OnSequenceStart(mark, tag, anchor, style);
    }

    virtual void OnSequenceEnd()
    {
        m_client.OnSequenceEnd();
    }

    virtual void OnMapStart(const Mark& mark, const std::string& tag,
                            anchor_t anchor, EmitterStyle::value style)
    {
        m_client.OnMapStart(mark, tag, anchor, style);
    }

    virtual void OnMapEnd()
    {
        m_client.OnMapEnd();
    }

    EventHandler & m_client;
};

/**
 * Implements tag resolution according to Core Schema,
 * as defined by YAML 1.2 spec:
 * http://yaml.org/spec/1.2/spec.html#id2804923
 */

class YAML_CPP_API CoreTagResolver : public TagResolver
{
private:

    static bool IsBase10Digit(char c)
    {
        return c >= '0' and c <= '9';
    }

    static bool IsBase8Digit(char c)
    {
        return c >= '0' and c <= '7';
    }

    static bool IsBase16Digit(char c)
    {
        return IsBase10Digit(c) || (c >= 'a' and c <= 'f') || (c >= 'A' and c <= 'F');
    }

    static bool IsBase8Int(const string & text);
    static bool IsBase16Int(const string & text);
    static bool isBase10Int(const string & text);

public:
    CoreTagResolver(EventHandler & client): TagResolver(client) {}

    static bool TagIsNonSpecific(const string & tag)
    {
        return (tag == "?" || tag == "!");
    }


    static bool ScalarIsNull(const string & text)
    {
        return (text == "null" || text == "Null" || text == "NULL" || text == "~");
    }

    static bool ScalarIsBool(const string & text)
    {
        return text == "true" ||
                text == "True" ||
                text == "TRUE" ||
                text == "false" ||
                text == "False" ||
                text == "FALSE";
    }

    static bool ScalarIsInt(const string & text);

    static bool ScalarIsFloat(const string & text);

    virtual void OnScalar(const Mark& mark, const std::string& tag,
                          anchor_t anchor, const std::string& value);
    virtual void OnSequenceStart(const Mark& mark, const std::string& tag,
                                 anchor_t anchor, EmitterStyle::value style);
    virtual void OnMapStart(const Mark& mark, const std::string& tag,
                            anchor_t anchor, EmitterStyle::value style);
};

}
