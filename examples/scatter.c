// Scatter plots: point clouds with transparency, a fitted line and every marker code.
// Build: cc -std=c99 examples/scatter.c -o scatter -lm      Run: ./scatter [--show]

#define GPLOT_IMPLEMENTATION
#include "../gplot_interface.h"

#include <math.h>
#include <string.h>

#define N 300

// deterministic normal samples (LCG + Box-Muller), so the example always draws the same picture
static double normal(void){

    static unsigned long state = 12345;
    double u1, u2;

    state = (state * 1103515245UL + 12345UL) % 2147483648UL;
    u1 = (state + 1.0) / 2147483649.0;
    state = (state * 1103515245UL + 12345UL) % 2147483648UL;
    u2 = (state + 1.0) / 2147483649.0;

    return sqrt(-2.0 * log(u1)) * cos(2.0 * 3.14159265358979323846 * u2);
}

int main(int argc, char** argv){

    static double xa[N], ya[N], xb[N], yb[N];
    static const char* markers[] = {".", ",", "o", "v", "^", "s", "D", "d", "p", "*", "+", "x"};
    double sx = 0, sy = 0, sxx = 0, sxy = 0, slope, intercept, line_x[2], line_y[2];
    bool show = argc > 1 && strcmp(argv[1], "--show") == 0;
    gp_graph* g;
    int i, err;

    for(i = 0; i < N; i++){
        xa[i] = 2.0 + normal();
        ya[i] = 1.0 + 0.8 * xa[i] + 0.6 * normal();
        xb[i] = 5.0 + 0.8 * normal();
        yb[i] = 7.0 - 0.5 * xb[i] + 0.5 * normal();
        sx += xa[i];
        sy += ya[i];
        sxx += xa[i] * xa[i];
        sxy += xa[i] * ya[i];
    }

    slope = (N * sxy - sx * sy) / (N * sxx - sx * sx);
    intercept = (sy - slope * sx) / N;
    line_x[0] = -1;
    line_x[1] = 7;
    line_y[0] = intercept + slope * line_x[0];
    line_y[1] = intercept + slope * line_x[1];

    g = Graph.new();
    if(g == NULL) return 1;

    Graph.title(g, "Two clusters");
    Graph.xlabel(g, "feature_1");
    Graph.ylabel(g, "feature_2");
    Graph.scatter(g, xa, ya, N, "alpha=0.6", "cluster A");
    Graph.scatter(g, xb, yb, N, "s ms=4 alpha=0.6", "cluster B");
    Graph.plot(g, line_x, line_y, 2, "k-- lw=1", "fit of A");
    Graph.legend(g, "lower right");
    Graph.grid(g, true);

    //Graph.savefig(g, "scatter.png");
    if(show) Graph.show(g);

    err = Graph.error(g);
    Graph.free(g);

    // every marker code, one row each
    g = Graph.new();
    if(g == NULL) return 1;

    Graph.title(g, "Marker codes");
    Graph.size(g, 640, 640);
    Graph.legend(g, "outside");

    for(i = 0; i < 12; i++){
        double mx[8], my[8];
        char fmt[32];
        int j;

        for(j = 0; j < 8; j++){
            mx[j] = j;
            my[j] = 12 - i;
        }

        snprintf(fmt, sizeof(fmt), "%s ms=8", markers[i]);
        Graph.scatter(g, mx, my, 8, fmt, markers[i]);
    }

    Graph.ylim(g, 0, 13);
    Graph.xlim(g, -1, 8);
    //Graph.savefig(g, "markers.png");
    if(show) Graph.show(g);

    if(err == GP_OK) err = Graph.error(g);
    if(err != GP_OK) fprintf(stderr, "scatter: %s\n", Graph.strerror(err));
    Graph.free(g);

    return err == GP_OK ? 0 : 1;
}
