/*
    GNU PLOT INTERFACE

    Author: Guilherme Arruda

    Header repository : https://github.com/ohananoshi/gplot_interface

    Created in: 08/04/2023

    Last updated: 12/06/2023
*/

//============================= HEADERS =========================================

#pragma once

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <limits.h>

//=============================== DATATYPES ========================================

enum TERMINAL_OPTIONS{
    TERMINAL_CAIROLATEX = 0,
    TERMINAL_CANVAS,
    TERMINAL_CGM,
    TERMINAL_CONTEXT,
    TERMINAL_DOMTERM,
    TERMINAL_DUMB,
    TERMINAL_DXF,
    TERMINAL_EMF,
    TERMINAL_EPSCAIRO,
    TERMINAL_EPSLATEX,
    TERMINAL_FIG,
    TERMINAL_GIF,
    TERMINAL_HPGL,
    TERMINAL_JPEG,
    TERMINAL_LUA,
    TERMINAL_MF,
    TERMINAL_MP,
    TERMINAL_PCL5,
    TERMINAL_PDFCAIRO,
    TERMINAL_PICT2E,
    TERMINAL_PNG,
    TERMINAL_PNGCAIRO,
    TERMINAL_POSTSCRIPT,
    TERMINAL_PSLATEX,
    TERMINAL_PSTEX,
    TERMINAL_PSTRICKS,
    TERMINAL_QT,
    TERMINAL_SIXELGD,
    TERMINAL_TEXDRAW,
    TERMINAL_TIKZ,
    TERMINAL_TKCANVAS,
    TERMINAL_UNKNOWN,
    TERMINAL_WINDOWS,
    TERMINAL_WXT
};

enum MATH_FUNCTIONS{
    ABS = TERMINAL_WXT + 1,
    ACOS,
    ACOSH,
    AIRY,
    ARG,
    ASIN,
    ASINH,
    ATAN,
    ATAN2,
    ATANH,
    BESI0,
    BESI1,
    BESIN,
    BESJ0,
    BESJ1,
    BESJN,
    BESY0,
    BESY1,
    BESYN,
    CDAWSON,
    CEIL,
    CERF,
    COS,
    COSH,
    ELLIPTICE,
    ELLIPTICK,
    ELLIPTICPI,
    ERF,
    ERFC,
    ERFI,
    EXP,
    EXPINT,
    FADDEEVA,
    FLOOR,
    GAMMA,
    IBETA,
    IGAMMA,
    IMAG,
    INVERF,
    INT,
    LAMBERTW,
    LGAMMA,
    LOG,
    LOG10,
    NORM,
    RAND,
    REAL,
    SGN,
    SIN,
    SINH,
    SQRT,
    TAN,
    TANH,
    VOIGT,
    VOIGT_PROFILE,
    VOXEL
};

enum PLOT_STYLES{

    ARROWS = VOXEL + 1,
    BEE_SWARM_PLOTS,
    BOXEERRORBARS,
    BOXES,
    BOXPLOT,
    CANDLESTICKS,
    CIRCLES,
    ELLIPSES,
    DOTS,
    FILLEDCURVES,
    FINANCEBARS,
    FSTEPS,
    FILLSTEPS,
    HISTEPS,
    HISTOGRAMS,
    IMAGE,
    IMPULSES,
    LABELS,
    LINES,
    LINESPOINTS,
    PARALLELAXES,
    POLAR_PLOTS,
    POINTS,
    POLYGONS,
    RGBALPHA,
    RGBIMAGE,
    SPIDERPLOT,
    STEPS,
    VECTORS,
    XERRORBARS,
    XYERRORBARS,
    XERRORLINES,
    XYERRORLINES,
    YERRORBARS,
    YERRORLINES,
    D3D_PLOTS,
    FENCE_PLOTS,
    ISOSURFACE,
    ZERRORFILL
};

typedef struct{
    char** style;

    void (*set_style)(char*** data_style, uint16_t plot_style, ...);

}GRAPH_STYLE;

typedef struct{
    char* title;
}GRAPH_LABEL;

typedef struct{
    double** data;
    uint16_t* data_width;

    uint16_t* (*set_data)(double*** data, uint16_t array_count, uint16_t array_std_size, ...);
}GRAPH_DATA;

typedef struct{
    GRAPH_STYLE plot_style;
    GRAPH_LABEL plot_label;
    GRAPH_DATA plot_data;

}plot_context;

//================================== PLOTTING STYLES ================================================

typedef struct{
    bool set_box_width;
    bool relative;
    float box_width;

    bool set_fill_color;
    char* fill_color;

    bool set_fill_style;
    char* fill_style;
    float density;

    bool set_border;
    char* border_color;

    char** format = 
}BOXES_2D;

//=================================== GPLOT COMMANDS ===============================================


char* ERRORS[] = {"Unable to start gnuplot.",
                  "Selected terminal doesn't exist.",
                  "The command could not be executed."};

char* GNUPLOT_TERMINALS[] = {"cairolatex", "canvas", "cgm", "context", "domterm",
                             "dumb", "dxf", "emf", "epscairo", "epslatex", "fig",
                             "gif", "hpgl", "jpeg", "lua", "mf", "mp", "pcl5",
                             "pdfcairo", "pict2e", "png","pngcairo", "postscript",
                             "pslatex", "pstex", "pstricks", "qt", "sixelgd", "texdraw",
                             "tikz", "tkcanvas", "unknown", "windows", "wxt"};

