//
// Created by wbull on 5/24/20.
//

#pragma once

class WView;

class Ascii
{
public:
    /* TODO: find a better name. This is not actually a "state machine",
     * but a "state machine's state", but that sounds silly.
     * Could be parser_state, formatter_state... */
    struct mcview_state_machine_t
    {
        off_t offset;               /* The file offset at which this is the state. */
        off_t unwrapped_column;     /* Columns if the paragraph wasn't wrapped, */
        /* used for positioning TABs in wrapped lines */
        gboolean nroff_underscore_is_underlined;    /* whether _\b_ is underlined rather than bold */
        gboolean print_lonely_combining;    /* whether lonely combining marks are printed on a dotted circle */
    };

public:
    /**
     * In both wrap and unwrap modes, dpy_start points to the beginning of the paragraph.
     *
     * In unwrap mode, start displaying from this position, probably applying an additional horizontal
     * scroll.
     *
     * In wrap mode, an additional dpy_paragraph_skip_lines lines are skipped from the top of this
     * paragraph. dpy_state_top contains the position and parser-formatter state corresponding to the
     * top left corner so we can just start rendering from here. Unless dpy_wrap_dirty is set in which
     * case dpy_state_top is invalid and we need to recompute first.
     */
    static void mcview_display_text(WView* view);

    static void mcview_state_machine_init(mcview_state_machine_t* state, off_t offset);

    /**
     * Move down.
     *
     * It's very simple. Just invisibly format the next "lines" lines, carefully carrying the formatter
     * state in wrap mode. But before each step we need to check if we've already hit the end of the
     * file, in that case we can no longer move. This is done by walking from dpy_state_bottom.
     *
     * Note that this relies on mcview_display_text() setting dpy_state_bottom to its correct value
     * upon rendering the screen contents. So don't call this function from other functions (e.g. at
     * the bottom of mcview_ascii_move_up()) which invalidate this value.
     */
    static void mcview_ascii_move_down(WView* view, off_t lines);

    /**
     * Move up.
     *
     * Unwrap mode: Piece of cake. Wrap mode: If we'd walk back more than the current line offset
     * within the paragraph, we need to jump back to the previous paragraph and compute its height to
     * see if we start from that paragraph, and repeat this if necessary. Once we're within the desired
     * paragraph, we still need to format it from its beginning to know the state.
     *
     * See the top of this file for comments about MAX_BACKWARDS_WALK_IN_PARAGRAPH.
     *
     * force_max is a nice protection against the rare extreme case that the file underneath us
     * changes, we don't want to endlessly consume a file of maybe full of zeros upon moving upwards.
     */
    static void mcview_ascii_move_up(WView* view, off_t lines);

    static void mcview_ascii_moveto_bol(WView* view);

    static void mcview_ascii_moveto_eol(WView* view);

private:
    /**
     * Recompute dpy_state_top from dpy_start and dpy_paragraph_skip_lines. Clamp
     * dpy_paragraph_skip_lines if necessary.
     *
     * This method should be called in wrap mode after changing one of the parsing or formatting
     * properties (e.g. window width, encoding, nroff), or when switching to wrap mode from unwrap or
     * hex.
     *
     * If we stayed within the same paragraph then try to keep the vertical offset within that
     * paragraph as well. It might happen though that the paragraph became shorter than our desired
     * vertical position, in that case move to its last row.
     */
    static void mcview_wrap_fixup(WView* view);

    /**
     * Parse, format and possibly display one paragraph (perhaps not from the beginning).
     *
     * Formatting starts at the given "state" (which encodes the file offset and parser and formatter's
     * internal state). In unwrap mode, this should point to the beginning of the paragraph with the
     * default state, the additional horizontal scrolling is added here. In wrap mode, this may point
     * to the beginning of the line within a paragraph (to display the partial paragraph at the top),
     * with the proper state at that point.
     *
     * Displaying the next paragraph should start at "state"'s new value, or if we displayed the bottom
     * line then state->offset tells the file offset to be shown in the top bar.
     *
     * If "row" is negative, don't display the first abs(row) lines and display the rest from the top.
     * This was a nice idea but it's now unused :)
     *
     * If "row" is too large, don't display the paragraph at all but still return the number of lines.
     * This is used when moving upwards.
     *
     * @param view ...
     * @param state the parser-formatter state machine's state, updated
     * @param row print starting at this row
     * @return the number of rows the paragraphs is wrapped to, that is, 0 if we were already at EOF,
     *   otherwise 1 in unwrap mode, >= 1 in wrap mode. We stop when reaching the bottom of the
     *   viewport, it's not counted how many more lines the paragraph would occupy
     */
    static int mcview_display_paragraph(WView* view, mcview_state_machine_t* state, int row);

    /**
     * Parse, format and possibly display one visual line of text.
     *
     * Formatting starts at the given "state" (which encodes the file offset and parser and formatter's
     * internal state). In unwrap mode, this should point to the beginning of the paragraph with the
     * default state, the additional horizontal scrolling is added here. In wrap mode, this should
     * point to the beginning of the line, with the proper state at that point.
     *
     * In wrap mode, if a line ends in a newline, it is consumed, even if it's exactly at the right
     * edge. In unwrap mode, the whole remaining line, including the newline is consumed. Displaying
     * the next line should start at "state"'s new value, or if we displayed the bottom line then
     * state->offset tells the file offset to be shown in the top bar.
     *
     * If "row" is offscreen, don't actually display the line but still update "state" and return the
     * proper value. This is used by mcview_wrap_move_down to advance in the file.
     *
     * @param view ...
     * @param state the parser-formatter state machine's state, updated
     * @param row print to this row
     * @param paragraph_ended store TRUE if paragraph ended by newline or EOF, FALSE if wraps to next
     *   line
     * @param linewidth store the width of the line here
     * @return the number of rows, that is, 0 if we were already at EOF, otherwise 1
     */
    static int mcview_display_line(WView* view, mcview_state_machine_t* state, int row, gboolean* paragraph_ended, off_t* linewidth);

