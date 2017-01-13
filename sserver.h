/*
* File:   sserver.h
* Author: dsultanov
*
* Created on January 11, 2017
*/

#pragma once

#include <iostream>
#include <algorithm>
#include <vector>
#include <array>
#include <chrono>
#include <random>
#include <map>
#include <list>
#include <random>
#include <sstream>
#include <thread>

#include <boost/asio.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/ip/tcp.hpp>

using bytes_array = std::array<char, 4096>;
using socket_ptr  = std::shared_ptr<boost::asio::ip::tcp::socket>;
using value_type  = std::int32_t;
using result_type = float;
using counter_type= std::int64_t;

class connection_base
{
public:
  // call to start communication
  virtual void start()
  {
    ioservice.run();
  }

protected:
  connection_base(std::string const& ip_addr, int port)
  {
    using boost::asio::ip::tcp;
    using boost::asio::ip::tcp;
    using boost::asio::ip::address;
    ep = std::unique_ptr<tcp::endpoint>{new tcp::endpoint(address::from_string(ip_addr), port)};
  }

  virtual ~connection_base() {}

  boost::asio::io_service ioservice;
  std::unique_ptr<boost::asio::ip::tcp::endpoint> ep;
  bytes_array bytes;
};

namespace server_ns
{
  class connection : public connection_base
  {
    int index = 0;
    int wite_timeout_;
    std::string output_file_;
    std::unique_ptr<boost::asio::ip::tcp::acceptor> acc;
    std::map<value_type, counter_type> values_;
 
  public:
    connection(std::string const& ip_addr, int port, int wite_timeout, std::string const& output_file);

  private:
    virtual void start() override;

    void start_connection(socket_ptr s);
    void connection_handler(socket_ptr s, const boost::system::error_code & e);
    void read_handler(int i, socket_ptr s, boost::system::error_code const& e, std::size_t nbytes);
    void process_received_data(int i, socket_ptr s, std::size_t nbytes);
    result_type calculate_average();
  };

} // namespace server_ns

namespace client_ns
{
  class connection : public connection_base
  {
    socket_ptr sock;
    int send_timeout_;
    std::random_device rdevice;
    std::mt19937 generator;
    std::uniform_int_distribution<> distribution;

  public:
    connection(std::string const& ip_addr, int port, int send_timeout);

  private:
    void read_handler(boost::system::error_code const& e, std::size_t nbytes);
    bool send_random_number();
    void print_received_data(size_t nbytes);
  };
} // namespace client_ns

namespace logger_ns
{
  enum class message_type
  {
    M_INFO,
    M_ERROR
  };

  std::ostream& operator << (std::ostream&, message_type m);
  void send_to_logger(std::string const& msg);
  std::string current_timestamp();
  void set_log_file(std::string const& fname);

  inline
  void print_log(std::ostream& os)
  {
  }

  template <class X, class ...T>
  void print_log(std::ostream& os, X x, T ...args)
  {
    os << x;
    print_log(os, args...);
  }

  template <class ...T>
  void logger(message_type mtype, T ...args)
  {
    std::ostringstream os;
    os << current_timestamp() << " -> " << mtype << ": ";
    print_log(os, args...);
    send_to_logger(os.str());
  }

} // logger_ns
