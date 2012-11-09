#ifndef _INCLUDE_STDLIB_
#define _INVLUDE_STDLIB_
#include <stdlib.h>
#endif

#ifndef _INCLUDE_STDIO_
#define _INCLUDE_STDIO_
#include <stdio.h>
#endif

#ifndef _INCLUDE_STRING_
#define _INCLUDE_STRING_
#include <string.h>
#endif

#ifndef _LINKPARSER_H
#define _LINKPARSER_H

#define LINKNODEA "<a"
#define LINKTOFIND "href"
#define CATEGORY1 ".html"
#define CATEGORY2 ".htm"
#define HTTPCATE "http:"
#define POTPOT "../"
/**
*analysis the href
*return all the suitable url
*/
int extract_link(char *dir, char *htmltxt, char res[][256]);
#endif