    /**
     * Get one base character, along with its combining or spacing mark characters.
     *
     * (A spacing mark is a character that extends the base character's width 1 into a combined
     * character of width 2, yet these two character cells should not be separated. E.g. Devanagari
     * <U+0939><U+094B>.)
     *
     * This method exists mainly for two reasons. One is to be able to tell if we fit on the current
     * line or need to wrap to the next one. The other is that both slang and ncurses seem to require
     * that the character and its combining marks are printed in a single call (or is it just a
     * limitation of mc's wrapper to them?).
     *
     * For convenience, this method takes care of converting CR or CR+LF into LF.
     * TODO this should probably happen later, when displaying the file?
     *
     * Normally: stores cs and color, updates state, returns >= 1 (entries in cs).
     * At EOF: state is unchanged, cs and color are undefined, returns 0.
     *
     * @param view ...
     * @param state the parser-formatter state machine's state, updated
     * @param cs store the characters here
     * @param clen the room available in cs (that is, at most clen-1 combining marks are allowed), must
     *   be at least 2
     * @param color if non-NULL, store the color here, taken from the first codepoint's color
     * @return the number of entries placed in cs, or 0 on EOF
     */
    static int mcview_next_combining_char_sequence(WView* view, mcview_state_machine_t* state, int* cs, int clen, int* color);

    /**
     * This function parses the next nroff character and gives it to you along with its desired color,
     * so you never have to care about nroff again.
     *
     * The nroff mode does the backspace trick for every single character (Unicode codepoint). At least
     * that's what the GNU groff 1.22 package produces, and that's what less 458 expects. For
     * double-wide characters (CJK), still only a single backspace is emitted. For combining accents
     * and such, the print-backspace-print step is repeated for the base character and then for each
     * accent separately.
     *
     * So, the right place for this layer is after the bytes are interpreted in UTF-8, but before
     * joining a base character with its combining accents.
     *
     * Normally: stores c and color, updates state, returns TRUE.
     * At EOF: state is unchanged, c and color are undefined, returns FALSE.
     *
     * color can be null if the caller doesn't care.
     */
    static gboolean mcview_get_next_maybe_nroff_char(WView* view, mcview_state_machine_t* state, int* c, int* color);

    /**
     * Just for convenience, a common interface in front of mcview_get_utf and mcview_get_byte, so that
     * the caller doesn't have to care about utf8 vs 8-bit modes.
     *
     * Normally: stores c, updates state, returns TRUE.
     * At EOF: state is unchanged, c is undefined, returns FALSE.
     *
     * Just as with mcview_get_utf(), invalid UTF-8 is reported using negative integers.
     *
     * Also, temporary hack: handle force_max here.
     * TODO: move it to lower layers (datasource.c)?
     */
    static gboolean mcview_get_next_char(WView* view, mcview_state_machine_t* state, int* c);

    static int mcview_char_display(const WView* view, int c, char* s);

    static gboolean mcview_isprint(const WView* view, int c);

    /* actually is_non_spacing_mark_or_enclosing_mark */
    static gboolean mcview_is_non_spacing_mark(const WView* view, int c);

    static gboolean mcview_ismark(const WView* view, int c);

    /* TODO: These methods shouldn't be necessary, see ticket 3257 */
    static int mcview_wcwidth(const WView* view, int c);

private:
    /*
     * Wrap mode: This is for safety so that jumping to the end of file (which already includes
     * scrolling back by a page) and then walking backwards is reasonably fast, even if the file is
     * extremely large and consists of maybe full zeros or something like that. If there's no newline
     * found within this limit, just start displaying from there and see what happens. We might get
     * some displaying parameteres (most importantly the columns) incorrect, but at least will show the
     * file without spinning the CPU for ages. When scrolling back to that point, the user might see a
     * garbled first line (even starting with an invalid partial UTF-8), but then walking back by yet
     * another line should fix it.
     *
     * Unwrap mode: This is not used, we wouldn't be able to do anything reasonable without walking
     * back a whole paragraph (well, view->data_area.height paragraphs actually).
     */
    static constexpr int MAX_BACKWARDS_WALK_IN_PARAGRAPH = (100 * 1000);

    /* I think anything other than space (e.g. arrows) just introduce visual clutter without actually
    * adding value. */
    static constexpr char PARTIAL_CJK_AT_LEFT_MARGIN = ' ';
    static constexpr char PARTIAL_CJK_AT_RIGHT_MARGIN = ' ';

    /* The Unicode standard recommends that lonely combining characters are printed over a dotted
    * circle. If the terminal is not UTF-8, this will be replaced by a dot anyway. */
    static constexpr int BASE_CHARACTER_FOR_LONELY_COMBINING = 0x25CC;      /* dotted circle */
    static constexpr int MAX_COMBINING_CHARS = 4;   /* both slang and ncurses support exactly 4 */
};
