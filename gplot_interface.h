/*
    GNU PLOT INTERFACE

    Author: Guilherme Arruda

    Header repository : https://github.com/ohananoshi/gplot_interface

    Created in: 08/04/2023

    Last updated: 20/11/2026

    matplotlib-like plotting for C on top of gnuplot (>= 5.4), in one header.

    ---------------------------------------------------------------------------
    USAGE
    ---------------------------------------------------------------------------
    In exactly ONE .c file of the program:

        #define GPLOT_IMPLEMENTATION
        #include "gplot_interface.h"

    Every other file only includes the header. A gp_graph holds the whole
    context of one graph and every operation goes through the Graph table:

        gp_graph* g = Graph.new();
        Graph.title(g, "Damped oscillation");
        Graph.xlabel(g, "t [s]");
        Graph.plot(g, t, y, n, "r--o", "measured");
        Graph.scatter(g, t, y2, n, "b^ ms=4", "model");
        Graph.legend(g, "upper right");
        Graph.savefig(g, "oscillation.png");
        Graph.show(g);
        Graph.free(g);

    Nothing is drawn until show/savefig: they build a complete gnuplot script
    from the graph, so the same graph can be shown and saved many times.

    ---------------------------------------------------------------------------
    FORMAT STRINGS (plot, scatter, fplot)
    ---------------------------------------------------------------------------
    matplotlib codes "[marker][line][color]", in any order:
        colors   b g r c m y k w, C0..C9 (default color cycle)
        lines    -  solid   --  dashed   -.  dash-dot   :  dotted
        markers  . point  , pixel  o circle  v ^ triangles  s square
                 D d diamonds  p pentagon  * star  + plus  x cross
    A token that is only a color is also accepted: "green", "#1f77b4", "tab:red".
    Then optional key=value options separated by spaces:
        color=<color>  lw=<line width>  ms=<marker size>  alpha=<0..1>
        ls=<- | -- | -. | : | none>  marker=<code | none>
    Example: "r--o lw=2 alpha=0.6". Numbers always use '.' as decimal point.
    A marker without a line style draws only markers; NULL or "" is a solid line.
    Labels that are NULL, "" or start with '_' are left out of the legend.

    ---------------------------------------------------------------------------
    CONFIGURATION (define before including, in the implementation file)
    ---------------------------------------------------------------------------
        GPLOT_GNUPLOT_CMD   command that starts gnuplot (default "gnuplot");
                            the GPLOT_GNUPLOT environment variable overrides it.
        GPLOT_MALLOC, GPLOT_REALLOC, GPLOT_FREE   allocator hooks.
        GPLOT_DEBUG         print every error to stderr.

    Strings given to fplot, terminal and cmd are raw gnuplot code, and gnuplot
    can run shell commands: never pass untrusted input to them. Titles, labels,
    legend entries and file names are escaped and safe.
*/

#ifndef GPLOT_INTERFACE_H
#define GPLOT_INTERFACE_H

//============================= HEADERS =========================================

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#define GPLOT_VERSION "1.0.0"

//=============================== DATATYPES ========================================

enum GP_ERRORS{
    GP_OK          =  0,
    GP_ERR_ARG     = -1,    // invalid argument (NULL graph or data, n == 0, unknown option)
    GP_ERR_NOMEM   = -2,    // out of memory
    GP_ERR_FMT     = -3,    // invalid format string
    GP_ERR_FORMAT  = -4,    // savefig: unknown file extension
    GP_ERR_SPAWN   = -5,    // gnuplot could not be started (is it installed?)
    GP_ERR_GNUPLOT = -6,    // gnuplot reported an error
    GP_ERR_IO      = -7     // writing the script or the file failed
};

// The whole context of one graph. Opaque: use the Graph methods.
typedef struct gp_graph gp_graph;

// Method table. Methods take the graph first and return GP_OK or a GP_ERR_* code.
// The first error is also kept in the graph (Graph.error), so checking once at the end is enough.
// Calling a method with a NULL graph does nothing and returns GP_ERR_ARG.
typedef struct{
    //---- lifecycle
    gp_graph* (*new)(void);                 // NULL when out of memory
    void      (*free)(gp_graph* g);
    int       (*clear)(gp_graph* g);        // removes every series, keeps title, labels and limits

    //---- data: arrays are copied, so they can be released right after the call
    // x == NULL plots y against 0, 1, ..., n-1.
    int (*plot)(gp_graph* g, const double* x, const double* y, size_t n, const char* fmt, const char* label);
    // Markers only; the default marker is 'o'.
    int (*scatter)(gp_graph* g, const double* x, const double* y, size_t n, const char* fmt, const char* label);
    // gnuplot expression in x, e.g. "sin(x)/x". Without xlim gnuplot samples [-10:10].
    int (*fplot)(gp_graph* g, const char* expr, const char* fmt, const char* label);

    //---- decoration (NULL text removes it)
    int (*title)(gp_graph* g, const char* text);
    int (*xlabel)(gp_graph* g, const char* text);
    int (*ylabel)(gp_graph* g, const char* text);
    int (*xlim)(gp_graph* g, double lo, double hi);     // NAN on a side keeps it automatic
    int (*ylim)(gp_graph* g, double lo, double hi);
    int (*xscale)(gp_graph* g, const char* scale);      // "linear" or "log"
    int (*yscale)(gp_graph* g, const char* scale);
    int (*grid)(gp_graph* g, bool on);
    // "best" (NULL), "upper right", "upper left", "lower left", "lower right", "right",
    // "center left", "center right", "lower center", "upper center", "center",
    // "outside", or "off". Without a call the legend is hidden, as in matplotlib.
    int (*legend)(gp_graph* g, const char* loc);
    int (*size)(gp_graph* g, int width, int height);    // pixels, default 640 x 480

    //---- gnuplot specifics
    // Terminal used by show, e.g. "qt", "wxt", "x11" or "dumb" (ASCII art). NULL = gnuplot default.
    int (*terminal)(gp_graph* g, const char* name);
    // Raw gnuplot command added before the plot, for anything not wrapped here.
    int (*cmd)(gp_graph* g, const char* command);

    //---- output
    // Opens a window and blocks until it is closed, like plt.show().
    // Without a display (Linux, no DISPLAY/WAYLAND_DISPLAY) it prints ASCII art instead.
    int (*show)(gp_graph* g);
    // Terminal chosen by the extension: .png .jpg .jpeg .gif .svg .pdf .eps .txt
    int (*savefig)(gp_graph* g, const char* path);
    // Standalone gnuplot script with the data inside; run with "gnuplot -persist file.gp".
    int (*save_script)(gp_graph* g, const char* path);
    // Writes the script to a stream. terminal/output are optional (NULL to omit).
    int (*render)(gp_graph* g, FILE* out, const char* terminal, const char* output);

    //---- errors
    int         (*error)(const gp_graph* g);    // first error recorded, GP_OK if none
    const char* (*strerror)(int err);
}gp_graph_class;

