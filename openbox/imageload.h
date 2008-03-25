#ifndef __imageload_h
#define __imageload_h

#ifdef USE_IMLIB2
#include "obrender/render.h"
RrImage* RrImageFetchFromFile(RrImageCache *cache, const gchar *name);
#else
#define RrImageFetchFromFile(cache, name) NULL
#endif

#endif
