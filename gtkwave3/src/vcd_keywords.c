/* C code produced by gperf version 3.0.3 */
/* Command-line: /usr/bin/gperf -o -i 1 -C -k '1,$' -L C -H keyword_hash -N check_identifier -tT ./vcd_keywords.gperf  */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gnu-gperf@gnu.org>."
#endif

#line 1 "./vcd_keywords.gperf"


/* AIX may need this for alloca to work */
#if defined _AIX
  #pragma alloca
#endif

#include <config.h>
#include <string.h>
#include "vcd.h"

struct vcd_keyword { const char *name; int token; };


#define TOTAL_KEYWORDS 25
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 14
#define MIN_HASH_VALUE 5
#define MAX_HASH_VALUE 41
/* maximum key range = 37, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
keyword_hash (str, len)
     register const char *str;
     register unsigned int len;
{
  static const unsigned char asso_values[] =
    {
      42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
      42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
      42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
      42, 42, 42, 42, 42, 42, 26, 42, 42, 42,
      42, 42, 42, 42, 42, 42, 42, 42,  4,  1,
      42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
      42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
      42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
      42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
      42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
      11,  6, 42,  1, 42, 16, 42, 42, 24, 42,
       1,  6, 16, 42,  1,  6,  1, 42, 42, 21,
      42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
      42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
      42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
      42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
      42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
      42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
      42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
      42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
      42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
      42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
      42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
      42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
      42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
      42, 42, 42, 42, 42, 42
    };
  return len + asso_values[(unsigned char)str[len - 1]] + asso_values[(unsigned char)str[0]];
}

#ifdef __GNUC__
__inline
#ifdef __GNUC_STDC_INLINE__
__attribute__ ((__gnu_inline__))
#endif
#endif
const struct vcd_keyword *
check_identifier (str, len)
     register const char *str;
     register unsigned int len;
{
  static const struct vcd_keyword wordlist[] =
    {
      {""}, {""}, {""}, {""}, {""},
#line 23 "./vcd_keywords.gperf"
      {"reg", V_REG},
#line 32 "./vcd_keywords.gperf"
      {"tri1", V_TRI1},
#line 29 "./vcd_keywords.gperf"
      {"trior", V_TRIOR},
#line 30 "./vcd_keywords.gperf"
      {"trireg", V_TRIREG},
#line 31 "./vcd_keywords.gperf"
      {"tri0", V_TRI0},
#line 38 "./vcd_keywords.gperf"
      {"out", V_OUT},
#line 26 "./vcd_keywords.gperf"
      {"time", V_TIME},
#line 17 "./vcd_keywords.gperf"
      {"event", V_EVENT},
#line 40 "./vcd_keywords.gperf"
      {"string", V_STRINGTYPE},
#line 25 "./vcd_keywords.gperf"
      {"supply1", V_SUPPLY1},
#line 22 "./vcd_keywords.gperf"
      {"realtime", V_REALTIME},
#line 21 "./vcd_keywords.gperf"
      {"real_parameter", V_REAL_PARAMETER},
#line 24 "./vcd_keywords.gperf"
      {"supply0", V_SUPPLY0},
#line 28 "./vcd_keywords.gperf"
      {"triand", V_TRIAND},
#line 37 "./vcd_keywords.gperf"
      {"in", V_IN},
#line 27 "./vcd_keywords.gperf"
      {"tri", V_TRI},
#line 36 "./vcd_keywords.gperf"
      {"port", V_PORT},
#line 39 "./vcd_keywords.gperf"
      {"inout", V_INOUT},
      {""},
#line 19 "./vcd_keywords.gperf"
      {"integer", V_INTEGER},
#line 35 "./vcd_keywords.gperf"
      {"wor", V_WOR},
#line 18 "./vcd_keywords.gperf"
      {"parameter", V_PARAMETER},
      {""}, {""},
#line 20 "./vcd_keywords.gperf"
      {"real", V_REAL},
      {""},
#line 34 "./vcd_keywords.gperf"
      {"wire", V_WIRE},
      {""}, {""}, {""}, {""},
#line 33 "./vcd_keywords.gperf"
      {"wand", V_WAND},
      {""}, {""}, {""}, {""},
#line 41 "./vcd_keywords.gperf"
      {"$end", V_END}
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = keyword_hash (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        {
          register const char *s = wordlist[key].name;

          if (*str == *s && !strcmp (str + 1, s + 1))
            return &wordlist[key];
        }
    }
  return 0;
}
#line 42 "./vcd_keywords.gperf"


int vcd_keyword_code(const char *s, unsigned int len)
{
const struct vcd_keyword *rc = check_identifier(s, len);
return(rc ? rc->token : V_STRING);
}

/*
 * $Id: vcd_keywords.gperf,v 1.3 2010/12/12 18:32:45 gtkwave Exp $
 * $Log: vcd_keywords.gperf,v $
 * Revision 1.3  2010/12/12 18:32:45  gtkwave
 * add "string" variable type to parsing of vcd variable declarations
 *
 * Revision 1.2  2010/02/22 21:13:36  gtkwave
 * added "realtime" VCD variable
 *
 * Revision 1.1.1.1  2007/05/30 04:27:35  gtkwave
 * Imported sources
 *
 * Revision 1.2  2007/04/20 02:08:17  gtkwave
 * initial release
 *
 */

