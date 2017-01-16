/*
* File:   sserver.cpp
* Author: dsultanov
*
* Created on January 11, 2017
*/

#include <sserver.h>

namespace client_ns
{

connection::connection(std::string const& ip_addr, int port, int send_timeout, bool quit)
  : connection_base(ip_addr, port),
    send_timeout_(send_timeout),
    generator{ rdevice() },
    distribution{ 0, 1023 },
    quit_{ quit }
{
  using namespace boost::asio;
  using namespace boost::asio::ip;
  receive_timer_ = timer_ptr(new timer_type(ioservice));
  sock = socket_ptr(new tcp::socket{ ioservice });
  sock->async_connect(*ep, [&, this](boost::system::error_code const& e)
  {
    read_handler(e, 0);
  });
}

void connection::read_handler(boost::system::error_code const& e, std::size_t nbytes)
{
  using namespace boost::asio;
  using namespace boost::asio::ip;
  if(!sock || e)
  {
    logger_ns::logger(logger_ns::message_type::M_ERROR, " error received: ", e.message());
    if(receive_timer_)
      receive_timer_->cancel();
    return;
  }
  if(quit_)
  {
    send_quit_signal();
    return;
  }
  print_received_data(nbytes);
  if(nbytes > 0)
    std::this_thread::sleep_for(std::chrono::duration<int, std::milli>(send_timeout_));
  if(!send_random_number())
    return;
  init_wait_timeout();
  sock->async_read_some(buffer(bytes),
                        [this](boost::system::error_code const& e, std::size_t nbytes)
  {
    read_handler(e, nbytes);
  });
}

void connection::send_quit_signal()
{
  using namespace boost::asio;
  try
  {
    if(write(*sock, buffer(quit_flag_)) == quit_flag_.size())
      std::this_thread::sleep_for(std::chrono::duration<int, std::milli>(send_timeout_));;
  }
  catch(std::exception& e)
  {
    logger_ns::logger(logger_ns::message_type::M_ERROR, " exception ", e.what());
  }
}

bool connection::send_random_number()
{
  using namespace boost::asio;
  try
  {
    value_type number = distribution(generator);
    write(*sock, buffer(&number, sizeof(number)));
  }
  catch(std::exception& e)
  {
    logger_ns::logger(logger_ns::message_type::M_ERROR, " exception ", e.what());
    return false;
  }
  return true;
}

void connection::print_received_data(size_t nbytes)
{
  if(nbytes == 0)
    return;
  size_t nnumbers = nbytes / sizeof(result_type);
  result_type const* numbers = reinterpret_cast<result_type const*>(bytes.data());
  for(size_t i = 0; i < nnumbers; ++i, ++numbers)
  {
    auto number = *numbers;
    logger_ns::logger(logger_ns::message_type::M_INFO, " received value = ", number);
  }
}

void connection::init_wait_timeout()
{
  if(!receive_timer_)
    return;
  receive_timer_->cancel();
  receive_timer_->expires_from_now(boost::posix_time::seconds(receive_timeout_ms_));
  receive_timer_->async_wait([this](boost::system::error_code const& e)
  {
    if(e)
      return;
    logger_ns::logger(logger_ns::message_type::M_INFO, " scoket closed by timeout");
    if(sock)
      sock->close();
  });
}

} // namespace client_ns