extern const gp_graph_class Graph;

#endif // GPLOT_INTERFACE_H

//============================== IMPLEMENTATION ======================================

#ifdef GPLOT_IMPLEMENTATION
#ifndef GPLOT_IMPLEMENTATION_INCLUDED
#define GPLOT_IMPLEMENTATION_INCLUDED

#include <locale.h>
#include <math.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef GPLOT_GNUPLOT_CMD
#define GPLOT_GNUPLOT_CMD "gnuplot"
#endif

#ifndef GPLOT_MALLOC
#define GPLOT_MALLOC(size)          malloc(size)
#define GPLOT_REALLOC(ptr, size)    realloc(ptr, size)
#define GPLOT_FREE(ptr)             free(ptr)
#endif

#ifdef _WIN32
#define gp__popen(command)  _popen(command, "w")
#define gp__pclose(pipe)    _pclose(pipe)
#else
// Declared here so the header also builds with strict -std=c99 (no POSIX feature macros).
extern FILE* popen(const char* command, const char* mode);
extern int pclose(FILE* stream);
#define gp__popen(command)  popen(command, "w")
#define gp__pclose(pipe)    pclose(pipe)
#endif

#define GP__DEFAULT_WIDTH   640
#define GP__DEFAULT_HEIGHT  480
#define GP__DEFAULT_LW      1.5     // matplotlib lines.linewidth
#define GP__DEFAULT_MS      6.0     // matplotlib lines.markersize
#define GP__LW_SCALE        1.4     // gnuplot lw for 1 matplotlib point (cairo terminals)
#define GP__PS_SCALE        (1.2 / 6.0)     // gnuplot ps for 1 matplotlib point of marker size
#define GP__TOKEN_MAX       64
#define GP__MARKER_NONE     (-1)

enum GP__LINESTYLES{
    GP__LS_UNSET = 0,
    GP__LS_NONE,
    GP__LS_SOLID,
    GP__LS_DASHED,
    GP__LS_DASHDOT,
    GP__LS_DOTTED
};

enum GP__SERIES_KINDS{
    GP__KIND_PLOT = 0,
    GP__KIND_SCATTER,
    GP__KIND_FPLOT
};

// matplotlib "tab10" property cycle
static const uint32_t GP__COLOR_CYCLE[10] = {0x1f77b4, 0xff7f0e, 0x2ca02c, 0xd62728, 0x9467bd,
                                             0x8c564b, 0xe377c2, 0x7f7f7f, 0xbcbd22, 0x17becf};

static const char GP__COLOR_CODES[] = "bgrcmykw";
static const uint32_t GP__COLOR_CODE_RGB[] = {0x0000ff, 0x008000, 0xff0000, 0x00bfbf,
                                              0xbf00bf, 0xbfbf00, 0x000000, 0xffffff};

static const struct{ const char* name; uint32_t rgb; } GP__COLOR_NAMES[] = {
    {"blue", 0x0000ff}, {"green", 0x008000}, {"red", 0xff0000}, {"cyan", 0x00ffff},
    {"magenta", 0xff00ff}, {"yellow", 0xffff00}, {"black", 0x000000}, {"white", 0xffffff},
    {"gray", 0x808080}, {"grey", 0x808080}, {"orange", 0xffa500}, {"purple", 0x800080},
    {"brown", 0xa52a2a}, {"pink", 0xffc0cb}, {"olive", 0x808000}, {"navy", 0x000080},
    {"teal", 0x008080}, {"lime", 0x00ff00}, {"maroon", 0x800000}, {"gold", 0xffd700},
    {"silver", 0xc0c0c0}, {"indigo", 0x4b0082}, {"violet", 0xee82ee}, {"turquoise", 0x40e0d0},
    {"coral", 0xff7f50}, {"salmon", 0xfa8072}, {"crimson", 0xdc143c}, {"tomato", 0xff6347},
    {"chocolate", 0xd2691e}, {"tan", 0xd2b48c}, {"khaki", 0xf0e68c}, {"skyblue", 0x87ceeb},
    {"steelblue", 0x4682b4}, {"royalblue", 0x4169e1}, {"forestgreen", 0x228b22}, {"seagreen", 0x2e8b57},
    {"darkblue", 0x00008b}, {"darkgreen", 0x006400}, {"darkred", 0x8b0000}, {"darkorange", 0xff8c00},
    {"darkgray", 0xa9a9a9}, {"darkgrey", 0xa9a9a9}, {"lightblue", 0xadd8e6}, {"lightgreen", 0x90ee90},
    {"lightgray", 0xd3d3d3}, {"lightgrey", 0xd3d3d3},
    {"tab:blue", 0x1f77b4}, {"tab:orange", 0xff7f0e}, {"tab:green", 0x2ca02c}, {"tab:red", 0xd62728},
    {"tab:purple", 0x9467bd}, {"tab:brown", 0x8c564b}, {"tab:pink", 0xe377c2}, {"tab:gray", 0x7f7f7f},
    {"tab:grey", 0x7f7f7f}, {"tab:olive", 0xbcbd22}, {"tab:cyan", 0x17becf}
};

