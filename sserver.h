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
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/ip/tcp.hpp>

using socket_ptr  = std::shared_ptr<boost::asio::ip::tcp::socket>;
using value_type  = std::int32_t;
using result_type = float;
using counter_type= std::int64_t;
using timer_type  = boost::asio::deadline_timer;
using timer_ptr   = std::unique_ptr<timer_type>;

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
  static const std::string quit_flag_;
};

namespace server_ns
{
  struct slot
  {
    socket_ptr s;
    std::array<char, 1024> buff_;
  };
  using slot_ptr = std::shared_ptr<slot>;
  
  class connection : public connection_base
  {
    int index = 0;
    int write_timeout_;
    std::string output_file_;
    std::unique_ptr<boost::asio::ip::tcp::acceptor> acc;
    std::map<value_type, counter_type> values_;
    timer_ptr write_timer_;
    bool please_stop_ = false;
 
  public:
    connection(std::string const& ip_addr, int port, int wite_timeout, std::string const& output_file);

  private:
    virtual void start() override;

    void start_connection();
    void connection_handler(slot_ptr slt, const boost::system::error_code & e);
    bool read_handler(int i, slot_ptr slt, boost::system::error_code const& e, std::size_t nbytes);
    bool process_received_data(int i, slot_ptr slt, std::size_t nbytes);
    result_type calculate_average() const;
    void on_timer(boost::system::error_code const& e);
    void write_results() const;
    void check_quit(int i, slot_ptr slt, size_t nbytes);
  };

} // namespace server_ns

namespace client_ns
{
  using bytes_array = std::array<char, 4096>;

  class connection : public connection_base
  {
    bytes_array bytes;
    socket_ptr sock;
    int send_timeout_;
    std::random_device rdevice;
    std::mt19937 generator;
    std::uniform_int_distribution<> distribution;
    timer_ptr receive_timer_;
    static const int receive_timeout_ms_ = 2;
    bool quit_{ false };

  public:
    connection(std::string const& ip_addr, int port, int send_timeout, bool quit);

  private:
    void read_handler(boost::system::error_code const& e, std::size_t nbytes);
    bool send_random_number();
    void send_quit_signal();
    void print_received_data(size_t nbytes);
    void init_wait_timeout();
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
