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
//#include <BSO/Building_Physics/BP_Simulation.hpp>
#include <BSO/Visualisation/Visualisation.hpp>
#include <BSO/Performance_Indexing.hpp>
#include <AEI Grammar/Grammar_zoning.hpp>

int main(int argc, char* argv[])
{
    out("Heya, welcome to my program", true, false);
    BSO::Spatial_Design::MS_Building MS("MS_Input.txt");out("Initialised the MS model",true, true);
    BSO::Spatial_Design::MS_Conformal CF(MS, &(BSO::Grammar::grammar_zoning));out("Initialised the CF model",true, true);

    CF.make_conformal();out("Made the CF model conformal",true, true);

    BSO::Spatial_Design::Zoning::Zoned_Design Zoned(&CF);
    out("Initialised the zoning model", true, true);
    Zoned.make_zoning();
    out("finished zoning",true,true);



    out("Commencing Visualisation....",false, false);
    BSO::Visualisation::init_visualisation(argc, argv);

    BSO::Visualisation::visualise(MS);
    //BSO::Visualisation::visualise(MS,"space_type");
    //BSO::Visualisation::visualise(MS,"surface_type");
    //BSO::Visualisation::visualise(CF,"rectangles");
    BSO::Visualisation::visualise(CF,"cuboids");

    // vis Zoned Designs
    for (unsigned int i = 0; i < Zoned.get_designs().size(); i++)
    {
        BSO::Visualisation::visualise(CF,"zones", i);
    }

    // SD-analysis unzoned design
    Zoned.reset_SD_model();
    Zoned.prepare_unzoned_SD_model();
    BSO::Structural_Design::SD_Analysis SD_Building(CF);
    SD_Building.analyse();
    BSO::Structural_Design::SD_Building_Results sd_results = SD_Building.get_results();
    BSO::SD_compliance_indexing(sd_results);
    std::cout << std::endl << "Total compliance in the unzoned design: " << sd_results.m_total_compliance
    << std::endl << "Structural volume: " << sd_results.m_struct_volume << std::endl;
    Zoned.add_unzoned_compliance(sd_results.m_total_compliance);
    BSO::Visualisation::visualise(SD_Building, 1);

    // SD-analysis zoned designs
	std::vector<double> m_compliance;
	std::vector<double> m_volume;
    for (unsigned int i = 0; i < Zoned.get_designs().size(); i++)
    {
        Zoned.reset_SD_model();
        Zoned.prepare_zoned_SD_model(i);
        BSO::Structural_Design::SD_Analysis SD_Building(CF);
        SD_Building.analyse();
        BSO::Structural_Design::SD_Building_Results sd_results = SD_Building.get_results();
        BSO::SD_compliance_indexing(sd_results);
        std::cout << "Total compliance in zoned design " << i + 1 << ": "
        << sd_results.m_total_compliance << std::endl << "Structural volume: " << sd_results.m_struct_volume << std::endl;
        Zoned.add_compliance(sd_results.m_total_compliance, i);
        BSO::Visualisation::visualise(SD_Building, 1);
		m_compliance.push_back(sd_results.m_total_compliance);
		m_volume.push_back(sd_results.m_struct_volume);
    }
	std::cout << std::endl << "Compliances:" << std::endl;
	for (unsigned int i = 0 ; i < m_compliance.size(); i++)
	{
		std::cout << m_compliance[i] << std::endl;
	}
	std::cout << std::endl << "Volumes:" << std::endl;
	for (unsigned int i = 0 ; i < m_volume.size(); i++)
	{
		std::cout << m_volume[i] << std::endl;
	}


    std::cout << "Lowest total compliance (" << Zoned.get_min_compliance().first << ") in zoned design " <<
    Zoned.get_min_compliance().second + 1 << std::endl;
    Zoned.reset_SD_model();
    Zoned.prepare_zoned_SD_model(Zoned.get_min_compliance().second);
    BSO::Structural_Design::SD_Analysis SD_Zoned_Building(CF);
    SD_Zoned_Building.analyse();
    BSO::Structural_Design::SD_Building_Results sd_zoned_results =
    SD_Zoned_Building.get_results();
    BSO::SD_compliance_indexing(sd_zoned_results);
    BSO::Visualisation::visualise(SD_Zoned_Building, 4);

    // vis SD_Building
    //BSO::Visualisation::visualise(SD_Building, 2);
    //BSO::Visualisation::visualise(SD_Building, 1);
    //BSO::Visualisation::visualise(SD_Building, 4);

    out("Finished visualisation",true, true);

    BSO::Visualisation::end_visualisation();
    return 0;
}
