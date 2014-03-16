/*
 * DebugResult.cpp
 *
 *  Created on: Mar 4, 2014
 *  \copyright 2014 DCBlaha.  Distributed under the GPL.
 */

#include "DebugResult.h"
#include "Debug.h"

#define DBG_RESULT 0
#if(DBG_RESULT)
DebugFile sDbgFile("DbgResult.txt");
#endif

static char const *skipSpace(char const *p)
    {
    while(isspace(*p))
        p++;
    return p;
    }

static bool isStartListC(char c)
    { return(c == '[' || c == '{'); }

static bool isEndListC(char c)
    { return(c == ']' || c == '}'); }

static bool isListItemSepC(char c)
    { return(c == ','); }

cDebugResult::~cDebugResult()
    {
    for(auto const &elem : mChildResults)
	{
	delete elem;
	}
    }

std::string cDebugResult::getAsString(int level) const
    {
    std::string str;

    std::string leadSpace(level*3, ' ');
    str = leadSpace;
    if(mVarName.length())
	{
	str = mVarName;
	str += " = ";
	if(mChildResults.size())
	    {
	    for(auto const &elem : mChildResults)
		{
		str += elem->getAsString(level+1);
		}
	    }
	}
    str += mConst;
    str += "\n";
    return str;
    }

// The following characters are prefixed with a backslash when they are in
// quotes:
//      backslash, doublequote
static char const *parseString(char const *resultStr)
    {
#if(DBG_RESULT)
sDbgFile.printflush("parseString %s\n", resultStr);
#endif
    char const *p = resultStr;
    if(*p == '\"')
	p++;
    while(*p)
	{
	if((*p == '\\' && *(p+1) == '\\' && *(p+2) == '\\' && *(p+3) == '\\') ||
		(*p == '\\' && *(p+1) == '\\' && *(p+2) == '\\' && *(p+3) == '\"'))
	    {
	    p+=3;
	    }
	else if(*p == '\\' && *(p+1) == '\"')
	    {
	    break;
	    }
	p++;
	}
    if(*p)
	p++;
#if(DBG_RESULT)
sDbgFile.printflush("   parseString %s\n", std::string(resultStr, p-resultStr).c_str());
#endif
    return p;
    }

// Parses stuff up to the end of an item.
// An item can be a variable name, or a value/const
static char const *parseItem(char const *resultStr)
    {
#if(DBG_RESULT)
sDbgFile.printflush("parseItem %s\n", resultStr);
#endif
    char const *start = skipSpace(resultStr);
    char const *p = start;
    while(*p)
        {
	if(*p == '\\' && *(p+1) == '\"')
	    {
	    p++;
	    p = parseString(p);
	    }
        else if(*p == '=' || isListItemSepC(*p) || isEndListC(*p))
            {
            break;
            }
        p++;
        }
#if(DBG_RESULT)
sDbgFile.printflush("   parseItem %s\n", std::string(start, p-start).c_str());
#endif
    return p;
    }

char const *cDebugResult::parseVarName(char const *resultStr)
    {
#if(DBG_RESULT)
sDbgFile.printflush("parseVarName %s\n", resultStr);
#endif
    char const *start = skipSpace(resultStr);
    char const *p = parseItem(start);
    if(*p == '=')
	{
	setVarName(start, p-start);
#if(DBG_RESULT)
sDbgFile.printflush("   parseVarName %s\n", std::string(start, p-start).c_str());
#endif
	p++;
	}
    else
	{
	p = start;
	}
    return p;
    }

char const *cDebugResult::parseConst(char const *resultStr)
    {
#if(DBG_RESULT)
sDbgFile.printflush("parseConst %s\n", resultStr);
#endif
    char const *start = skipSpace(resultStr);
    char const * const p = parseItem(start);
#if(DBG_RESULT)
sDbgFile.printflush("   parseConst %s\n", std::string(start, p-start).c_str());
#endif
    mConst.assign(start, p-start);
    return p;
    }

// Parse a list of comma separated stuff. The list ends when
// isEndC returns true.
char const *cDebugResult::parseList(char const *resultStr)
    {
#if(DBG_RESULT)
sDbgFile.printflush("parseList\n");
#endif
    char const *p = resultStr;
    while(*p)
        {
        if(isListItemSepC(*p))
            {
            p++;
            }
        else if(isEndListC(*p))
            {
            break;
            }
        else
            {
            cDebugResult &res = addResult();
            p = res.parseResult(p);
            }
        }
    return p;
    }

// "value=" has already been parsed.
// Ex:  value="{mVarName = {
//	static npos = <optimized out>,
//	_M_dataplus =	{
//	<std::allocator<char>> = {<__gnu_cxx::new_allocator<char>> =
//	{<No data fields>},
//	<No data fields>},
//	_M_p = 0x6fcc3fa4 <libstdc++-6!_ZNSs4_Rep20_S_empty_rep_storageE+12> \"\"}}, "
//	"mConst = {
//	static npos = <optimized out>,
//	_M_dataplus ="" {
//	<std::allocator<char>> = {
//	<__gnu_cxx::new_allocator<char>> ={<No data fields>},
//	<No data fields>},
//	_M_p = 0x6fcc3fa4 <libstdc++-6!_ZNSs4_Rep20_S_empty_rep_storageE+12> \"\"}},
//	mChildResults = {c = {<std::_Deque_base<cDebugResult*,
//	std::allocator<cDebugResult*> >> = {_M_impl = {<std::allocator<cDebugResult*>>
//	= {<__gnu_cxx::new_allocator<cDebugResult*>> = {<No data fields>},
//	<No data fields>}, _M_map = 0x6a11e0, _M_map_size = 8, _M_start =
//	{_M_cur = 0x6a1218, _M_first = 0x6a1218, _M_last = 0x6a1418, _M_node
//	= 0x6a11ec}, _M_finish = {_M_cur = 0x6a1218, _M_first = 0x6a1218,
//	_M_last = 0x6a1418, _M_node = 0x6a11ec}}}, <No data fields>}}}"};
//
//     char const * const square = "{curly}";
//		value="0x40d1af <_ZStL19piecewise_construct+159> \"{curly}\""
//
//		value="0x40d1b7 <_ZStL19piecewise_construct+167> \"[square]\""
//		value="0x40d1bf <_ZStL19piecewise_construct+175> \"\\\"quotes\\\"\""
//
//	value="{curly = 0x40d1af <_ZStL19piecewise_construct+159> \"{curly}\",
//	    square = 0x40d1b7 <_ZStL19piecewise_construct+167> \"[square]\",
// 	    quotes = 0x40d1c0 <_ZStL19piecewise_construct+176> \"\\\"quotes\\\"\"}"
char const *cDebugResult::parseResult(char const *resultStr)
    {
#if(DBG_RESULT)
sDbgFile.printflush("parseResult %s\n", resultStr);
#endif
    char const *start = skipSpace(resultStr);
    char const *p = start;
    p = parseVarName(p);
    if(*p == '[' || *p == '{')
	{
	p = parseList(p);
	}
    else if(*p == '\"')
	{
	p++;
	if(isStartListC(*p))
	    {
	    p++;
	    p = parseList(p);
	    }
	else
	    p = parseConst(p);
	if(*p == '\"')
	    p++;
	}
    else
	p = parseConst(p);
    return p;
    }