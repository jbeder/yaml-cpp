### Note: this page is no longer relevant since yaml-cpp now includes an emitter. I'm leaving it here for future reference, though, but it will no longer be updated. ###

# Introduction #

Currently, yaml-cpp is just a YAML parser with an emitter that simply parrots back a pre-parsed YAML document. It would be nice to be able to generate YAML documents (and then write them), but I'm not convinced what an ideal API would look like for this.

The purpose of this document is to compile a list of possible ways to generate YAML documents. Please leave comments critiquing the solutions, or providing your own. The idea (at the moment) is **not** to write the actual behind-the-scenes code, but rather to describe an API, and so to write samples of what a user of this API would write.

I'll be updating this wiki based on the comments that people leave. Also, if you like, indicate whether or not such an API would be useful to you.

# Copy/Assignment Method #

This method uses the copy constructors, assignment operators, and STL-like member functions for generating the content, and then the (existing) operator << to write it.

```
// basic scalars
YAML::Node scalar = "Hello World";
YAML::Node number = 52;

// sequence
YAML::Node sequence;
sequence.push_back(scalar);
sequence.push_back(number);

// map
YAML::Node price;
price["monitor"] = 250;
price["RAM (1GB)"] = 70;
price["some random sequence"] = sequence;
```

Potential problems:
  * How to generate anchors/aliases
  * How to generate directives
  * How to indicate inline sequences and maps

# Writer Method #

(thanks to Vadim Zeitlin)

