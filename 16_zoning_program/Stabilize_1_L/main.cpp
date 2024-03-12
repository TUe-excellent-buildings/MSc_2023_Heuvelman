#include <iostream>
#include <ctime>

static double end = clock();
static double begin = end;

template<class T>
void out(T t, bool e, bool i){
    std::cout << t;
    end = clock();
    if (i) std::cout << " (" << 1000*(end-begin)/CLOCKS_PER_SEC << " ms)";
    if (e) std::cout << std::endl;
    begin = end;
} // out()

#include <BSO/Spatial_Design/Movable_Sizable.hpp>
#include <BSO/Spatial_Design/Conformation.hpp>
#include <BSO/Spatial_Design/Zoning.hpp>
#include <BSO/Structural_Design/SD_Analysis.hpp>
#include <BSO/Structural_Design/Stabilization/Stabilize.hpp>
#include <BSO/Visualisation/Visualisation.hpp>
#include <BSO/Performance_Indexing.hpp>
#include <AEI_Grammar/Grammar_stabilize.hpp>

int main(int argc, char* argv[])
{
    BSO::Spatial_Design::MS_Building MS("MS_Input.txt");out("Initialised the MS model",true, true);
    BSO::Spatial_Design::MS_Conformal CF(MS, &(BSO::Grammar::grammar_stabilize));out("Initialised the CF model",true, true);
    CF.make_conformal();out("Made the CF model conformal",true, true);

    out("Commencing Visualisation...",false, false); std::cout << std::endl;
    BSO::Visualisation::init_visualisation(argc, argv);
    BSO::Visualisation::visualise(MS);
    BSO::Visualisation::visualise(CF,"rectangles");

    out("Commencing SD-Analysis...",false, false);
    BSO::Structural_Design::SD_Analysis SD_Building(CF);

    BSO::Visualisation::end_visualisation();out("Finished visualisation",true, true);

    return 0;
}
