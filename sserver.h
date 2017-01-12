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
#include <thread>
#include <sstream>

#include <boost/asio.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/ip/tcp.hpp>

using bytes_array = std::array<char, 4096>;
using socket_ptr  = std::shared_ptr<boost::asio::ip::tcp::socket>;

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
    acc = std::unique_ptr<tcp::acceptor>{new tcp::acceptor(ioservice, *ep)};
  }

  virtual ~connection_base() = 0 {}

  boost::asio::io_service ioservice;
  std::unique_ptr<boost::asio::ip::tcp::endpoint> ep;
  std::unique_ptr<boost::asio::ip::tcp::acceptor> acc;
  bytes_array bytes;
};

namespace server_ns
{
  class connection : public connection_base
  {
    int index = 0;

  public:
    connection(std::string const& ip_addr, int port);

  private:
    virtual void start() override;

    void start_connection(socket_ptr s);
    void connection_handler(socket_ptr s, const boost::system::error_code & e);
    void read_handler(int i, socket_ptr s, boost::system::error_code const& e, std::size_t nbytes);
  };

} // namespace server_ns

namespace client_ns
{
  class connection : public connection_base
  {
    socket_ptr sock;

  public:
    connection(std::string const& ip_addr, int port);

  private:
    void read_handler(boost::system::error_code const& e, std::size_t nbytes);
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