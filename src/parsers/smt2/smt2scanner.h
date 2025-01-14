/*++
Copyright (c) 2006 Microsoft Corporation

Module Name:

    smt2scanner.h

Abstract:

    <abstract>

Author:

    Leonardo de Moura (leonardo) 2011-03-09.

Revision History:

--*/
#pragma once

#include<istream>
#include "util/symbol.h"
#include "util/vector.h"
#include "util/rational.h"
#include "cmd_context/cmd_context.h"


#include <sstream>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/filtering_stream.hpp>


namespace smt2 {


    template<typename Ch>
    class basic_tee_input_filter  {
    public:
        typedef Ch char_type;
        struct category
            : boost::iostreams::multichar_input_filter_tag
            { };
        explicit basic_tee_input_filter(std::ostream &os)
            : m_os(os)
            { }
        std::streamsize optimal_buffer_size() const { return 0; }

        template<typename Source>
        std::streamsize read(Source& src, char_type* s, std::streamsize n)
        {
            std::streamsize result = boost::iostreams::read(src, s, n);
            if (result < 0)
                return result;
            m_os.write(s, result);
            return result;
        }
    private:
        std::ostream &m_os;
    };
    BOOST_IOSTREAMS_PIPABLE(basic_tee_input_filter, 1)

    typedef basic_tee_input_filter<char>     tee_input_filter;
    typedef basic_tee_input_filter<wchar_t>  tee_winput_filter;


    typedef cmd_exception scanner_exception;
    typedef boost::iostreams::filtering_istream filtering_istream;

    class scanner {
    private:
        cmd_context&       ctx;
        bool               m_interactive;
        int                m_spos; // position in the current line of the stream
        char               m_curr;  // current char;
        bool               m_at_eof;
        
        int                m_line;  // line
        int                m_pos;   // start position of the token
        // data
        symbol             m_id;
        rational           m_number;
        unsigned           m_bv_size;
        // end of data
        signed char        m_normalized[256];
#define SCANNER_BUFFER_SIZE 1024
        char               m_buffer[SCANNER_BUFFER_SIZE];
        unsigned           m_bpos;
        unsigned           m_bend;
        svector<char>      m_string;
        // std::istream*      m_stream;
        filtering_istream* m_stream;
        std::stringstream* m_tee_stream;
        
        bool               m_cache_input;
        svector<char>      m_cache;
        svector<char>      m_cache_result;
        
        
        char curr() const { return m_curr; }
        void new_line() { m_line++; m_spos = 0; }
        void next();
        void dump();
        
    public:
        
        enum token {
            NULL_TOKEN = 0,
            LEFT_PAREN = 1,
            RIGHT_PAREN,
            KEYWORD_TOKEN,
            SYMBOL_TOKEN,
            STRING_TOKEN,
            INT_TOKEN,
            BV_TOKEN,
            FLOAT_TOKEN,
            EOF_TOKEN
        };
        
        scanner(cmd_context & ctx, std::istream& stream, bool interactive = false);
        ~scanner();
        
        int get_line() const { return m_line; }
        int get_pos() const { return m_pos; }
        symbol const & get_id() const { return m_id; }
        rational get_number() const { return m_number; }
        unsigned get_bv_size() const { return m_bv_size; }
        char const * get_string() const { return m_string.begin(); }
        token scan();
        
        token read_symbol_core();
        token read_symbol();
        token read_quoted_symbol();
        void read_comment();
        void read_multiline_comment();
        token read_number();
        token read_signed_number();
        token read_string();
        token read_bv_literal();

        void start_caching() { m_cache_input = true; m_cache.reset(); }
        void stop_caching() { m_cache_input = false; }
        unsigned cache_size() const { return m_cache.size(); }
        void reset_cache() { m_cache.reset(); }
        void reset_input(std::istream & stream, bool interactive = false);

        char const * cached_str(unsigned begin, unsigned end);
    };

};


