#include "plotter.hpp"
#include"C:/msys64/mingw64/include/png.h"
#include<string>
#include<limits>
#include<fstream>
#include <iostream>
#include <sstream>
#include<vector>
#include"error.hpp"
#include"WaterSurface.hpp"

void Plotter::createHeightMap(std::string inputFilename, std::string outputFilename, int nrow, int ncol){
    INFO("Creating Heightmap from "<< inputFilename);
    //Lecture des données, lecture de la hauteur max et min
    std::ifstream file(inputFilename, std::ifstream::in);
    std::vector<std::vector<double>> heightMapVector(nrow, std::vector<double>(ncol, 0.0));
    INFO("Taille : "<<nrow <<"*"<<ncol<<std::endl);
    double maxHeight = std::numeric_limits<float>::lowest(), minHeight = std::numeric_limits<float>::max();
    std::string line;
    while (std::getline(file, line)){
        std::istringstream iss(line);
        int x, y;
        double height;
        if (iss >> x >> y >> height){
            heightMapVector[x][y] = height;
            if (height > maxHeight){
                maxHeight = height;
            }
            if(height < minHeight){
                minHeight = height;
            }
        }
    }
    file.close();
    ///////

    FILE *fp = fopen(outputFilename.c_str(), "wb");
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

    png_init_io(png, fp);
    png_infop info = png_create_info_struct(png);
    // Écriture des en-têtes PNG
    png_set_IHDR(
        png,
        info,
        ncol, nrow, //width, height
        8, //bit depth of the image
        PNG_COLOR_TYPE_RGB,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT
        );
    png_write_info(png, info);

    std::vector<std::vector<png_byte>> imageData(nrow, std::vector<png_byte>(3 * ncol, 0));

    for (int x = 0; x<ncol; x++) {
        for (int y = 0; y<nrow; y++){
            float normalizedHeight = (heightMapVector[x][y] - minHeight) / (maxHeight - minHeight);
            if (normalizedHeight <= 0.5) {
                double factor = normalizedHeight / 0.5;
                imageData[y][3 * x] = 0;
                imageData[y][3 * x + 1] = static_cast<int>(255 * factor);
                imageData[y][3 * x + 2] = static_cast<int>(255* (1-factor));
            } else {
                double factor = (normalizedHeight - 0.5) / 0.5;
                imageData[y][3 * x] = static_cast<int>(255 * factor);
                imageData[y][3 * x + 1] = static_cast<int>(255* (1-factor));
                imageData[y][3 * x + 2] = 0;
            }
        }
    }

    // Écriture des pixels
    for (int y = 0; y < nrow; ++y) {
        png_write_row(png, imageData[y].data());
    }

    // Fin de l'écriture
    png_write_end(png, nullptr);

    fclose(fp);

    png_destroy_write_struct(&png, &info);
}

void Plotter::exportHeightMap(std::string outputFile, WaterSurface* surface, int nrows, int ncols){
    (*surface).drawHeighField("./heightfield.dat");
    INFO("HeightField drawn !");
    Plotter::createHeightMap("./heightfield.dat", outputFile, nrows, ncols);
}
