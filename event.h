#ifndef EVENT_H
#define EVENT_H

#ifdef __cplusplus
extern "C" {
#endif

/*自定义模型个数*/
#define SELFEVMODENUM (2)

/*操作系统中模型个数*/
#define SYSEVMODENUM (5)

/*选择事件*/
#if defined(_WIN32)
#error no support operate system
#include <winsock.h>
#define createeventtop(reactor) \
{ \
setevtopep((etlist), 0); \
setevtoppo((etlist), 1); \
setevtopse((etlist), 2); \
setevtopdp((etlist), 3); \
}

#elif defined(__APPLE__) && defined(__MACH__)
#include "kqueue.h"
#include "poll.h"
#include "select.h"
#include "dpoll.h"
#define createeventtop(etlist) \
{ \
setevtopkq((etlist), 0); \
setevtoppo((etlist), 1); \
setevtopse((etlist), 2); \
setevtopdp((etlist), 3); \
}

#elif defined(__linux__) || defined(_AIX) || defined(__FreeBSD__)
#include "epoll.h"
#include "poll.h"
#include "select.h"
#include "dpoll.h"
#define createeventtop(reactor) \
{ \
setevtopep((etlist), 0); \
setevtoppo((etlist), 1); \
setevtopse((etlist), 2); \
setevtopdp((etlist), 3); \
}

#else
#error no support operate system
#endif
    
#ifdef __cplusplus
}
#endif

#include "reactor.h"

#endif
