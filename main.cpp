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
    ("port,p", po::value<int>(), "listening given port")
    ("send_timeout,t", po::value<int>(), "sending random integer every given number of milliseconds")
    ("write_timeout,w", po::value<int>(), "writing to file every given number of seconds")
    ("output_file,f", po::value<std::string>(), "write result to given file")
    ("log_file,l", po::value<std::string>(), "log file name. for server only")
    ("quit,q", "send quit signal to server. for client only");

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
  int send_timeout = 200;
  int write_timeout = 10;
  std::string output_file{"numbers.dat"};

  if(vm.count("ip_addr"))
  {
    ip_addr = vm["ip_addr"].as<std::string>();
  }

  if(vm.count("port"))
  {
    port = vm["port"].as<int>();
  }

  if(vm.count("send_timeout"))
  {
    send_timeout = vm["send_timeout"].as<int>();
  }

  if(vm.count("write_timeout"))
  {
    write_timeout = vm["write_timeout"].as<int>();
  }

  if(vm.count("output_file"))
  {
    output_file = vm["output_file"].as<int>();
  }

  std::shared_ptr<connection_base> conn;
  if(vm.count("server"))
  {
    std::string log_file{"log.txt"};
    if(vm.count("log_file"))
    {
      log_file = vm["log_file"].as<std::string>();
    }
    logger_ns::set_log_file(log_file);
    logger_ns::logger(logger_ns::message_type::M_INFO, "starting server address ", ip_addr, " port ", port);
    conn.reset(new server_ns::connection(ip_addr, port, write_timeout, output_file));
  }
  else
  {
    bool quit{};
    if(vm.count("quit"))
    {
        quit = true;
    }
    logger_ns::logger(logger_ns::message_type::M_INFO, "starting client address ", ip_addr, " port ", port);
    conn.reset(new client_ns::connection(ip_addr, port, send_timeout, quit));
  }

  if(!conn)
  {
    logger_ns::logger(logger_ns::message_type::M_ERROR, "unable to open connection");
    return 0;
  }

  conn->start();
  return 0;
}
