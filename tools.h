/*
Copyright (c) 2012 Gerhard Reitmayr <gerhard.reitmayr@gmail.com>

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#ifndef _TOOLS_H_
#define _TOOLS_H_

#include <array>
#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <typeinfo>
#include <vector>

/**
\mainpage C++ Tools

A single header file with many C++ tools for convenient prototyping.

  - \ref options convenient, lightweight option parsing
  - \ref iostream more input/output
*/

/**
\defgroup iostream IOStream operators
More input/output operators for various templates.
@{
*/

template <typename T, int N>
std::ostream & operator<<( std::ostream & out, const std::array<T, N> & a ){
    for(size_t i = 0; i < N-1; ++i)
        out << a[i] << " ";
    return out << a[N-1];
}

template <typename T, int N>
std::istream & operator>>( std::istream & in, std::array<T, N> & a ){
    for(size_t i = 0; i < N; ++i)
        in >> a[i];
    return in;
}

template <typename T, typename A>
std::ostream & operator<<( std::ostream & out, const std::vector<T, A> & v ){
    if(v.size() > 1)
        for(size_t i = 0; i < v.size()-1; ++i)
            out << v[i] << " ";
    if(!v.empty())
        out << v.back();
    return out;
}

/**
tries to read as many elements as possible from in and pushes them into the vector v.
*/
template <typename T, typename A>
std::istream & operator>>( std::istream & in, std::vector<T, A> & v ){
    T temp;
    while(in >> temp) 
        v.push_back(temp);
    return in;
}

/**
@}
*/

/**
\defgroup options Option parsing
A set of very simple functions to parse options and set variables to the new values.
The following example shows how to use the options mechanism:

\code
string test;                // defines a string variable that is set with -t
options::make(test, "t");

bool yesno = false;         // defines a flag to be enabled with -y
options::make(yesno, "y");

double param = 100;         // a double variable set with -p
options::make(param, "p");

ofstream log;               // an ofstream that opens a file given with -l
options::make(log, "l");

options::print();           // prints the current table of known options
cout << endl;

options::parse(argc, argv); // parse the input options

// now the values are set in the variables defined above and can be used 
// in the program as usuall.
cout << test << "\t" << yesno << "\t" << param << endl;
log << test << "\t" << yesno << "\t" << param << endl;
\endcode

*/
namespace options {

struct option {
    virtual bool parse() = 0;
    virtual void parse( const std::string & opt ) = 0;
    virtual std::string type() const = 0;
    virtual std::string value() const = 0;
};

template <int LIFT = 0>
struct store {
    static std::map<std::string, option *> flags;
};

template <int LIFT> std::map<std::string, option *> store<LIFT>::flags;

template <typename T>
struct typed_option : public option {
    T & val;
    typed_option( T & v ) : val(v) {}
    
    bool parse() { return true; };
    void parse( const std::string & opt ){
        std::istringstream in(opt);
        in >> val;
    }

    std::string type() const { return typeid(val).name(); }
    std::string value() const {
        std::ostringstream out;
        out << val;
        return out.str();
    }
};

template <>
struct typed_option<bool> : public option {
    bool & val;
    typed_option( bool & v ) : val(v) {}
    
    bool parse() { val = true; return false; };
    void parse( const std::string & opt ) {}
    std::string type() const { return "bool"; }
    std::string value() const {
        std::ostringstream out;
        out << val;
        return out.str();
    }
};

template <>
struct typed_option<std::string> : public option {
    std::string & val;
    typed_option( std::string & v ) : val(v) {}
    
    bool parse() { return true; };
    void parse( const std::string & opt ) { val = opt; }
    std::string type() const { return typeid(val).name(); }
    std::string value() const { return val; }
};

template <>
struct typed_option<std::ofstream> : public option {
    std::ofstream & val;
    typed_option( std::ofstream & v ) : val(v) {}
    
    bool parse() { return true; };
    void parse( const std::string & opt ){
        val.open(opt.c_str());
    }

    std::string type() const {
        return typeid(val).name();
    }
    std::string value() const { return "file"; }
};

/**
\ingroup options
defines a new option. This function takes a reference to the variable to set 
and a string giving the name of the option as used on the command line.
*/
template <typename T>
static inline void make( T & val, const std::string & name ){
    option * new_option = new typed_option<T>( val );
    store<>::flags[name] = new_option;
}

/**
parses the option values and sets the option variables. This function parses
the input command line arguments of the program and sets the option values
appropriately. It stops parsing when it encounters an unknown string (not starting 
with '-') and returns the index of the last unparsed string.
\ingroup options
*/
static inline int parse( const int argc, char ** argv ){
    int i = 1;
    while(i < argc){
        if(argv[i][0] == '-'){
            const std::string opt = argv[i]+1;
            std::map<std::string, option *>::const_iterator flag = store<>::flags.find(opt);
            if( flag != store<>::flags.end()){
                if(flag->second->parse() && i < argc-1){
                    ++i;
                    flag->second->parse(argv[i]);
                }
            }
            ++i;
        } else 
            break;
    }
    return i;
}

/**
prints a simple table with option name, current value and type name. The 
typename is the compiler given one and may not make much sense.
\ingroup options
*/
static inline void print( std::ostream & out = std::cout ){
    out << "option\tdefault\ttype\n";
    for(std::map<std::string, option *>::const_iterator flag = store<>::flags.begin(); flag != store<>::flags.end(); ++flag){
        out << flag->first << "\t" << flag->second->value() << "\t" << flag->second->type() << "\n";
    }
}

} // namespace options

#endif // _TOOLS_H_
