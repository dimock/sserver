/*
* File:   logger.cpp
* Author: dsultanov
*
* Created on January 11, 2017
*/

#include <sserver.h>
#include <fstream>
#include "boost/date_time/posix_time/posix_time.hpp"

namespace
{
  std::string s_logfile_name;
} // namespace {}

namespace logger_ns
{
  void send_to_logger(std::string const& msg)
  {
    std::cout << msg << std::endl;
    if(s_logfile_name.empty())
      return;
    std::ofstream of_log(s_logfile_name, std::ios::app);
    if(of_log)
      of_log << msg << std::endl;
  }

  void set_log_file(std::string const& fname)
  {
    s_logfile_name = fname;
  }

  std::string current_timestamp()
  {
    using namespace boost::posix_time;
    static std::locale loc(std::locale(),
                           new time_facet("%Y:%m:%d %H:%M:%S"));

    ptime now = second_clock::universal_time();
    std::ostringstream os;
    os.imbue(loc);
    os << now;
    return os.str();
  }

  std::ostream& operator << (std::ostream& os, message_type m)
  {
    static std::map<message_type, std::string> types
    {
      { message_type::M_INFO, "INFO" },
      { message_type::M_ERROR, "ERROR" }
    };
    os << types.at(m);
    return os;
  }

} // namespace logger_ns