char* GNUPLOT_MATH_FUNCTIONS[] = {"abs", "acos", "acosh", "airy", "arg", "asin", "asinh", "atan", "atan2", "atanh",
                                  "besi0", "besi1", "besin", "besj0", "besj1", "besjn", "besy0", "besy1", "besyn",
                                  "cdawson", "ceil", "cerf", "cos", "cosh",
                                  "EllipticE", "EllipticK", "EllipticPi", "erf", "erfc", "erfi", "exp", "expint",
                                  "faddeeva", "floor", "gamma", "ibeta", "igamma", "imag", "inverf", "int", "lambertw", "lgamma", "log", "log10",
                                  "norm", "rand", "real", "sgn", "sin", "sinh", "sqrt",
                                  "tan", "tanh", "voigt", "Voigt Profile", "voxel"};

char* GNUPLOT_PLOT_STYLES[] = {"arrows", "Bee swarm plots", "boxeerrorbars", "boxes", "boxplot", "candlesticks",
                               "circles", "ellipses", "dots", "filledcurves", "financebars", "fsteps", "fillsteps",
                               "histeps", "histograms", "image", "impulses", "labels", "lines", "linespoints", "parallelaxes",
                               "Polar plots", "points", "polygons", "rgbalpha", "rgbimage", "spiderplot", "steps", "vectors",
                               "xerrorbars", "xyerrorbars", "xerrorlines", "xyerrorlines", "yerrorbars", "yerrorlines",
                               "3D plots", "Fence plots", "isosurface", "Zerrorfill"};


//================================ FUNCTIONS ===============================================
/*
typedef struct{
    bool set_box_width;
    bool relative;
    float box_width;

    bool set_fill_color;
    char* fill_color;

    bool set_fill_style;
    char* fill_style;
    float density;

    bool set_border;
    char* border_color;
}BOXES_2D;*/

uint16_t* set_data(double*** data, uint16_t array_count, uint16_t array_std_size, ...){

    uint16_t* width = (uint16_t*)calloc(2, sizeof(uint16_t));

    width[0] = array_count;
    width[1] = array_std_size;

    (*data) = (double**)calloc(array_count, sizeof(double*));
    for(uint16_t i = 0; i < array_count; i++){
        (*data)[i] = (double*)calloc(array_std_size, sizeof(double));
    }

    va_list arrays;
    va_start(arrays, array_std_size);

    double* temp_array;

    for(uint16_t i = 0; i < array_count; i++){
        //printf("Array %d: ", i);
        temp_array = va_arg(arrays, double*);

        for(uint16_t j = 0; j < array_std_size; j++){
            (*data)[i][j] = temp_array[j];
            //printf(" %f | %f -", (*data)[i][j] , temp_array[j]);
        }
        //printf("\n");
    }

    va_end(arrays);

    return width;
}

void set_style(char*** data_style, uint16_t plot_style, uint8_t string_counter, ...){

    switch (plot_style)
    {
    case BOXES:
        {

        }
    
    default:
        break;
    }
}

plot_context gplot_create_plot_context(/*char* plot_type, char* plot_color*/){

    plot_context new_plot;

    //new_plot.plot_style.type = plot_type;
    //new_plot.plot_style.color = plot_color;
    new_plot.plot_data.set_data = set_data;

    return new_plot;

}

FILE* gplot_init(const char* command_line_arguments){

    char* command_string = (char*)calloc(20 + strlen(command_line_arguments),sizeof(char));

    sprintf(command_string, "%s %s", "gnuplot", command_line_arguments);

    //printf("\n\n%s\n\n", command_string);
    //system("pause");

    FILE* gplot_pipe = popen(command_string, "w");
    fflush(gplot_pipe);
    free(command_string);

    if(gplot_pipe == NULL){
        fprintf(stderr, ERRORS[0]);
        exit(0);
    }

    return gplot_pipe;
}


char* gplot_set_terminal(FILE* gnuplot_pipe, uint16_t terminal_name){

    if((terminal_name < TERMINAL_CAIROLATEX) || (terminal_name > TERMINAL_WXT)){
        fprintf(stderr, ERRORS[1]);
        return NULL;
    }

    char* command_string = (char*)calloc(40, sizeof(char));
    int result;

    strcpy(command_string, "set terminal ");
    sprintf(command_string + strlen(command_string), GNUPLOT_TERMINALS[terminal_name]);

    result = fprintf(gnuplot_pipe, command_string);
    fflush(gnuplot_pipe);

    if(result < 0){
        fprintf(stderr, ERRORS[2]);
        return NULL;
    }

    free(command_string);

    return GNUPLOT_TERMINALS[terminal_name];
}

void gplot_plot_context(FILE* gplot_pipe, plot_context context){

    fprintf(gplot_pipe, "set title \"%s\"\n", context.plot_label.title);

    fprintf(gplot_pipe,"plot '-' with %s linecolor \"%s\"\n", context.plot_style.type, context.plot_style.color);

    for(uint16_t i = 0; i < context.plot_data.data_width[1]; i++){
        for(uint16_t j = 0; j < context.plot_data.data_width[0]; j++){
            fprintf(gplot_pipe, "%f ", context.plot_data.data[j][i]);
        }
        fprintf(gplot_pipe, "\n");
        //fprintf(gplot_pipe,"%f %f\n", context.plot_data.data[0][i], context.plot_data.data[1][i]);
    }

    fprintf(gplot_pipe, "e\n");
}
