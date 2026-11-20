// Line plots: data series, format strings, an expression, decoration and output.
// Build: cc -std=c99 examples/lines.c -o lines -lm      Run: ./lines [--show]

#define GPLOT_IMPLEMENTATION
#include "../gplot_interface.h"

#include <math.h>
#include <string.h>

#define N 400

int main(int argc, char** argv){

    static double t[N], y[N], upper[N], lower[N];
    const double pi = 3.14159265358979323846;
    gp_graph* g;
    int i, err;

    for(i = 0; i < N; i++){
        t[i] = 10.0 * i / (N - 1);
        upper[i] = exp(-t[i] / 3.0);
        lower[i] = -upper[i];
        y[i] = upper[i] * cos(2.0 * pi * t[i]);
    }

    g = Graph.new();
    if(g == NULL) return 1;

    Graph.title(g, "Damped oscillation");
    Graph.xlabel(g, "t [s]");
    Graph.ylabel(g, "amplitude");

    Graph.plot(g, t, y, N, NULL, "signal");
    Graph.plot(g, t, upper, N, "k--", "envelope");
    Graph.plot(g, t, lower, N, "k-- lw=1", NULL);
    Graph.fplot(g, "0.5*exp(-x/3)*sin(2*pi*x)", "C3 lw=1 alpha=0.6", "quadrature / 2");

    Graph.xlim(g, 0, 10);
    Graph.grid(g, true);
    Graph.legend(g, "upper right");

    //Graph.savefig(g, "lines.png");
    //Graph.savefig(g, "lines.svg");
    //Graph.savefig(g, "lines.pdf");
    //Graph.save_script(g, "lines.gp");

    if(argc > 1 && strcmp(argv[1], "--show") == 0) Graph.show(g);

    err = Graph.error(g);
    if(err != GP_OK) fprintf(stderr, "lines: %s\n", Graph.strerror(err));

    // reuse the same graph: clear the series, keep the decoration
    Graph.clear(g);
    Graph.title(g, "Line styles and log scale");
    Graph.ylabel(g, "growth");
    Graph.xlim(g, NAN, NAN);
    Graph.yscale(g, "log");
    Graph.legend(g, "upper left");

    for(i = 0; i < N; i++) y[i] = exp(t[i] / 2.0);
    Graph.plot(g, t, y, N, "-", "solid  -");
    for(i = 0; i < N; i++) y[i] = exp(t[i] / 2.5);
    Graph.plot(g, t, y, N, "--", "dashed  --");
    for(i = 0; i < N; i++) y[i] = exp(t[i] / 3.0);
    Graph.plot(g, t, y, N, "-.", "dash-dot  -.");
    for(i = 0; i < N; i++) y[i] = exp(t[i] / 3.5);
    Graph.plot(g, t, y, N, ":", "dotted  :");
    for(i = 0; i <= 20; i++){
        upper[i] = i / 2.0;
        lower[i] = exp(upper[i] / 4.0);
    }
    Graph.plot(g, upper, lower, 21, "o- ms=5", "line + markers  o-");

    Graph.savefig(g, "lines_styles.png");

    if(argc > 1 && strcmp(argv[1], "--show") == 0) Graph.show(g);

    if(Graph.error(g) != GP_OK) err = Graph.error(g);
    Graph.free(g);

    return err == GP_OK ? 0 : 1;
}
