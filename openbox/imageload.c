/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-
   imageload.c for the Openbox window manager
   by Libor Kadlcik (aka KadlSoft)
*/

/*
    All loaded images are cached. There's no separate cache for the images,
    instead they are simply stored in image cache (RrImageCache) as RrImages,
    ready to be used.
    Every RrImage loaded from file is associated with name of the file. This is
    done by file name table (RrImageCache.file_name_table), which is a simple
    hash table, where file names are keys to pointers to RrImage.
    If you request to load file that is already in image cache, nothing will be
    loaded and you just got the RrImage from cache.
    When RrImage is destroyed (see RrImageDestroyNotify), the file name - pointer
    to RrImage pair is removed from the file name table.
*/

#include "debug.h"
#include "menu.h"
#include "openbox.h"
#include "gettext.h"
#include "obrender/render.h"
#include "obrender/image.h"
#include "obrender/imagecache.h"
#include "imageload.h"
#include <Imlib2.h>


static void CreateFileNameTable(RrImageCache *self)
{
    g_assert(self->file_name_table == NULL);
    self->file_name_table = g_hash_table_new(&g_str_hash, &g_str_equal);
}

static void DestroyFileNameTable(RrImageCache *self)
{
    g_assert(g_hash_table_size(self->file_name_table) == 0);
    g_hash_table_destroy(self->file_name_table);
    self->file_name_table = NULL;
}

/*! Return file name from which this image has been loaded. */
static gchar* GetFileName(RrImage *image)
{
    GHashTableIter iter;
    void *key, *value;

    g_hash_table_iter_init(&iter, image->cache->file_name_table);
    while (g_hash_table_iter_next(&iter, &key, &value)) {
        if (value == image)
            return key;
    }
    return NULL;
}

/* RrImage is about to be deleted. So remove it from file name table. */
static void RrImageDestroyNotify(RrImage *image)
{
    gchar *file_name = GetFileName(image);
    g_assert(file_name != NULL);
    ob_debug("Image \"%s\" no longer needed\n", file_name);
    g_hash_table_remove(image->cache->file_name_table, file_name);
    g_free(file_name);

    if (g_hash_table_size(image->cache->file_name_table) == 0) {
        ob_debug("No RrImage in file_name_table, destroying\n");
        DestroyFileNameTable(image->cache);
    }
}

#if (RrDefaultAlphaOffset != 24 || RrDefaultRedOffset != 16 \
    || RrDefaultGreenOffset != 8 || RrDefaultBlueOffset != 0)
#error RrImageFetchFromFile cannot handle current bit layout of RrPixel32.
#endif

/*! Load image from specified file and create RrImage for it (RrImage will be
    linked into specified image cache). Reference count of the RrImage will
    be set to 1.
    If that image has already been loaded into the image cache, RrImage
    from the cache will be returned and its reference count will be incremented.
*/
RrImage* RrImageFetchFromFile(RrImageCache *cache, const gchar *name)
{
    RrImage *rr_image, *found_rr_image;
    gint w, h;
    DATA32 *ro_data;

    imlib_set_color_usage(128);

    if (cache->file_name_table == NULL)
        CreateFileNameTable(cache);

    /* Find out if that image has already been loaded to this cache. */
    rr_image = g_hash_table_lookup(cache->file_name_table, name);
    if (rr_image && rr_image->cache == cache) {
        ob_debug("\"%s\" already loaded in this image cache.\n", name);
        RrImageRef(rr_image);
        return rr_image;
    }

    Imlib_Image imlib_image = imlib_load_image(name);
    if (imlib_image == NULL) {
        g_message(_("Cannot load image from file \"%s\""), name);
        return NULL;
    }

    /* Get data and dimensions of the image. */
    imlib_context_set_image(imlib_image);
    g_message("Alpha = %d\n", imlib_image_has_alpha());
    ro_data = imlib_image_get_data_for_reading_only();
    w = imlib_image_get_width();
    h = imlib_image_get_height();
    ob_debug("Loaded \"%s\", dimensions %dx%d\n", name, w, h);

    /* There must not be any duplicated pictures in RrImageCache. */
    found_rr_image = RrImageCacheFind(cache, ro_data, w, h);
    if (found_rr_image) {
        rr_image = found_rr_image;
        RrImageRef(rr_image);
        ob_debug("Image \"%s\" is duplicate\n", name);
    }
    else {
        /* Create RrImage from the image and add it to file name table. */
        rr_image = RrImageNew(cache);
        RrImageSetDestroyFunc(rr_image, &RrImageDestroyNotify);
        /* XXX: Is Imlib2's format of DATA32 always identical to RrPixel32? */
        RrImageAddPicture(rr_image, ro_data, w, h);
        g_hash_table_insert(cache->file_name_table, g_strdup(name), rr_image);
    }

    imlib_free_image();

    return rr_image;
}
