#include "sleep.hh"
#include <argparse/argparse.hpp>

int main(int argc, char **argv) { 
    argparse::ArgumentParser program("voting simulator");
    program
        .add_argument("-t")
        .required()
        .help("specify the time for each voter to arrive")
        .scan<'i', int>();
        
    program
        .add_argument("-p")
        .required()
        .help("specify the probability of a voter arriving")
        .scan<'f', float>();

    program
        .add_argument("-c")
        .help("specify the number of polling stations")
        .default_value(1)
        .scan<'i', int>();


    try {
      program.parse_args(argc, argv);
    }
    catch (const std::runtime_error& err) {
      std::cerr << err.what() << std::endl;
      std::cerr << program;
      return 1;
    }

    auto time               = program.get<int>("-t");
    auto probability        = program.get<float>("-p");
    auto number_of_stations = program.get<int>("-c");

    std::cout << "time: " << time << "\n";

    std::cout << "prob: " << probability << "\n";

    std::cout << "stations: " << number_of_stations << "\n";


    return 0;
}