```
using namespace std;

// just an example custom scalar type
struct Date { int d, m, y; };
ostream& operaot<<ostream& s, const Date& d)
{
   // this implementation suffers from many problems, it's
   // just an example and shouldn't be really used like this
       s << d.year << '-' << d.month << '-' << d.day;
}

// an example struct type
struct Person { string name; int age };


YAML::Writer w(cout);           // or any other ostream
w.SetIndent(4);                 // example of setting options
w.SetCompactMaps(false);        // choose
                                //   -
                                //     key: value
                                // rather than (default)
                                //   - key: value
                                // style

// text after "=> " below indicates the expected output

// this may or may not be done by default, but in any case we need
// to have it to allow outputting multiple documents in the same
// stream
w.StartDocument();              // => "---"

w.Comment("Generated automatically, don't modify");
// => # Generated automatically, don't modify

// each entry may or may not be named
w.Scalar(17);                   // => "- 17"
w.Name("Age").Scalar(17);       // => "Age: 18"

// but usually a convenient shortcut will be used:
w.Scalar("Age", 18);            // exactly the same as above

// we could be even cuter and overload operator() to allow writing
w("Age", 18);
// but I'm not sure if this is not going to be too confusing

// appropriate scalar format should be auto-detected:
w.Scalar("Version", 1.0);       // => Version: 1.0
w.Scalar("Name", "Foo");        // => Name: Foo
                                       // (notice absence of quoting)
w.Scalar("Desc", "One\nTwo");   // => Desc: "One\nTwo"
w.Scalar("Author", "A \"YAML\" Hacker"); // => Author: 'A "YAML" Hacker'

w.Scalar("Comment",
         "very long multiline string which wouldn't fit on a single"
         "line");                // => Comment: >
                                 //        very long multiline
                                 //        string which wouldn't fit
                                 //        on a single line

// but some of them can't be auto-detected and, in any case, it
// should be possible to override by using flags
w.Scalar("Logo", "A\n B\n  C",  // => Logo: |
         YAML::Block);          //          A
                                //           B
                                //            C
w.Scalar("Sign", "--", YAML::SingleQuote);// => Sign: '--'

// some scalar types may need special handling

// this uses default format, maybe there should be a global
// option to choose it
w.Scalar("Latest", true);       // => Latest: y

// this special method allows to specify the format to use
w.Bool("Latest", true, YAML::LongBool); // => Latest: yes
w.Bool("Latest", true, YAML::OnOffBool | YAML::UpperCase); // => Latest: ON

// it's also probably better to have separate named methods for
// integer options
w.Scalar("Mode", 0755);         // => Mode: 493
w.Oct("Mode", 0755);            // => Mode: 0755
w.Hex("Mode", 0755);            // => Mode: 0x1ed

// ... although we could also have flags for it, e.g.
w.Scalar("Max", 256, YAML::Hex); // => Max: 0x100

// using NULL is not going to work (C++0x nullptr would...) so we
// need a special method for this
w.Null("Extra");                // => Extra: ~

// custom types may be used provided they overload operator<<()
w.Scalar("Created", Date(2008, 9, 7)); // => Created: 2008-09-07


// structs are more complicated: we can't output them without some
// help so the user code must provide specialization of YAMLFields
// template for any custom type it wants to serialize
template <>
struct YAMLFields<Person> {
    typedef void (*Writer)(YAML::WriteName&, const Person&);

    static void WriteName(YAML::Writer& w, const Person& p) {
        w.Scalar("name", p.name);
    }

    static void WriteAge(YAML::Writer& w, const Person& p) {
        w.Scalar("age", p.age);
    }

    // this method must return a NULL-terminated array of
    // functions writing individual fields, YAML::Writer will
    // call them in turn
    static Writer *GetWriters() {
        static const Writer writers[] = {
            &WriteName, &WriteAge, NULL
        };

        return writers;
    }

    // TODO: I'd like to do something similar for reading
    // instead of doing it manually
};

// then this works (and chooses the inline style for short strings)
w.Map("Me", Person("VZ", 77));  // => Me: { name: VZ, age: 77 }

// maybe there should be some simpler API for ad-hoc maps output,
// although this is not going to work for saving arrays of structs
YAML::Map map;
map["name"] = "VZ";
map["age"] = 66;
w.Map(map);


// arrays are pretty straightforward as they simply delegate to
// the handling of their individual elements

// any iterators are supported and best format is auto-detected
int primes[] = { 2, 3, 5, 7, 11 };
w.Seq("Primes", primes, primes + sizeof(primes)/sizeof(primes[0]));
                                 // => Primes: [ 2, 3, 5, 7, 11 ]

// format can be overridden
w.Seq("Primes", primes, primes + sizeof(primes)/sizeof(primes[0]),
       YAML::Block);           // => Primes:
                               //      - 2
                               //      - 3
                               //      - 5
                               //      - 7
                               //      - 11

// custom types are also supported, of course, and there is a
// convenient wrapper for containers which frees you from using
// begin() and end() in the most common case
vector<Person> folks;
folks.push_back(Person("me", 10));
folks.push_back(Person("you", 100));
w.Seq("we", folks);             // => we:
                                //      - name: me
                                //        age: 10
                                //      - name: you
                                //        age: 100

// this may or may not be done by default
w.EndDocument();                // => ...
```

# Stream method #

This is a combination of the above Writer method and STL ostream manipulators.