// matplotlib marker code -> gnuplot point type and size factor (same order in both tables)
static const char GP__MARKER_CODES[] = ".,ov^sDdp*+x";
static const struct{ int point_type; double scale; } GP__MARKER_POINTS[] = {
    {7, 0.4}, {0, 1.0}, {7, 1.0}, {11, 1.0}, {9, 1.0}, {5, 1.0},
    {13, 1.0}, {13, 0.75}, {15, 1.0}, {3, 1.0}, {1, 1.0}, {2, 1.0}
};

// indexed by GP__LINESTYLES; gnuplot scales the lengths with the line width, like matplotlib
static const char* GP__DASH_TYPES[] = {"", "", " dt solid", " dt (11,5)", " dt (16,5,3,5)", " dt (3,5)"};

static const struct{ const char* loc; const char* key; } GP__LEGEND_LOCS[] = {
    {"best", "inside right top"}, {"upper right", "inside right top"},
    {"upper left", "inside left top"}, {"lower left", "inside left bottom"},
    {"lower right", "inside right bottom"}, {"right", "inside right center"},
    {"center left", "inside left center"}, {"center right", "inside right center"},
    {"lower center", "inside center bottom"}, {"upper center", "inside center top"},
    {"center", "inside center center"},
    {"outside", "outside right top offset -1,0"}    // the offset keeps the box's right edge on the canvas
};

typedef struct{
    bool has_color;
    uint32_t rgb;           // 0xRRGGBB
    double alpha;           // 1 = opaque
    int linestyle;          // GP__LINESTYLES
    int marker;             // 0 = unset, GP__MARKER_NONE, or a char of GP__MARKER_CODES
    double linewidth;       // 0 = default
    double markersize;      // 0 = default
}gp__style;

typedef struct{
    double* xy;             // n x values followed by n y values; NULL for expressions
    size_t n;
    char* expr;
    char* label;            // NULL = left out of the legend
    gp__style style;        // fully resolved when the series is added
    bool lines;
    bool points;
}gp__series;

struct gp_graph{
    char* title;
    char* xlabel;
    char* ylabel;
    double xlim[2];         // NAN = automatic
    double ylim[2];
    bool xlog;
    bool ylog;
    bool grid;
    const char* legend;     // gnuplot key placement, NULL = no legend
    int width;
    int height;
    char* terminal;

    char** cmds;
    size_t cmd_count;

    gp__series* series;
    size_t series_count;
    size_t color_index;

    int error;
};

//================================ HELPERS ===============================================

static const char* gp__strerror(int err){

    switch(err){
    case GP_OK:          return "No error.";
    case GP_ERR_ARG:     return "Invalid argument.";
    case GP_ERR_NOMEM:   return "Out of memory.";
    case GP_ERR_FMT:     return "Invalid format string.";
    case GP_ERR_FORMAT:  return "Unknown output file extension.";
    case GP_ERR_SPAWN:   return "Unable to start gnuplot.";
    case GP_ERR_GNUPLOT: return "gnuplot reported an error.";
    case GP_ERR_IO:      return "Could not write the output.";
    default:             return "Unknown error.";
    }
}

static int gp__fail(gp_graph* g, int err){

    if(g != NULL && g->error == GP_OK) g->error = err;

#ifdef GPLOT_DEBUG
    fprintf(stderr, "gplot: %s\n", gp__strerror(err));
#endif

    return err;
}

static char* gp__strdup(const char* s){

    size_t size = strlen(s) + 1;
    char* copy = (char*)GPLOT_MALLOC(size);

    if(copy != NULL) memcpy(copy, s, size);

    return copy;
}

static char gp__lower(char c){
    return (c >= 'A' && c <= 'Z') ? (char)(c - 'A' + 'a') : c;
}

static bool gp__streq_nocase(const char* a, const char* b){

    while(*a != '\0' && gp__lower(*a) == gp__lower(*b)){
        a++;
        b++;
    }

    return gp__lower(*a) == gp__lower(*b);
}

// The C library formats and parses numbers with the locale's decimal point;
// gnuplot and format strings always use '.'.
static char gp__decimal_point(void){

    const struct lconv* lc = localeconv();

    return (lc != NULL && lc->decimal_point != NULL && lc->decimal_point[0] != '\0') ? lc->decimal_point[0] : '.';
}

static void gp__format_number(char* buffer, size_t size, const char* spec, double value, char decimal_point){

    char* c;

    snprintf(buffer, size, spec, value);

    if(decimal_point != '.'){
        for(c = buffer; *c != '\0'; c++){
            if(*c == decimal_point) *c = '.';
        }
    }
}

static bool gp__parse_number(const char* text, double* value){

    char buffer[GP__TOKEN_MAX];
    char decimal_point = gp__decimal_point();
    char* end;
    size_t i, length = strlen(text);

    if(length == 0 || length >= sizeof(buffer)) return false;

    for(i = 0; i <= length; i++){
        buffer[i] = (text[i] == '.') ? decimal_point : text[i];
    }

    *value = strtod(buffer, &end);

    return *end == '\0' && isfinite(*value);
}

static int gp__set_string(gp_graph* g, char** field, const char* text){

    char* copy = NULL;

    if(g == NULL) return GP_ERR_ARG;
    if(text != NULL && (copy = gp__strdup(text)) == NULL) return gp__fail(g, GP_ERR_NOMEM);

    GPLOT_FREE(*field);
    *field = copy;

    return GP_OK;
}

//================================ FORMAT STRINGS ===============================================

static int gp__hex_digit(char c){

    if(c >= '0' && c <= '9') return c - '0';
    if(c >= 'a' && c <= 'f') return c - 'a' + 10;
    if(c >= 'A' && c <= 'F') return c - 'A' + 10;

    return -1;
}

