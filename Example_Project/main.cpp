#include <iostream>

#include <BSO/Spatial_Design/Movable_Sizable.hpp>
#include <BSO/Spatial_Design/Conformation.hpp>
#include <BSO/Structural_Design/SD_Analysis.hpp>
#include <BSO/Building_Physics/BP_Simulation.hpp>
#include <BSO/Visualisation/Visualisation.hpp>
#include <BSO/Performance_Indexing.hpp>
#include <AEI Grammar/Grammar_2.hpp>


void out(std::string s)
{std::cout << s << std::endl;}

int main(int argc, char* argv[])
{
    BSO::Spatial_Design::MS_Building MS("MS_Input.txt");out("Created the MS model");
    BSO::Spatial_Design::MS_Conformal CF(MS, &(BSO::Grammar::grammar_2));out("Created the CF model");
    CF.make_conformal();out("made the CF model conformal");

    BSO::Building_Physics::BP_Simulation BP(CF); out("created the BP model");
    BP.sim_period(); out("simulated the BP_model");

    BSO::Building_Physics::BP_Building_Results BP_result = BP.get_results(); std::cout << "total building energy: " << BP_result.m_total_energy << std::endl;
    BSO::BP_thermal_demand_indexing(BP_result); out("Indexed the BP results");

    BSO::Structural_Design::SD_Analysis SD(CF); out("created the SD model");
    SD.analyse();

    BSO::Structural_Design::SD_Building_Results SD_result = SD.get_results(); std::cout << "total building compliance: " << SD_result.m_total_compliance << std::endl;
    BSO::SD_compliance_indexing(SD_result); out("Indexed the SD results");

    BSO::Visualisation::init_visualisation(argc, argv);
    BSO::Visualisation::visualise(MS);
    BSO::Visualisation::visualise(CF, "rectangles");
    BSO::Visualisation::visualise(CF, "cuboids");
    BSO::Visualisation::visualise(BP);
    BSO::Visualisation::visualise(MS, BP_result);
    BSO::Visualisation::visualise(SD,1,true);
    BSO::Visualisation::visualise(SD,4);
    BSO::Visualisation::visualise(MS, SD_result);
    BSO::Visualisation::end_visualisation();
    return 0;
}
