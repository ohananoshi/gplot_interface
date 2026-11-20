# gplot_interface

matplotlib-like plotting for C, on top of [gnuplot](http://www.gnuplot.info/), in a single header.

```c
#define GPLOT_IMPLEMENTATION
#include "gplot_interface.h"

int main(void){
    double t[100], y[100];
    for(int i = 0; i < 100; i++){ t[i] = i / 10.0; y[i] = t[i] * t[i]; }

    gp_graph* g = Graph.new();
    Graph.title(g, "Free fall");
    Graph.xlabel(g, "t [s]");
    Graph.ylabel(g, "distance [m]");
    Graph.plot(g, t, y, 100, "r--", "measured");
    Graph.legend(g, "upper left");
    Graph.grid(g, true);
    Graph.savefig(g, "fall.png");
    Graph.show(g);                  // blocks until the window is closed, like plt.show()
    Graph.free(g);
    return 0;
}
```

## Requirements

- A C99 compiler (C11, C17 and C23 work too). The header is C-only.
- gnuplot 5.4 or newer on the `PATH`, built with the cairo terminals (`pngcairo`, `pdfcairo`) and an interactive one (`qt`, `wxt` or `x11`) for `show`. On Debian/Ubuntu: `sudo apt install gnuplot`.

## Using the header

The library is one file, written in the [stb](https://github.com/nothings/stb) style:

```c
// in exactly ONE .c file of your program
#define GPLOT_IMPLEMENTATION
#include "gplot_interface.h"

// in every other file
#include "gplot_interface.h"
```

Then compile normally, for example `cc -std=c99 main.c -o main`.

## Design

A `gp_graph` holds the whole context of one graph: its series, title, labels, limits, scales, legend, size and terminal. You operate on it through the `Graph` method table, so typing `Graph.` gives you every operation in the editor's autocomplete:

```c
gp_graph* g = Graph.new();
Graph.title(g, "...");
Graph.plot(g, x, y, n, "fmt", "label");
Graph.show(g);
Graph.free(g);
```

Nothing is drawn while you build the graph. `show`, `savefig` and `save_script` turn the whole graph into one gnuplot script at that moment. That means:

- the same graph can be shown, saved as PNG, saved as PDF, changed and saved again;
- data arrays are copied when you add them, so you can free or reuse them right away;
- there is no hidden global state: different graphs are independent.

## Methods

Every method takes the graph first and returns `GP_OK` (0) or a negative `GP_ERR_*` code, unless noted.

| Method | What it does |
|---|---|
| `Graph.new()` | New empty graph. Returns `NULL` when out of memory. |
| `Graph.free(g)` | Releases the graph and everything in it. |
| `Graph.clear(g)` | Removes all series and restarts the color cycle. Keeps title, labels, limits and other settings. |
| `Graph.plot(g, x, y, n, fmt, label)` | Line plot. `x == NULL` plots `y` against 0, 1, ..., n-1. |
| `Graph.scatter(g, x, y, n, fmt, label)` | Markers only (default marker `o`). |
| `Graph.fplot(g, expr, fmt, label)` | Plots a gnuplot expression in `x`, e.g. `"sin(x)/x"`. Without `xlim` gnuplot samples [-10, 10]. |
| `Graph.title(g, text)` / `xlabel` / `ylabel` | Sets the text; `NULL` removes it. |
| `Graph.xlim(g, lo, hi)` / `ylim` | Axis limits. `NAN` on a side keeps that side automatic. `lo > hi` reverses the axis. |
| `Graph.xscale(g, "log")` / `yscale` | `"linear"` or `"log"`. |
| `Graph.grid(g, on)` | Shows or hides the grid. |
| `Graph.legend(g, loc)` | Shows the legend at `loc` (see below). Without a call the legend is hidden, as in matplotlib. |
| `Graph.size(g, width, height)` | Size in pixels, default 640 x 480 (matplotlib's 6.4 x 4.8 in at 100 dpi). |
| `Graph.terminal(g, name)` | Terminal used by `show`: `"qt"`, `"wxt"`, `"x11"`, `"dumb"` (ASCII art)... `NULL` = gnuplot's default. |
| `Graph.cmd(g, command)` | Adds a raw gnuplot command, the escape hatch for anything not wrapped (`"set xtics 0.5"`). |
| `Graph.show(g)` | Opens a window and waits until it is closed. On Linux without a display it prints ASCII art instead. |
| `Graph.savefig(g, path)` | Saves to a file. The format comes from the extension: `.png` `.jpg` `.jpeg` `.gif` `.svg` `.pdf` `.eps` `.txt`. |
| `Graph.save_script(g, path)` | Writes a standalone gnuplot script with the data inside (`gnuplot -persist file.gp`). |
| `Graph.render(g, stream, terminal, output)` | Writes the script to any `FILE*`; `terminal` and `output` may be `NULL`. |
| `Graph.error(g)` | First error recorded on the graph, or `GP_OK`. |
| `Graph.strerror(err)` | Message for an error code. |

## Format strings

`fmt` uses matplotlib's codes, `[marker][line][color]`, in any order:

| Kind | Codes |
|---|---|
| Colors | `b` `g` `r` `c` `m` `y` `k` `w`, and `C0`...`C9` for the default color cycle |
| Lines | `-` solid, `--` dashed, `-.` dash-dot, `:` dotted |
| Markers | `.` point, `,` pixel, `o` circle, `v` `^` triangles, `s` square, `D` `d` diamonds, `p` pentagon, `*` star, `+` plus, `x` cross |

- A marker without a line style draws only markers (`"ro"`); add a line style to get both (`"ro-"`).
- `NULL` or `""` draws a solid line in the next color of the cycle (matplotlib's "tab10").
- A token that is only a color also works: `"green"`, `"#1f77b4"`, `"tab:red"`.

After the codes you can add `key=value` options, separated by spaces:

| Option | Meaning |
|---|---|
| `color=` / `c=` | Any color form above |
| `lw=` / `linewidth=` | Line width in points (default 1.5) |
| `ms=` / `markersize=` | Marker size in points (default 6) |
| `alpha=` | Opacity from 0 to 1 |
| `ls=` / `linestyle=` | `-` `--` `-.` `:` `solid` `dashed` `dashdot` `dotted` `none` |
| `marker=` | A marker code, or `none` |

Example: `"r--o lw=2 ms=4 alpha=0.6"`. Numbers always use `.` as the decimal point, whatever the locale.

Labels that are `NULL`, `""` or start with `_` are left out of the legend.

## Legend locations

`"best"` (same as `NULL`; gnuplot has no automatic placement, so it means upper right), `"upper right"`, `"upper left"`, `"lower left"`, `"lower right"`, `"right"`, `"center left"`, `"center right"`, `"lower center"`, `"upper center"`, `"center"`, `"outside"` (right of the plot), and `"off"` to hide it again.

## Errors

Errors never end the program. Each method returns its error code, and the graph also remembers the first error, so you can check once at the end:

```c
Graph.plot(g, x, y, n, "r--", "a");
Graph.scatter(g, x, z, n, "bo", "b");
Graph.savefig(g, "out.png");
if(Graph.error(g) != GP_OK) fprintf(stderr, "plot failed: %s\n", Graph.strerror(Graph.error(g)));
```

| Code | Meaning |
|---|---|
| `GP_ERR_ARG` | Invalid argument (`NULL` graph or data, `n == 0`, unknown option) |
| `GP_ERR_NOMEM` | Out of memory |
| `GP_ERR_FMT` | Invalid format string |
| `GP_ERR_FORMAT` | `savefig`: unknown file extension |
| `GP_ERR_SPAWN` | gnuplot could not be started (is it installed?) |
| `GP_ERR_GNUPLOT` | gnuplot reported an error (its message goes to stderr) |
| `GP_ERR_IO` | Writing the script or the file failed |

## Configuration

Define these before including the header in the implementation file:

| Macro | Default | Purpose |
|---|---|---|
| `GPLOT_GNUPLOT_CMD` | `"gnuplot"` | Command that starts gnuplot. The `GPLOT_GNUPLOT` environment variable overrides it at run time. |
| `GPLOT_MALLOC`, `GPLOT_REALLOC`, `GPLOT_FREE` | `malloc`, `realloc`, `free` | Allocator hooks (define all three). |
| `GPLOT_DEBUG` | off | Prints every error to stderr. |

## Safety notes

- Titles, labels, legend entries and file names are escaped, including backquotes, which gnuplot would otherwise run as shell commands.
- The strings given to `fplot`, `terminal` and `cmd` are raw gnuplot code, and gnuplot can run shell commands. Never pass untrusted input to them.
- While `show`/`savefig` talk to gnuplot, `SIGPIPE` is ignored, so a gnuplot that fails to start cannot kill your program. Signal handlers are process-wide, so avoid calling `show`/`savefig` while another thread changes the `SIGPIPE` handler.

## Building the tests and examples

```bash
make test
```

`make test` builds the test suite with AddressSanitizer and UndefinedBehaviorSanitizer and runs it. It checks the generated gnuplot scripts and does not need gnuplot.

```bash
make examples
```

`make examples` builds `examples/lines.c` and `examples/scatter.c` and writes their PNG, SVG and PDF output to `build/`. This needs gnuplot. Pass `--show` to an example to open the windows too.

## Roadmap

The first version covers line and scatter plots. Next candidates: bar charts and histograms, error bars and filled areas, images and heatmaps, subplots, and 3D surfaces.