static bool gp__parse_color(const char* text, uint32_t* rgb){

    const char* code;
    size_t i;

    if(text[0] != '\0' && text[1] == '\0' && (code = strchr(GP__COLOR_CODES, text[0])) != NULL){
        *rgb = GP__COLOR_CODE_RGB[code - GP__COLOR_CODES];
        return true;
    }

    if(text[0] == 'C' && text[1] >= '0' && text[1] <= '9' && text[2] == '\0'){
        *rgb = GP__COLOR_CYCLE[text[1] - '0'];
        return true;
    }

    if(text[0] == '#' && strlen(text) == 7){
        uint32_t value = 0;

        for(i = 1; i < 7; i++){
            int digit = gp__hex_digit(text[i]);
            if(digit < 0) return false;
            value = value * 16 + (uint32_t)digit;
        }

        *rgb = value;
        return true;
    }

    for(i = 0; i < sizeof(GP__COLOR_NAMES) / sizeof(GP__COLOR_NAMES[0]); i++){
        if(gp__streq_nocase(text, GP__COLOR_NAMES[i].name)){
            *rgb = GP__COLOR_NAMES[i].rgb;
            return true;
        }
    }

    return false;
}

// matplotlib "[marker][line][color]" codes; each kind may appear once.
static bool gp__parse_codes(const char* token, gp__style* style){

    gp__style parsed = *style;
    bool has_line = false, has_marker = false, has_color = false;
    const char* code;
    size_t i = 0;

    while(token[i] != '\0'){
        char c = token[i];

        if(c == '-' || c == ':'){
            if(has_line) return false;
            has_line = true;

            if(c == ':')                   { parsed.linestyle = GP__LS_DOTTED;  i += 1; }
            else if(token[i + 1] == '-')   { parsed.linestyle = GP__LS_DASHED;  i += 2; }
            else if(token[i + 1] == '.')   { parsed.linestyle = GP__LS_DASHDOT; i += 2; }
            else                           { parsed.linestyle = GP__LS_SOLID;   i += 1; }
        }
        else if(strchr(GP__MARKER_CODES, c) != NULL){
            if(has_marker) return false;
            has_marker = true;
            parsed.marker = c;
            i += 1;
        }
        else if(c == 'C' && token[i + 1] >= '0' && token[i + 1] <= '9'){
            if(has_color) return false;
            has_color = true;
            parsed.has_color = true;
            parsed.rgb = GP__COLOR_CYCLE[token[i + 1] - '0'];
            i += 2;
        }
        else if((code = strchr(GP__COLOR_CODES, c)) != NULL){
            if(has_color) return false;
            has_color = true;
            parsed.has_color = true;
            parsed.rgb = GP__COLOR_CODE_RGB[code - GP__COLOR_CODES];
            i += 1;
        }
        else{
            return false;
        }
    }

    *style = parsed;

    return true;
}

static bool gp__parse_linestyle(const char* text, int* linestyle){

    if(!strcmp(text, "-")  || gp__streq_nocase(text, "solid"))   { *linestyle = GP__LS_SOLID;   return true; }
    if(!strcmp(text, "--") || gp__streq_nocase(text, "dashed"))  { *linestyle = GP__LS_DASHED;  return true; }
    if(!strcmp(text, "-.") || gp__streq_nocase(text, "dashdot")) { *linestyle = GP__LS_DASHDOT; return true; }
    if(!strcmp(text, ":")  || gp__streq_nocase(text, "dotted"))  { *linestyle = GP__LS_DOTTED;  return true; }
    if(gp__streq_nocase(text, "none"))                           { *linestyle = GP__LS_NONE;    return true; }

    return false;
}

// key=value option; the token is modified.
static bool gp__parse_option(char* token, gp__style* style){

    char* value = strchr(token, '=');
    const char* key = token;
    double number;

    *value++ = '\0';

    if(!strcmp(key, "color") || !strcmp(key, "c")){
        if(!gp__parse_color(value, &style->rgb)) return false;
        style->has_color = true;
        return true;
    }

    if(!strcmp(key, "lw") || !strcmp(key, "linewidth")){
        if(!gp__parse_number(value, &number) || number <= 0) return false;
        style->linewidth = number;
        return true;
    }

    if(!strcmp(key, "ms") || !strcmp(key, "markersize")){
        if(!gp__parse_number(value, &number) || number <= 0) return false;
        style->markersize = number;
        return true;
    }

    if(!strcmp(key, "alpha")){
        if(!gp__parse_number(value, &number) || number < 0 || number > 1) return false;
        style->alpha = number;
        return true;
    }

    if(!strcmp(key, "ls") || !strcmp(key, "linestyle")){
        return gp__parse_linestyle(value, &style->linestyle);
    }

    if(!strcmp(key, "marker")){
        if(gp__streq_nocase(value, "none")){
            style->marker = GP__MARKER_NONE;
            return true;
        }
        if(value[0] == '\0' || value[1] != '\0' || strchr(GP__MARKER_CODES, value[0]) == NULL) return false;
        style->marker = value[0];
        return true;
    }

    return false;
}

static int gp__parse_style(const char* fmt, gp__style* style){

    char token[GP__TOKEN_MAX];
    size_t length;
    uint32_t rgb;

    memset(style, 0, sizeof(*style));
    style->alpha = 1.0;

    if(fmt == NULL) return GP_OK;

    for(;;){
        while(*fmt == ' ' || *fmt == '\t') fmt++;

        length = strcspn(fmt, " \t");
        if(length == 0) break;
        if(length >= sizeof(token)) return GP_ERR_FMT;

        memcpy(token, fmt, length);
        token[length] = '\0';
        fmt += length;

        if(strchr(token, '=') != NULL){
            if(!gp__parse_option(token, style)) return GP_ERR_FMT;
        }
        else if(!gp__parse_codes(token, style)){
            // like matplotlib, a token that is only a color: "green", "#008000", "tab:red"
            if(!gp__parse_color(token, &rgb)) return GP_ERR_FMT;
            style->has_color = true;
            style->rgb = rgb;
        }
    }

    return GP_OK;
}

//================================ SERIES ===============================================

static void gp__series_free(gp__series* series){

    GPLOT_FREE(series->xy);
    GPLOT_FREE(series->expr);
    GPLOT_FREE(series->label);
}

