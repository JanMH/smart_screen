#include <iostream>
#include <boost/program_options.hpp>
#include <fstream>
#include "EventVisualizer.h"
#include "CrossValidationStats.h"


boost::program_options::options_description getProblemsDescription();


boost::program_options::variables_map parseCommandLine(int argc, const char *argv[]);

void printHelp();

void performVisualisation(const boost::program_options::variables_map &options, std::ostream *output_stream);

bool
performValidationResultsVisualisation(const boost::program_options::variables_map &options, std::ostream *output_stream,
                                      const std::string &command);


int main(int argc, const char *argv[]) {
    using namespace std;

    auto options = parseCommandLine(argc, argv);
    if (options.count("help") || options.count("command") == 0 || options["command"].as<std::string>() == "help") {
        printHelp();
        return 1;
    }
    std::ostream *output_stream = &cout;
    std::ofstream file;

    if (options.count("output")) {
        file.open(options["output"].as<std::string>());
        output_stream = &file;
    }
    performVisualisation(options, output_stream);

}

boost::program_options::options_description getProblemsDescription() {
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()("help,h", "produce help message")
            ("event-id", boost::program_options::value<unsigned long>(), "id of the event that should be visualized")
            ("event-directory,d", boost::program_options::value<std::string>()->default_value("events"),
             "the folder in which the events are stored")
            ("output,o", boost::program_options::value<std::string>(), "output file")
            ("command", boost::program_options::value<std::string>(), "command to execute")
            ("validation-results-archive", boost::program_options::value<std::string>(),
             "location of a validation results archive")

            ("number-of-elements", boost::program_options::value<unsigned long>()->default_value(6000),
             "number of data points to plot");


    return desc;
}

boost::program_options::variables_map parseCommandLine(int argc, const char **argv) {

    auto desc = getProblemsDescription();

    boost::program_options::positional_options_description command_option;
    command_option.add("command", -1);

    boost::program_options::variables_map vm;
    boost::program_options::store(
            boost::program_options::command_line_parser(argc, argv).options(desc).positional(command_option).run(), vm);
    boost::program_options::notify(vm);
    return vm;
}

void printHelp() {
    std::cout << getProblemsDescription() << std::endl;
}

void performVisualisation(const boost::program_options::variables_map &options, std::ostream *output_stream) {
    EventVisualizer<BluedDataPoint> visualizer;
    visualizer.setEventStorageDirectory(options["event-directory"].as<std::string>());
    auto command = options["command"].as<std::string>();

    if (command == "event") {
        if (options.count("event-id") == 0) {
            std::cerr << "please provide an event id" << std::endl;
            return;
        }
        visualizer.printEvent(options["event-id"].as<ulong>(), *output_stream);

    } else if (command == "fvect") {
        if (options.count("event-id") == 0) {
            std::cerr << "please provide an event id" << std::endl;
            return;
        }
        visualizer.printFeatureVector(options["event-id"].as<ulong>(), *output_stream);
    } else if (command == "fft") {
        if (options.count("event-id") == 0) {
            std::cerr << "please provide an event id" << std::endl;
            return;
        }
        visualizer.printFFTOfEvent(options["event-id"].as<ulong>(), *output_stream);
    } else if (command == "fft-before") {
        if (options.count("event-id") == 0) {
            std::cerr << "please provide an event id" << std::endl;
            return;
        }
        visualizer.printFFTBeforeEvent(options["event-id"].as<ulong>(), *output_stream);
    } else if (command == "fft-limited") {
        if (options.count("event-id") == 0) {
            std::cerr << "please provide an event id" << std::endl;
            return;
        }
        visualizer.printFFTOfEventLimitDataPoints(options["event-id"].as<ulong>(), *output_stream);
    } else if (command == "bmh") {
        visualizer.printBlackmanHarris(options["number-of-elements"].as<ulong>(), *output_stream);
    } else if (!performValidationResultsVisualisation(options, output_stream, command)) {
        std::cerr << "failed to parse command " << options["command"].as<std::string>() << std::endl;
    }
}

bool
performValidationResultsVisualisation(const boost::program_options::variables_map &options, std::ostream *output_stream,
                                      const std::string &command) {
    if (options.count("validation-results-archive") == 0) {
        std::cerr << "please provide a validation-results-archive" << std::endl;
        return true;
    }

    std::fstream fs(options["validation-results-archive"].as<std::string>());
    boost::archive::text_iarchive archive(fs);
    std::vector<CrossValidationResult> results;
    archive >> results;
    CrossValidationStats stats;
    stats.calculateStats(results);

    if (command == "false-classifications") {

        for (const auto &guess: stats.incorrectClassifications()) {
            *output_stream << guess.actual_label << ", " << guess.classified_label << ", " << guess.event_id << "\n";
        }
        return true;
    }
    if (command == "stats") {

        *output_stream << "correct classifications: " << stats.correctClassifications().size()
                       << "  false classifications: " << stats.incorrectClassifications().size() << "\n";
        return true;
    }
    if (command == "appliance-stats") {
        auto app_stats = stats.classificationStats();
        *output_stream << "Label, Correct Classifications, Incorrect Classifications\n";
        for (const auto &st: app_stats) {
            *output_stream << st.first << ", " << st.second.first << ", " << st.second.second << "\n";
        }
        *output_stream << "\n";
        return true;
    }
    return false;
}
