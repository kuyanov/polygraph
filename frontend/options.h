#pragma once

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

class GlobalOptions {
public:
    std::string command;
    std::vector<std::string> command_options;

    static GlobalOptions &Get() {
        static GlobalOptions options;
        return options;
    }

    void Init(int argc, char **argv) {
        po::options_description desc;
        desc.add_options()("command", po::value<std::string>());
        desc.add_options()("command_options", po::value<std::vector<std::string>>());
        po::positional_options_description pos;
        pos.add("command", 1);
        pos.add("command_options", -1);
        po::variables_map vm;
        po::parsed_options parsed = po::command_line_parser(argc, argv)
                                        .options(desc)
                                        .positional(pos)
                                        .allow_unregistered()
                                        .run();
        po::store(parsed, vm);
        po::notify(vm);
        if (!vm.count("command")) {
            PrintHelpMessage();
            exit(EXIT_FAILURE);
        }
        command = vm["command"].as<std::string>();
        command_options = po::collect_unrecognized(parsed.options, po::include_positional);
        command_options.erase(std::find(command_options.begin(), command_options.end(), command));
    }

private:
    static void PrintHelpMessage() {
        std::cerr << "Usage:  polygraph COMMAND [COMMAND_OPTIONS]" << std::endl;
        std::cerr << std::endl;
        std::cerr << "Commands:" << std::endl;
        std::cerr << "  run      "
                  << "Run workflow" << std::endl;
        std::cerr << "  runner   "
                  << "Manage runners" << std::endl;
        std::cerr << "  start    "
                  << "Start polygraph service" << std::endl;
        std::cerr << "  stop     "
                  << "Stop polygraph service" << std::endl;
        std::cerr << std::endl;
        std::cerr << "Run 'polygraph COMMAND --help' for more information about the command."
                  << std::endl;
    }
};

class RunOptions {
public:
    std::string workflow;

    static RunOptions &Get() {
        static RunOptions options;
        return options;
    }

    void Init(const std::vector<std::string> &run_options) {
        po::options_description desc("Options");
        desc.add_options()("help", "print help message");
        desc.add_options()("workflow", po::value<std::string>(), "path to workflow file");
        po::variables_map vm;
        po::store(po::command_line_parser(run_options).options(desc).run(), vm);
        po::notify(vm);
        if (vm.count("help")) {
            std::cerr << "Run a workflow." << std::endl;
            std::cerr << std::endl;
            std::cerr << desc << std::endl;
            exit(EXIT_FAILURE);
        }
        if (!vm.count("workflow")) {
            std::cerr << "Missing required option 'workflow'." << std::endl;
            std::cerr << "See 'polygraph run --help'." << std::endl;
            exit(EXIT_FAILURE);
        }
        workflow = vm["workflow"].as<std::string>();
    }
};

class RunnerOptions {
public:
    std::string action;
    std::vector<std::string> action_options;

    static RunnerOptions &Get() {
        static RunnerOptions options;
        return options;
    }

    void Init(const std::vector<std::string> &runner_options) {
        po::options_description desc;
        desc.add_options()("action", po::value<std::string>());
        desc.add_options()("action_options", po::value<std::vector<std::string>>());
        po::positional_options_description pos;
        pos.add("action", 1);
        pos.add("action_options", -1);
        po::variables_map vm;
        po::parsed_options parsed = po::command_line_parser(runner_options)
                                        .options(desc)
                                        .positional(pos)
                                        .allow_unregistered()
                                        .run();
        po::store(parsed, vm);
        po::notify(vm);
        if (!vm.count("action")) {
            PrintHelpMessage();
            exit(EXIT_FAILURE);
        }
        action = vm["action"].as<std::string>();
        action_options = po::collect_unrecognized(parsed.options, po::include_positional);
        action_options.erase(std::find(action_options.begin(), action_options.end(), action));
    }

private:
    static void PrintHelpMessage() {
        std::cerr << "Usage:  polygraph runner ACTION [ACTION_OPTIONS]" << std::endl;
        std::cerr << std::endl;
        std::cerr << "Actions:" << std::endl;
        std::cerr << "  add      "
                  << "Connect a new runner" << std::endl;
        std::cerr << "  list     "
                  << "Show all runners" << std::endl;
        std::cerr << "  remove   "
                  << "Disconnect a runner" << std::endl;
        std::cerr << std::endl;
        std::cerr << "Run 'polygraph runner ACTION --help' for more information about the action."
                  << std::endl;
    }
};

class StartOptions {
public:
    std::string host;
    int port;
    std::string user_dir;

    static StartOptions &Get() {
        static StartOptions options;
        return options;
    }

    void Init(const std::vector<std::string> &start_options) {
        po::options_description desc("Options");
        desc.add_options()("help", "print help message");
        desc.add_options()("host", po::value<std::string>()->default_value("127.0.0.1"),
                           "polygraph host");
        desc.add_options()("port", po::value<int>()->default_value(3000), "polygraph port");
        desc.add_options()("user-dir", po::value<std::string>()->default_value("/home/polygraph"),
                           "directory for user files");
        po::variables_map vm;
        po::store(po::command_line_parser(start_options).options(desc).run(), vm);
        po::notify(vm);
        if (vm.count("help")) {
            std::cerr << "Start polygraph service." << std::endl;
            std::cerr << std::endl;
            std::cerr << desc << std::endl;
            exit(EXIT_FAILURE);
        }
        host = vm["host"].as<std::string>();
        port = vm["port"].as<int>();
        user_dir = vm["user-dir"].as<std::string>();
    }
};

class StopOptions {
public:
    static StopOptions &Get() {
        static StopOptions options;
        return options;
    }

    void Init(const std::vector<std::string> &stop_options) {
        po::options_description desc("Options");
        desc.add_options()("help", "print help message");
        po::variables_map vm;
        po::store(po::command_line_parser(stop_options).options(desc).run(), vm);
        po::notify(vm);
        if (vm.count("help")) {
            std::cerr << "Stop polygraph service." << std::endl;
            std::cerr << std::endl;
            std::cerr << desc << std::endl;
            exit(EXIT_FAILURE);
        }
    }
};