// Parses fmt, decides between lines and markers and copies the label.
static int gp__prepare_series(gp__series* series, const char* fmt, const char* label, int kind){

    gp__style* style = &series->style;
    int err = gp__parse_style(fmt, style);

    if(err != GP_OK) return err;

    if(kind == GP__KIND_SCATTER){
        if(style->marker == GP__MARKER_NONE) return GP_ERR_FMT;
        if(style->marker == 0) style->marker = 'o';
        series->lines = false;
        series->points = true;
    }else{
        series->points = style->marker > 0;
        series->lines = (style->linestyle == GP__LS_UNSET) ? !series->points : (style->linestyle != GP__LS_NONE);
        if(!series->lines && !series->points) return GP_ERR_FMT;
        if(series->lines && style->linestyle == GP__LS_UNSET) style->linestyle = GP__LS_SOLID;
    }

    if(style->linewidth == 0) style->linewidth = GP__DEFAULT_LW;
    if(style->markersize == 0) style->markersize = GP__DEFAULT_MS;

    if(label != NULL && label[0] != '\0' && label[0] != '_'){
        if((series->label = gp__strdup(label)) == NULL) return GP_ERR_NOMEM;
    }

    return GP_OK;
}

// Appends the series (taking ownership) and gives it the next cycle color if it has none.
static int gp__push_series(gp_graph* g, gp__series* series){

    gp__series* grown = (gp__series*)GPLOT_REALLOC(g->series, (g->series_count + 1) * sizeof(gp__series));

    if(grown == NULL) return GP_ERR_NOMEM;
    g->series = grown;

    if(!series->style.has_color){
        series->style.has_color = true;
        series->style.rgb = GP__COLOR_CYCLE[g->color_index % 10];
        g->color_index++;
    }

    g->series[g->series_count++] = *series;

    return GP_OK;
}

static int gp__add_data(gp_graph* g, const double* x, const double* y, size_t n, const char* fmt, const char* label, int kind){

    gp__series series;
    size_t i;
    int err;

    if(g == NULL) return GP_ERR_ARG;
    if(y == NULL || n == 0) return gp__fail(g, GP_ERR_ARG);
    if(n > SIZE_MAX / (2 * sizeof(double))) return gp__fail(g, GP_ERR_NOMEM);

    memset(&series, 0, sizeof(series));

    err = gp__prepare_series(&series, fmt, label, kind);

    if(err == GP_OK){
        series.xy = (double*)GPLOT_MALLOC(2 * n * sizeof(double));
        if(series.xy == NULL) err = GP_ERR_NOMEM;
    }

    if(err == GP_OK){
        for(i = 0; i < n; i++){
            series.xy[i] = (x != NULL) ? x[i] : (double)i;
            series.xy[n + i] = y[i];
        }
        series.n = n;
        err = gp__push_series(g, &series);
    }

    if(err != GP_OK){
        gp__series_free(&series);
        return gp__fail(g, err);
    }

    return GP_OK;
}

//================================ GRAPH METHODS ===============================================

static gp_graph* gp__new(void){

    gp_graph* g = (gp_graph*)GPLOT_MALLOC(sizeof(gp_graph));

    if(g == NULL) return NULL;

    memset(g, 0, sizeof(*g));
    g->xlim[0] = g->xlim[1] = NAN;
    g->ylim[0] = g->ylim[1] = NAN;
    g->width = GP__DEFAULT_WIDTH;
    g->height = GP__DEFAULT_HEIGHT;

    return g;
}

static int gp__clear(gp_graph* g){

    size_t i;

    if(g == NULL) return GP_ERR_ARG;

    for(i = 0; i < g->series_count; i++){
        gp__series_free(&g->series[i]);
    }

    GPLOT_FREE(g->series);
    g->series = NULL;
    g->series_count = 0;
    g->color_index = 0;

    return GP_OK;
}

static void gp__free(gp_graph* g){

    size_t i;

    if(g == NULL) return;

    gp__clear(g);

    for(i = 0; i < g->cmd_count; i++){
        GPLOT_FREE(g->cmds[i]);
    }

    GPLOT_FREE(g->cmds);
    GPLOT_FREE(g->title);
    GPLOT_FREE(g->xlabel);
    GPLOT_FREE(g->ylabel);
    GPLOT_FREE(g->terminal);
    GPLOT_FREE(g);
}

static int gp__plot(gp_graph* g, const double* x, const double* y, size_t n, const char* fmt, const char* label){
    return gp__add_data(g, x, y, n, fmt, label, GP__KIND_PLOT);
}

static int gp__scatter(gp_graph* g, const double* x, const double* y, size_t n, const char* fmt, const char* label){
    return gp__add_data(g, x, y, n, fmt, label, GP__KIND_SCATTER);
}

static int gp__fplot(gp_graph* g, const char* expr, const char* fmt, const char* label){

    gp__series series;
    int err;

    if(g == NULL) return GP_ERR_ARG;
    if(expr == NULL || expr[0] == '\0' || strpbrk(expr, "\r\n") != NULL) return gp__fail(g, GP_ERR_ARG);

    memset(&series, 0, sizeof(series));

    err = gp__prepare_series(&series, fmt, label, GP__KIND_FPLOT);
    if(err == GP_OK && (series.expr = gp__strdup(expr)) == NULL) err = GP_ERR_NOMEM;
    if(err == GP_OK) err = gp__push_series(g, &series);

    if(err != GP_OK){
        gp__series_free(&series);
        return gp__fail(g, err);
    }

    return GP_OK;
}

static int gp__title(gp_graph* g, const char* text){
    return gp__set_string(g, g != NULL ? &g->title : NULL, text);
}

static int gp__xlabel(gp_graph* g, const char* text){
    return gp__set_string(g, g != NULL ? &g->xlabel : NULL, text);
}

static int gp__ylabel(gp_graph* g, const char* text){
    return gp__set_string(g, g != NULL ? &g->ylabel : NULL, text);
}

static int gp__set_limits(gp_graph* g, double* limits, double lo, double hi){

    if(g == NULL) return GP_ERR_ARG;
    if(isinf(lo) || isinf(hi) || lo == hi) return gp__fail(g, GP_ERR_ARG);

    limits[0] = lo;
    limits[1] = hi;

    return GP_OK;
}

