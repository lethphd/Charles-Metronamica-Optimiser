
#include <iostream>
#include <string>
#include <random>
#include <chrono>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include "MetronamicaOF2_RandstadSmallCaseStudy_2Obj.hpp"
#include "../Pathify.hpp"


int main(int argc, char *argv[])
{

    CmdLinePaths metro_exe;  // path to the Metronamic executable GeonamicaCmd.exe
    CmdLinePaths mck_exe;    // path to the Map comparison kit executable
    CmdLinePaths wine_exe;   // path to the wine executable (windows emulator)
    CmdLinePaths java_exe;   // Path to jvm
    CmdLinePaths geoproj_manip_jar;  // path to the java jar for manipulating geoproject files.
    CmdLinePaths template_dir;
    std::string geoproj_file;
    std::string wine_working_dir_path;
    CmdLinePaths log_file;
    CmdLinePaths wine_reg_mod_file;
    CmdLinePaths actual_map_file;
    CmdLinePaths original_map_file;
    CmdLinePaths masking_map_file;
    CmdLinePaths fks_coefficients_file;
    CmdLinePaths working_dir;

//    boost::filesystem::path metro_exe_path;

    int pop_size = 3; // For the GA
    int max_gen_hvol;  // Termination condition for the GA
    int max_gen;
    int mail_hvol_gen;
    int replicates;

    namespace po = boost::program_options;
    po::options_description desc("Allowed options");
    desc.add_options()
            ("help,h", "produce help message")
            ("metro-exe,m", po::value<std::string>(&metro_exe.first), "path to Metronamica executable GeonamicaCmd.exe")
            ("mck-exe,k", po::value<std::string>(&mck_exe.first), "path to the Map comparison kit executable")
            ("wine-exe,w", po::value<std::string>(&wine_exe.first)->default_value("wine"), "path to the wine executable (windows emulator)")
            ("java_exe,j", po::value<std::string>(&java_exe.first)->default_value("java"), "Path to jvm" )
            ("geoproj-manip,g", po::value<std::string>(&geoproj_manip_jar.first), "path to the java jar for manipulating geoproject files.")
            ("template,t", po::value<std::string>(&template_dir.first), "path to template geoproject directory")
            ("geoproj-file,f", po::value<std::string>(&geoproj_file), "name of geoproject file (without full path)")
            ("log-name,l", po::value<std::string>(&log_file.first), "path of the log settings xml file (full path in wine format)")
            ("actual-map,a", po::value<std::string>(&actual_map_file.first), "path of the actual landuse map at end of simulation to compare to")
            ("orignal-map,o", po::value<std::string>(&original_map_file.first), "path of the landuse map at start of the simulation")
            ("masking-map,s", po::value<std::string>(&masking_map_file.first), "path of the masking map to mask out/in sections of the raster")
            ("fks-coeef,c", po::value<std::string>(&fks_coefficients_file.first), "path of the fuzzy coeeficint table")
            ("working-dir,d", po::value<std::string>(&working_dir.first)->default_value(boost::filesystem::current_path().string()), "path of directory for storing temp files during running")
            ("wine-work-dir,n", po::value<std::string>(&wine_working_dir_path), "path to working directory (working-dir,d), but in wine path format - e.g. Z:\\path\\to\\working\\dir")
            ("wine-reg-edit,r", po::value<std::string>(&wine_reg_mod_file.first), "path of the wine registry file to update registry on nodes")
            ("replicates,i", po::value<int>(&replicates)->default_value(10), "Number of times to rerun Metronamica to account for stochasticity of model for each objective function evaluation");



    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).
              options(desc).run(), vm);
    po::notify(vm);



    if (vm.count("help")) {
        std::cout << desc << "\n";
        return EXIT_SUCCESS;
    }

    pathify(metro_exe); //.second = boost::filesystem::path(metro_exe.first);
    pathify(mck_exe);
    pathify(wine_exe);
    pathify(java_exe);
    pathify(geoproj_manip_jar);
    pathify(template_dir);
    pathify(actual_map_file);
    pathify(original_map_file);
    pathify(masking_map_file);
    pathify(fks_coefficients_file);
    pathify(working_dir);
    pathify(wine_reg_mod_file);
    pathify(log_file);

    MetronamicaOF2SmallCaseStudy metro_eval(metro_exe.second,
                             mck_exe.second,
                             wine_exe.second,
                             java_exe.second,
                             geoproj_manip_jar.second,
                             template_dir.second,
                             working_dir.second,
                                            working_dir.second,
                             wine_working_dir_path,
                             geoproj_file,
                             log_file.second,
                             actual_map_file.second,
                             original_map_file.second,
                             masking_map_file.second,
                             fks_coefficients_file.second,
                             wine_reg_mod_file.second,
                             0,
                             true,
                             replicates);

    // The random number generator
    typedef std::mt19937 RNG;
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    RNG rng(seed);
    PopulationSPtr pop = intialisePopulationRandomDVAssignment(pop_size, metro_eval.getProblemDefinitions(), rng);

    int i = 0;
    BOOST_FOREACH(IndividualSPtr ind, *pop)
    {
        std::vector<double> objectives;
        std::vector<double> constraints;
        boost::filesystem::path save_dir = working_dir.second / ("individual_" + std::to_string(i++));
        if (!boost::filesystem::exists(save_dir)) boost::filesystem::create_directory(save_dir);
        std::tie(objectives, constraints) = metro_eval(ind->getRealDVVector(), ind->getIntDVVector(), save_dir);
        ind->setObjectives(objectives);
        ind->setConstraints(constraints);
        std::cout << *ind << std::endl;
    }

    return 0;
}
