/*
* File:   main.cpp
* Author: dsultanov
*
* Created on January 11, 2017
*/

#include <sserver.h>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>

int main(int argc, char** argv)
{
  namespace po = boost::program_options;
  po::options_description desc("Options");
  desc.add_options()
    ("help,h", "print this help")
    ("server,s", "start application as server. it would be client otherwise")
    ("ip_addr,i", po::value<std::string>(), "listening given ip-address")
    ("port,p", po::value<int>(), "listening given port");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if(vm.count("help"))
  {
    std::cout << desc << "\n";
    return 0;
  }

  std::string ip_addr{ "127.0.0.1" };
  int port = 2017;

  if(vm.count("ip_addr"))
  {
    ip_addr = vm["ip_addr"].as<std::string>();
  }

  if(vm.count("port"))
  {
    port = vm["port"].as<int>();
  }

  std::shared_ptr<connection_base> conn;
  if(vm.count("server"))
  {
    logger_ns::logger(logger_ns::message_type::M_INFO, "starting server...");
    conn.reset(new server_ns::connection(ip_addr, port));
  }
  else
  {
    logger_ns::logger(logger_ns::message_type::M_INFO, "starting client...");
    conn.reset(new client_ns::connection(ip_addr, port));
  }

  if(!conn)
  {
    logger_ns::logger(logger_ns::message_type::M_ERROR, "unable to open connection");
    return 0;
  }

  conn->start();
  return 0;
}
