//
// Created by wbull on 5/24/20.
//

#pragma once

class WView;

class CoordCache
{
public:
    enum ccache_type
    {
        CCACHE_OFFSET,
        CCACHE_LINECOL
    };
public:
    /* A cache entry for mapping offsets into line/column pairs and vice versa.
     * cc_offset, cc_line, and cc_column are the 0-based values of the offset,
     * line and column of that cache entry. cc_nroff_column is the column
     * corresponding to cc_offset in nroff mode.
     */
    struct coord_cache_entry_t
    {
        off_t cc_offset;
        off_t cc_line;
        off_t cc_column;
        off_t cc_nroff_column;
    };

    struct coord_cache_t
    {
        size_t size;
        size_t capacity;
        coord_cache_entry_t **cache;
    };
private:
    enum nroff_state_t
    {
        NROFF_START,
        NROFF_BACKSPACE,
        NROFF_CONTINUATION
    };
private:
    static constexpr int VIEW_COORD_CACHE_GRANUL = 1024;
    static constexpr int CACHE_CAPACITY_DELTA = 64;

    // FIXME DB use `using` or std::function
    typedef gboolean (*cmp_func_t) (const coord_cache_entry_t * a, const coord_cache_entry_t * b);
public:
    static coord_cache_t* coord_cache_new();

    static void coord_cache_free(coord_cache_t * cache);

#ifdef MC_ENABLE_DEBUGGING_CODE
    static void mcview_ccache_dump (WView* view);
#endif

    /** Look up the missing components of ''coord'', which are given by
     * ''lookup_what''. The function returns the smallest value that
     * matches the existing components of ''coord''.
     */
    static void mcview_ccache_lookup(WView* view, coord_cache_entry_t* coord, enum ccache_type lookup_what);

private:
    /** Find and return the index of the last cache entry that is
    * smaller than ''coord'', according to the criterion ''sort_by''. */
    static size_t mcview_ccache_find(WView* view, const coord_cache_entry_t * coord, cmp_func_t cmp_func);

    static gboolean mcview_coord_cache_entry_less_nroff(const coord_cache_entry_t* a, const coord_cache_entry_t* b);

    static gboolean mcview_coord_cache_entry_less_plain(const coord_cache_entry_t* a, const coord_cache_entry_t* b);

    static gboolean mcview_coord_cache_entry_less_offset(const coord_cache_entry_t* a, const coord_cache_entry_t* b);

    /* insert new cache entry into the cache */
    static void mcview_ccache_add_entry(coord_cache_t* cache, size_t pos, const coord_cache_entry_t* entry);
};
