#ifndef PLOTTER_H
#define PLOTTER_H

#include<string>
#include"WaterSurface.hpp"

class Plotter
{
public:
    static void createHeightMap(std::string inputFilename, std::string outputFilename, int nrow, int ncol);
    static void exportHeightMap(std::string outputFile, WaterSurface* surface, int nrows, int ncols);
};

#endif // PLOTTER_H