static int gp__xlim(gp_graph* g, double lo, double hi){
    return gp__set_limits(g, g != NULL ? g->xlim : NULL, lo, hi);
}

static int gp__ylim(gp_graph* g, double lo, double hi){
    return gp__set_limits(g, g != NULL ? g->ylim : NULL, lo, hi);
}

static int gp__set_scale(gp_graph* g, bool* log_scale, const char* scale){

    if(g == NULL) return GP_ERR_ARG;

    if(scale != NULL && gp__streq_nocase(scale, "linear")) *log_scale = false;
    else if(scale != NULL && gp__streq_nocase(scale, "log")) *log_scale = true;
    else return gp__fail(g, GP_ERR_ARG);

    return GP_OK;
}

static int gp__xscale(gp_graph* g, const char* scale){
    return gp__set_scale(g, g != NULL ? &g->xlog : NULL, scale);
}

static int gp__yscale(gp_graph* g, const char* scale){
    return gp__set_scale(g, g != NULL ? &g->ylog : NULL, scale);
}

static int gp__grid(gp_graph* g, bool on){

    if(g == NULL) return GP_ERR_ARG;

    g->grid = on;

    return GP_OK;
}

static int gp__legend(gp_graph* g, const char* loc){

    size_t i;

    if(g == NULL) return GP_ERR_ARG;
    if(loc == NULL) loc = "best";

    if(gp__streq_nocase(loc, "off")){
        g->legend = NULL;
        return GP_OK;
    }

    for(i = 0; i < sizeof(GP__LEGEND_LOCS) / sizeof(GP__LEGEND_LOCS[0]); i++){
        if(gp__streq_nocase(loc, GP__LEGEND_LOCS[i].loc)){
            g->legend = GP__LEGEND_LOCS[i].key;
            return GP_OK;
        }
    }

    return gp__fail(g, GP_ERR_ARG);
}

static int gp__size(gp_graph* g, int width, int height){

    if(g == NULL) return GP_ERR_ARG;
    if(width <= 0 || height <= 0) return gp__fail(g, GP_ERR_ARG);

    g->width = width;
    g->height = height;

    return GP_OK;
}

static int gp__terminal(gp_graph* g, const char* name){
    return gp__set_string(g, g != NULL ? &g->terminal : NULL, (name != NULL && name[0] != '\0') ? name : NULL);
}

static int gp__cmd(gp_graph* g, const char* command){

    char** grown;
    char* copy;

    if(g == NULL) return GP_ERR_ARG;
    if(command == NULL) return gp__fail(g, GP_ERR_ARG);

    if((copy = gp__strdup(command)) == NULL) return gp__fail(g, GP_ERR_NOMEM);

    grown = (char**)GPLOT_REALLOC(g->cmds, (g->cmd_count + 1) * sizeof(char*));
    if(grown == NULL){
        GPLOT_FREE(copy);
        return gp__fail(g, GP_ERR_NOMEM);
    }

    g->cmds = grown;
    g->cmds[g->cmd_count++] = copy;

    return GP_OK;
}

static int gp__error(const gp_graph* g){
    return (g != NULL) ? g->error : GP_ERR_ARG;
}

//================================ SCRIPT GENERATION ===============================================

// gnuplot double-quoted string. Backquotes are escaped because gnuplot runs
// `command` substitutions even inside double quotes.
static void gp__put_string(FILE* out, const char* s){

    fputc('"', out);

    for(; *s != '\0'; s++){
        switch(*s){
        case '\\': fputs("\\\\", out); break;
        case '"':  fputs("\\\"", out); break;
        case '`':  fputs("\\140", out); break;
        case '\n': fputs("\\n", out); break;
        case '\t': fputs("\\t", out); break;
        case '\r': break;
        default:   fputc(*s, out);
        }
    }

    fputc('"', out);
}

static void gp__put_number(FILE* out, const char* spec, double value, char decimal_point){

    char buffer[64];

    if(!isfinite(value)){
        fputs("NaN", out);
        return;
    }

    gp__format_number(buffer, sizeof(buffer), spec, value, decimal_point);
    fputs(buffer, out);
}

static void gp__put_range(FILE* out, char axis, const double limits[2], char decimal_point){

    fprintf(out, "set %crange [", axis);

    if(isnan(limits[0])) fputc('*', out);
    else gp__put_number(out, "%.17g", limits[0], decimal_point);

    fputc(':', out);

    if(isnan(limits[1])) fputc('*', out);
    else gp__put_number(out, "%.17g", limits[1], decimal_point);

    fputs("]\n", out);
}

// An empty graph has nothing to autoscale on: fill the automatic sides like matplotlib's empty axes.
static void gp__empty_range(const double limits[2], bool log_scale, double range[2]){

    double lo = limits[0], hi = limits[1];

    if(isnan(lo)) lo = isnan(hi) ? (log_scale ? 1.0 : 0.0) : (log_scale ? hi / 10.0 : hi - 1.0);
    if(isnan(hi)) hi = log_scale ? lo * 10.0 : lo + 1.0;

    range[0] = lo;
    range[1] = hi;
}

static void gp__put_series(FILE* out, const gp__series* series, size_t index, char decimal_point){

    const gp__style* style = &series->style;
    unsigned transparency = (unsigned)((1.0 - style->alpha) * 255.0 + 0.5);

    if(series->xy != NULL) fprintf(out, "$gp%lu using 1:2", (unsigned long)index);
    else fputs(series->expr, out);

    if(series->lines && series->points) fputs(" with linespoints", out);
    else if(series->lines) fputs(" with lines", out);
    else fputs(" with points", out);

    if(transparency == 0) fprintf(out, " lc rgb \"#%06lx\"", (unsigned long)style->rgb);
    else fprintf(out, " lc rgb \"#%02x%06lx\"", transparency, (unsigned long)style->rgb);

    fputs(" lw ", out);
    gp__put_number(out, "%g", style->linewidth * GP__LW_SCALE, decimal_point);

    if(series->lines) fputs(GP__DASH_TYPES[style->linestyle], out);

    if(series->points){
        size_t marker = (size_t)(strchr(GP__MARKER_CODES, style->marker) - GP__MARKER_CODES);

        fprintf(out, " pt %d ps ", GP__MARKER_POINTS[marker].point_type);
        gp__put_number(out, "%g", style->markersize * GP__MARKER_POINTS[marker].scale * GP__PS_SCALE, decimal_point);
    }

    if(series->label != NULL){
        fputs(" title ", out);
        gp__put_string(out, series->label);
    }else{
        fputs(" notitle", out);
    }
}

