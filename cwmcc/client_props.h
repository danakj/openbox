#ifndef __cwmcc_client_get_props_h
#define __cwmcc_client_get_props_h

void cwmcc_client_get_protocols(Window win, Atom **protocols);

void cwmcc_client_get_wm_state(Window win, gulong *state);
void cwmcc_client_set_wm_state(Window win, gulong state);

void cwmcc_client_get_name(Window win, char **name);

void cwmcc_client_get_icon_name(Window win, char **name);

void cwmcc_client_get_class(Window win, char **class, char **name);

/*! Possible flags for MWM Hints (defined by Motif 2.0) */
enum Cwmcc_MwmFlags {
    Cwmcc_MwmFlag_Functions   = 1 << 0, /*!< The Hints define functions */
    Cwmcc_MwmFlag_Decorations = 1 << 1  /*!< The Hints define decorations */
};

/*! Possible functions for MWM Hints (defined by Motif 2.0) */
enum Cwmcc_MwmFunctions {
    Cwmcc_MwmFunc_All      = 1 << 0, /*!< All functions */
    Cwmcc_MwmFunc_Resize   = 1 << 1, /*!< Allow resizing */
    Cwmcc_MwmFunc_Move     = 1 << 2, /*!< Allow moving */
    Cwmcc_MwmFunc_Iconify  = 1 << 3, /*!< Allow to be iconfied */
    Cwmcc_MwmFunc_Maximize = 1 << 4  /*!< Allow to be maximized */
    /*MwmFunc_Close    = 1 << 5 /!< Allow to be closed */
};

/*! Possible decorations for MWM Hints (defined by Motif 2.0) */
enum Cwmcc_MwmDecorations {
    Cwmcc_MwmDecor_All      = 1 << 0, /*!< All decorations */
    Cwmcc_MwmDecor_Border   = 1 << 1, /*!< Show a border */
    Cwmcc_MwmDecor_Handle   = 1 << 2, /*!< Show a handle (bottom) */
    Cwmcc_MwmDecor_Title    = 1 << 3, /*!< Show a titlebar */
    Cwmcc_MwmDecor_Menu     = 1 << 4, /*!< Show a menu */
    Cwmcc_MwmDecor_Iconify  = 1 << 5, /*!< Show an iconify button */
    Cwmcc_MwmDecor_Maximize = 1 << 6  /*!< Show a maximize button */
};

/*! The MWM Hints as retrieved from the window property
  This structure only contains 3 elements, even though the Motif 2.0
  structure contains 5. We only use the first 3, so that is all gets
  defined.
*/
struct Cwmcc_MwmHints {
    /*! A bitmask of Cwmcc_MwmFlags values */
    gulong flags;
    /*! A bitmask of Cwmcc_MwmFunctions values */
    gulong functions;
    /*! A bitmask of Cwmcc_MwmDecorations values */
    gulong decorations;
};

void cwmcc_client_get_mwmhints(Window win, struct Cwmcc_MwmHints *hints);

void cwmcc_client_get_desktop(Window win, gulong *desk);
void cwmcc_client_set_desktop(Window win, gulong desk);

void cwmcc_client_get_type(Window win, gulong **types);
void cwmcc_client_set_type(Window win, gulong *types);

void cwmcc_client_get_state(Window win, gulong **states);
void cwmcc_client_set_state(Window win, gulong *states);

void cwmcc_client_get_strut(Window win, int *l, int *t, int *r, int *b);

/*! Holds an icon in ARGB format */
struct Cwmcc_Icon {
    gulong width, height;
    gulong *data;
};

/* Returns an array of Cwms_Icons. The array is terminated by a Cwmcc_Icon with
   its data member set to NULL */
void cwmcc_client_get_icon(Window win, struct Cwmcc_Icon **icons);

void cwmcc_client_get_premax(Window win, int *x, int *y, int *w, int *h);
void cwmcc_client_set_premax(Window win, int x, int y, int w, int h);

#endif