The simple case:
```
YAML::Emitter out;
out << "Hello, World!";
out.WriteToFile("out.yaml");
// Hello, World!
std::ofstream fout("other.yaml");
out.WriteToStream(fout);
// same thing
```
A simple list:
```
YAML::Emitter out;
out << YAML::BeginSeq;
out << "eggs";
out << "bread";
out << "milk";
out << YAML::EndSeq;  // if we omit this, should we a) throw or b) assume it's there?
// - eggs
// - bread
// - milk
```
A simple map:
```
YAML::Emitter out;
out << YAML::BeginMap;
out << YAML::Key << "name";  // should we instead have a one-off method for creating a key/value pair?
out << YAML::Value << "Ryan Braun";
out << YAML::Key << "position";
out << YAML::Value << "3B";
out << YAML::EndMap;
// name: Ryan Braun
// position: 3B
```
Examples can be mixed:
```
YAML::Emitter out;
out << YAML::BeginMap;
out << YAML::Key << "name";
out << YAML::Value << "Barack Obama";
out << YAML::Key << "children";
out << YAML::Value << YAML::BeginSeq << "Sasha" << "Malia" << YAML::EndSeq;
out << YAML::EndMap;
// name: Barack Obama
// children:
//   - Sasha
//   - Malia
```
Output format can be modified using manipulators:
```
YAML::Emitter out;
out << YAML::Literal << "A\n B\n  C";
// |
// A
//  B
//   C
```
```
YAML::Emitter out;
out << YAML::Flow;
out << YAML::BeginSeq << 2 << 3 << 5 << 7 << 11 < YAML::EndSeq;
// [2, 3, 5, 7, 11]
```
We can also have comments:
```
YAML::Emitter out;
out << YAML::BeginMap;
out << YAML::Key << "method";
out << YAML::Value << "least squares";
out << YAML::Comment("should we change this method?");
out << YAML::EndMap;
// method: least squares # should we change this method?
```
And aliases/anchors:
```
YAML::Emitter out;
out << YAML::BeginSeq;
out << YAML::Anchor("fred");
out << YAML::BeginMap;
out << YAML::Key << "name" << YAML::Value << "Fred";
out << YAML::Key << "age" << YAML::Value << "42";
out << YAML::EndMap;
out << YAML::Alias("fred");
out << YAML::EndSeq;
// - &fred
//   name: Fred
//   age: 42
// - *fred
```
As usual, we can overload `operator <<`:
```
struct Vec3 { int x; int y; int z; };
YAML::Emitter& operator << (YAML::Emitter& out, const Vec3& v) {
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << v.z << YAML:EndSeq;
	return out;
}
```
We'll provide overloads for STL vectors (and lists and sets and any container with an iterator and a dereferencable value?)
```
YAML::Emitter out;
std::vector <int> values;
values.push_back(3);
values.push_back(5);
out << values;
// - 3
// - 5
```
... and maps:
```
YAML::Emitter out;
std::map <std::string, int> ages;
ages["Daniel"] = 26;
ages["Jesse"] = 24;
out << ages;
// Daniel: 26
// Jesse: 24
```
All manipulators affect **only** the next item in the stream they all have global setters that can be **locally** changed using a manipulator:
```
YAML::Emitter out;
out.SetIndent(4);
out.SetMapStyle(YAML::Flow);
```

List of string manipulators:
```
YAML::Auto;  // auto-detect - try for no quotes, but put double quotes if necessary
YAML::SingleQuoted;
YAML::DoubleQuoted;
YAML::Literal;
```
List of bool manipulators:
```
YAML::YesNoBool;  // yes, no
YAML::TrueFalseBool;  // true, false
YAML::OnOffBool;  // on, off
YAML::UpperCase;  // TRUE, N
YAML::LowerCase;  // f, yes
YAML::CamelCase;  // No, Off
YAML::LongBool;  // yes, On
YAML::ShortBool;  // y, t
```
List of int manipulators:
```
YAML::Dec;
YAML::Hex;
YAML::Oct;
```
List of sequence manipulators:
```
YAML::Flow;
YAML::Block;
```
List of map manipulators:
```
YAML::Flow;
YAML::Block;
YAML::Auto;  // auto-detect key - try for simple key, but make it a long key if necessary
YAML::LongKey;
```
Note about sequence/map manipulators: they're the same locally, but they have different global flag setters (which take the same parameters)

List of misc manipulators:
```
YAML::Null;
YAML::NewLine;  // print a blank line
```

List of comment manipulators:
```
YAML::Comment;
YAML::Indent(int n);  // number of spaces to indent a comment after a value
YAML::PostIndent(int n);  // number of spaces to indent the actual text of a comment after the '#' sign
// e.g., with Indent(3) and PostIndent(2):
// Hello   #  this is a comment
```