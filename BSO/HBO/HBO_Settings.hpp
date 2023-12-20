#ifndef HBO_SETTINGS_HPP
#define HBO_SETTINGS_HPP

#include <BSO/Spatial_Design/Movable_Sizable.hpp>

#include <iostream>
#include <vector>
#include <sstream>
#include <cstdlib>

namespace BSO { namespace HBO{

    enum class assessment_level{INDIVIDUAL, CLUSTERS, ARG_COUNT}; // Whether spaces should be evaluated individually or in clusters

    enum class performance_assessment{AGGREGATED, MULTIDISCIPLINAR, ARG_COUNT}; // Evaluate by aggregating or multiple performances

    enum class aggregate_disciplines{SUMMATION, PRODUCT, DISTANCE, ARG_COUNT}; // which aggregation function should be used

    enum class space_selection{BEST, WORST, ARG_COUNT}; // select either the best or worst spaces

    enum class building_modification{SCALE, SWEEP, RELOCATE, ARG_COUNT}; // master modification techniques

    enum class rescaling_options{X, Y, Z, XY, XZ, YZ, XYZ, ARG_COUNT}; // which axis scale

    enum class sweeping_options{X, Y, Z, ARG_COUNT}; // which axis to sweep

    enum class sweep_direction{ POSITIVE, NEGATIVE, ARG_COUNT}; // Sweeping in the positive or negative direction


    struct Settings
    {
    public:
        performance_assessment assessment_options;
        space_selection selection_options;
        building_modification modification_options;

        aggregate_disciplines aggregate_options;
        assessment_level space_or_clus;
        rescaling_options rescaling;
        sweep_direction direction;

        std::vector<double> weights; // weight factors for the different performances
        unsigned int space_removal_requested ; // the amount of spaces to be selected for removal, can increase due to spaces with similar performance
        unsigned int space_removal_selected; // the amount of spaces selected for removal


        Settings() // default settings
        {
            assessment_options = performance_assessment::SOLO_AGGREGATED;
            selection_options = space_selection::POSITIVE_COEVO;
            modification_options = building_modification::SCALE_X;

            aggregate_options = aggregate_disciplines::SUMMATION;
            individual_or_clus = assessment_level::SPACES;
            space_removal_requested = 5;
            direction = sweep_direction.POSITIVE;
        }
    };

} // namespace HBO
} // namespace BSO
#endif // HBO_SETTINGS
