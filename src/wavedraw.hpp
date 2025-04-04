#ifndef WAVEDRAW_HPP
#define WAVEDRAW_HPP

#include "definitions.hpp"
#include "settings.hpp"
#include "EquivalentSource.hpp"
#include "WaterSurface.hpp"


class WaveDraw
{
public:
    static std::vector<VEC2> positionSources(int nb_sources);
    static void setSinglePointtoHeight(VEC3 constr, VEC2 sourcePos, WaterSurface* ws);
    static FLOAT evaluateSolution(WaterSurface* ws);
    static void setAmplisFromConstr(WaterSurface* ws);
    static void test(WaterSurface* ws);
private:

};

#endif // WAVEDRAW_HPP
