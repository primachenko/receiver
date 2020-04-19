#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>

#ifdef DEBUG
#define DEBUG_ALL
#endif /* DEBUG */

#if DETECTOR_DEBUG | DEBUG_ALL
	#define DTCT_DBG(__info__,...) { fprintf(stderr, "[DTCT-DBG]: <%s> %d: " __info__, __FUNCTION__, __LINE__, ##__VA_ARGS__); fprintf(stderr, "\n"); }
#else
	#define DTCT_DBG(__info__,...)
#endif /* DETECTOR_DEBUG | DEBUG_ALL */

#define DTCT_ERR(__info__,...) { fprintf(stderr, "[DTCT-ERR]: <%s> %d: " __info__, __FUNCTION__, __LINE__, ##__VA_ARGS__); fprintf(stderr, "\n"); }

#if RECEIVER_DEBUG | DEBUG_ALL
	#define RCVR_DBG(__info__,...) { fprintf(stderr, "[RCVR-DBG]: <%s> %d: " __info__, __FUNCTION__, __LINE__, ##__VA_ARGS__); fprintf(stderr, "\n"); }
#else
	#define RCVR_DBG(__info__,...)
#endif /* RECEIVER_DEBUG | DEBUG_ALL */

#define RCVR_ERR(__info__,...) { fprintf(stderr, "[RCVR-ERR]: <%s> %d: " __info__, __FUNCTION__, __LINE__, ##__VA_ARGS__); fprintf(stderr, "\n"); }
#define RCVR_CB(__info__,...) { fprintf(stderr, "[RCVR-CB]: <%s> %d: " __info__, __FUNCTION__, __LINE__, ##__VA_ARGS__); fprintf(stderr, "\n"); }

#if DUMP_DEBUG | DEBUG_ALL
	#define DMP_DBG(__info__,...) { fprintf(stderr, "[DMP-DBG]: <%s> %d: " __info__, __FUNCTION__, __LINE__, ##__VA_ARGS__); fprintf(stderr, "\n"); }
#else
	#define DMP_DBG(__info__,...)
#endif /* DUMP_DEBUG | DEBUG_ALL */

#define DMP_ERR(__info__,...) { fprintf(stderr, "[DMP-ERR]: <%s> %d: " __info__, __FUNCTION__, __LINE__, ##__VA_ARGS__); fprintf(stderr, "\n"); }

#if IMPORT_DEBUG | DEBUG_ALL
	#define IMP_DBG(__info__,...) { fprintf(stderr, "[IMP-DBG]: <%s> %d: " __info__, __FUNCTION__, __LINE__, ##__VA_ARGS__); fprintf(stderr, "\n"); }
#else
	#define IMP_DBG(__info__,...)
#endif /* IMPORT_DEBUG | DEBUG_ALL */

#define IMP_ERR(__info__,...) { fprintf(stderr, "[IMP-ERR]: <%s> %d: " __info__, __FUNCTION__, __LINE__, ##__VA_ARGS__); fprintf(stderr, "\n"); }

#if MAIN_DEBUG | DEBUG_ALL
	#define MAIN_DBG(__info__,...) { fprintf(stderr, "[MAIN-DBG]: <%s> %d: " __info__, __FUNCTION__, __LINE__, ##__VA_ARGS__); fprintf(stderr, "\n"); }
#else
	#define MAIN_DBG(__info__,...)
#endif /* MAIN_DEBUG | DEBUG_ALL */

#define MAIN_ERR(__info__,...) { fprintf(stderr, "[MAIN-ERR]: <%s> %d: " __info__, __FUNCTION__, __LINE__, ##__VA_ARGS__); fprintf(stderr, "\n"); }

#endif /* DEBUG_H */