static int gp__write_script(const gp_graph* g, FILE* out, const char* terminal, const char* output, bool interactive){

    char decimal_point = gp__decimal_point();
    bool has_expr = false;
    size_t i, j;

    fputs("reset\nset encoding utf8\n", out);

    if(terminal != NULL){
        fprintf(out, "set terminal %s\n", terminal);
    }else if(interactive){
        fprintf(out, "if (GPVAL_TERM eq \"qt\" || GPVAL_TERM eq \"wxt\" || GPVAL_TERM eq \"x11\") "
                     "{ eval sprintf(\"set terminal %%s size %d,%d\", GPVAL_TERM) }\n", g->width, g->height);
    }

    if(output != NULL){
        fputs("set output ", out);
        gp__put_string(out, output);
        fputc('\n', out);
    }

    // literal text like matplotlib ("x_1" is not a subscript) and a matplotlib-like look
    fputs("set termoption noenhanced\n"
          "set tics out nomirror\n"
          "set style line 100 lc rgb \"#b0b0b0\" lw 0.8 dt solid\n"
          "set style line 101 lc rgb \"#cccccc\" lw 1\n", out);

    if(g->title != NULL){
        fputs("set title ", out);
        gp__put_string(out, g->title);
        fputc('\n', out);
    }

    if(g->xlabel != NULL){
        fputs("set xlabel ", out);
        gp__put_string(out, g->xlabel);
        fputc('\n', out);
    }

    if(g->ylabel != NULL){
        fputs("set ylabel ", out);
        gp__put_string(out, g->ylabel);
        fputc('\n', out);
    }

    if(g->series_count == 0){
        double range[2];

        gp__empty_range(g->xlim, g->xlog, range);
        gp__put_range(out, 'x', range, decimal_point);
        gp__empty_range(g->ylim, g->ylog, range);
        gp__put_range(out, 'y', range, decimal_point);
    }else{
        if(!isnan(g->xlim[0]) || !isnan(g->xlim[1])) gp__put_range(out, 'x', g->xlim, decimal_point);
        if(!isnan(g->ylim[0]) || !isnan(g->ylim[1])) gp__put_range(out, 'y', g->ylim, decimal_point);
    }

    if(g->xlog) fputs("set logscale x\n", out);
    if(g->ylog) fputs("set logscale y\n", out);
    if(g->grid) fputs("set grid back ls 100\n", out);

    if(g->legend != NULL) fprintf(out, "set key %s reverse Left samplen 2 box ls 101 opaque\n", g->legend);
    else fputs("unset key\n", out);

    for(i = 0; i < g->cmd_count; i++){
        fputs(g->cmds[i], out);
        fputc('\n', out);
    }

    for(i = 0; i < g->series_count; i++){
        const gp__series* series = &g->series[i];

        if(series->xy == NULL){
            has_expr = true;
            continue;
        }

        fprintf(out, "$gp%lu << EOD\n", (unsigned long)i);
        for(j = 0; j < series->n; j++){
            gp__put_number(out, "%.17g", series->xy[j], decimal_point);
            fputc(' ', out);
            gp__put_number(out, "%.17g", series->xy[series->n + j], decimal_point);
            fputc('\n', out);
        }
        fputs("EOD\n", out);
    }

    if(has_expr) fputs("set samples 1000\n", out);

    if(g->series_count == 0){
        fputs("plot NaN notitle\n", out);
    }else{
        fputs("plot ", out);
        for(i = 0; i < g->series_count; i++){
            if(i > 0) fputs(", \\\n     ", out);
            gp__put_series(out, &g->series[i], i, decimal_point);
        }
        fputc('\n', out);
    }

    if(interactive){
        fputs("if (GPVAL_TERM eq \"qt\" || GPVAL_TERM eq \"wxt\" || GPVAL_TERM eq \"x11\") { pause mouse close }\n", out);
    }

    if(output != NULL) fputs("unset output\n", out);

    return ferror(out) ? GP_ERR_IO : GP_OK;
}

//================================ GNUPLOT PROCESS ===============================================

static int gp__exit_code(int status){

    if(status == -1) return -1;

#ifdef _WIN32
    return status;
#else
    if((status & 0x7f) != 0) return 128 + (status & 0x7f);     // killed by a signal

    return (status >> 8) & 0xff;
#endif
}

// Starts gnuplot, sends the script and waits for it to finish.
static int gp__run(const gp_graph* g, const char* terminal, const char* output, bool interactive){

    const char* env = getenv("GPLOT_GNUPLOT");
    const char* program = (env != NULL && env[0] != '\0') ? env : GPLOT_GNUPLOT_CMD;
    char* command;
    FILE* pipe;
    int err, code;
#ifdef SIGPIPE
    void (*previous_handler)(int);
#endif

    command = (char*)GPLOT_MALLOC(strlen(program) + sizeof(" -persist"));
    if(command == NULL) return GP_ERR_NOMEM;

    strcpy(command, program);
    if(interactive) strcat(command, " -persist");

#ifdef SIGPIPE
    // a gnuplot that died (or was never found) must not kill the program
    previous_handler = signal(SIGPIPE, SIG_IGN);
#endif

    // gnuplot shares our stdout (dumb terminal, messages): keep the order
    fflush(stdout);

    pipe = gp__popen(command);
    GPLOT_FREE(command);

    if(pipe == NULL){
        err = GP_ERR_SPAWN;
    }else{
        err = gp__write_script(g, pipe, terminal, output, interactive);
        code = gp__exit_code(gp__pclose(pipe));

#ifdef _WIN32
        if(code == 9009) err = GP_ERR_SPAWN;        // cmd.exe: command not found
#else
        if(code == 127) err = GP_ERR_SPAWN;         // sh: command not found
#endif
        else if(code != 0) err = GP_ERR_GNUPLOT;
    }

#ifdef SIGPIPE
    if(previous_handler != SIG_ERR) signal(SIGPIPE, previous_handler);
#endif

    return err;
}

