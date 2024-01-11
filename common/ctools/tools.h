/**
 * author: brando
 * date: 1/11/24
 */

#ifndef TOOLS_H
#define TOOLS_H

#ifdef TESTING
#define TOOL_TEST main
#define TOOL_MAIN __main
#else // TESTING
#define TOOL_TEST __main
#define TOOL_MAIN main
#endif // TESTING

#endif // TOOLS_H