static int gp__show(gp_graph* g){

    char buffer[256];
    const char* terminal = NULL;
    int err;

    if(g == NULL) return GP_ERR_ARG;

    if(g->terminal != NULL){
        terminal = g->terminal;

        // "dumb" sizes are in characters; a size given by the user wins
        if(strncmp(g->terminal, "dumb", 4) != 0 && strstr(g->terminal, "size") == NULL){
            int length = snprintf(buffer, sizeof(buffer), "%s size %d,%d", g->terminal, g->width, g->height);
            if(length > 0 && (size_t)length < sizeof(buffer)) terminal = buffer;
        }
    }
#ifdef __linux__
    else if(getenv("DISPLAY") == NULL && getenv("WAYLAND_DISPLAY") == NULL){
        terminal = "dumb";      // no display: ASCII art instead of a window that cannot open
    }
#endif

    err = gp__run(g, terminal, NULL, true);

    return (err != GP_OK) ? gp__fail(g, err) : GP_OK;
}

static int gp__savefig(gp_graph* g, const char* path){

    char terminal[128], width[32], height[32], extension[8];
    const char* dot;
    const char* separator;
    char decimal_point = gp__decimal_point();
    size_t i;
    int err;

    if(g == NULL) return GP_ERR_ARG;
    if(path == NULL || path[0] == '\0') return gp__fail(g, GP_ERR_ARG);

    dot = strrchr(path, '.');
    separator = strrchr(path, '/');
    if(separator == NULL) separator = strrchr(path, '\\');

    if(dot == NULL || (separator != NULL && dot < separator) || strlen(dot + 1) >= sizeof(extension)){
        return gp__fail(g, GP_ERR_FORMAT);
    }

    for(i = 0; dot[i + 1] != '\0'; i++) extension[i] = gp__lower(dot[i + 1]);
    extension[i] = '\0';

    // Vector formats are sized in inches, at matplotlib's 100 dpi. pdf/eps default to
    // fontscale 0.5 and draw lw 1 as 0.5 pt: "linewidth 1.43" makes lw 2.1 (matplotlib 1.5) 1.5 pt.
    gp__format_number(width, sizeof(width), "%g", g->width / 100.0, decimal_point);
    gp__format_number(height, sizeof(height), "%g", g->height / 100.0, decimal_point);

    if(!strcmp(extension, "png"))
        snprintf(terminal, sizeof(terminal), "pngcairo size %d,%d", g->width, g->height);
    else if(!strcmp(extension, "jpg") || !strcmp(extension, "jpeg"))
        snprintf(terminal, sizeof(terminal), "jpeg size %d,%d", g->width, g->height);
    else if(!strcmp(extension, "gif"))
        snprintf(terminal, sizeof(terminal), "gif size %d,%d", g->width, g->height);
    else if(!strcmp(extension, "svg"))
        snprintf(terminal, sizeof(terminal), "svg size %d,%d background rgb \"white\"", g->width, g->height);
    else if(!strcmp(extension, "pdf"))
        snprintf(terminal, sizeof(terminal), "pdfcairo size %sin,%sin font \",10\" fontscale 1 linewidth 1.43", width, height);
    else if(!strcmp(extension, "eps"))
        snprintf(terminal, sizeof(terminal), "epscairo size %sin,%sin font \",10\" fontscale 1 linewidth 1.43", width, height);
    else if(!strcmp(extension, "txt"))
        snprintf(terminal, sizeof(terminal), "dumb");
    else
        return gp__fail(g, GP_ERR_FORMAT);

    err = gp__run(g, terminal, path, false);

    return (err != GP_OK) ? gp__fail(g, err) : GP_OK;
}

static int gp__save_script(gp_graph* g, const char* path){

    FILE* file;
    int err;

    if(g == NULL) return GP_ERR_ARG;
    if(path == NULL || path[0] == '\0') return gp__fail(g, GP_ERR_ARG);

    if((file = fopen(path, "w")) == NULL) return gp__fail(g, GP_ERR_IO);

    err = gp__write_script(g, file, NULL, NULL, false);
    if(fclose(file) != 0 && err == GP_OK) err = GP_ERR_IO;

    return (err != GP_OK) ? gp__fail(g, err) : GP_OK;
}

static int gp__render(gp_graph* g, FILE* out, const char* terminal, const char* output){

    int err;

    if(g == NULL) return GP_ERR_ARG;
    if(out == NULL) return gp__fail(g, GP_ERR_ARG);

    err = gp__write_script(g, out, terminal, output, false);

    return (err != GP_OK) ? gp__fail(g, err) : GP_OK;
}

//================================ METHOD TABLE ===============================================

const gp_graph_class Graph = {
    .new         = gp__new,
    .free        = gp__free,
    .clear       = gp__clear,

    .plot        = gp__plot,
    .scatter     = gp__scatter,
    .fplot       = gp__fplot,

    .title       = gp__title,
    .xlabel      = gp__xlabel,
    .ylabel      = gp__ylabel,
    .xlim        = gp__xlim,
    .ylim        = gp__ylim,
    .xscale      = gp__xscale,
    .yscale      = gp__yscale,
    .grid        = gp__grid,
    .legend      = gp__legend,
    .size        = gp__size,

    .terminal    = gp__terminal,
    .cmd         = gp__cmd,

    .show        = gp__show,
    .savefig     = gp__savefig,
    .save_script = gp__save_script,
    .render      = gp__render,

    .error       = gp__error,
    .strerror    = gp__strerror
};

#endif // GPLOT_IMPLEMENTATION_INCLUDED
#endif // GPLOT_IMPLEMENTATION
