/* 
 * File:   sserver.cpp
 * Author: dsultanov
 *
 * Created on January 11, 2017
 */

#include <sserver.h>
#include <fstream>
#include <numeric>
#include <boost/algorithm/string/join.hpp>

const std::string connection_base::quit_str_ = "quit";

namespace
{

// write this struct to binary file
#pragma pack (push,1)
struct bin_values_struct
{
  value_type   value;
  counter_type counter;
};
#pragma pack (pop)

} // namespace {}

namespace server_ns
{

connection::connection(std::string const& ip_addr, int port,
                       int write_timeout, std::string const& output_file)
  : connection_base(ip_addr, port),
    write_timeout_(write_timeout),
    output_file_(output_file)
{
  using namespace boost::asio::ip;
  acc = std::unique_ptr<tcp::acceptor>{new tcp::acceptor(ioservice, *ep)}; 
  write_timer_ = timer_ptr(new timer_type(ioservice, boost::posix_time::seconds(write_timeout_)));
  write_timer_->async_wait([this](boost::system::error_code const& e)
  {
    on_timer(e);
  });
}

void connection::start()
{
  start_connection(socket_ptr(new boost::asio::ip::tcp::socket(ioservice)));
  connection_base::start();
}

void connection::start_connection(socket_ptr s)
{
  if(!s || !acc)
    return;
  acc->async_accept(*s, [this, s](const boost::system::error_code & e)
  {
    connection_handler(s, e);
  });
}

void connection::connection_handler(socket_ptr s, const boost::system::error_code & e)
{
  using boost::asio::buffer;
  if(!read_handler(index, s, e, 0))
    return;
  logger_ns::logger(logger_ns::message_type::M_INFO, index, "-th client is connected");
  index++;
  start_connection(socket_ptr(new boost::asio::ip::tcp::socket(ioservice)));
}

bool connection::read_handler(int i, socket_ptr s,
                              boost::system::error_code const& e,
                              std::size_t nbytes)
{
  check_quit(nbytes);
  using boost::asio::buffer;
  if(!s || e || please_stop_)
    return false;
  if(!process_received_data(i, s, nbytes))
    return false;
  s->async_read_some(buffer(bytes),
                     [this, s, i](boost::system::error_code const& e,
                     std::size_t nbytes) {
    read_handler(i, s, e, nbytes);
  });
  return true;
}

bool connection::process_received_data(int i, socket_ptr s, std::size_t nbytes)
{
  if(nbytes == 0)
    return true;
  if(!s)
    return false;
  using boost::asio::buffer;
  size_t nnumbers = nbytes / sizeof(value_type);
  value_type const* numbers = reinterpret_cast<value_type const*>(bytes.data());
  std::vector<std::string> str_numbers;
  std::transform(numbers, numbers + nnumbers,
                 std::back_inserter(str_numbers),
                 [this](value_type v)
                 {
                   // side effect
                   values_[v]++;
                   return std::to_string(v);
                 });
  auto str = boost::algorithm::join(str_numbers, ",");
  logger_ns::logger(logger_ns::message_type::M_INFO, i, "-th client sent ",
                    nbytes, " bytes with numbers set {", str, "}");
  result_type result = calculate_average();
  try
  {
    boost::asio::write(*s, buffer(&result, sizeof(result)));
  }
  catch(std::exception& ex)
  {
    logger_ns::logger(logger_ns::message_type::M_ERROR, i,
                      "-th cleint reading caused exception ", ex.what());
    return false;
  }
  return true;
}

// this version has been written non-optimized to clarify algorithm
// it is possible to store summ and count somewhere but
// requires some tricks to have deal with calculation error
result_type connection::calculate_average() const
{
  // calculate weighted average based on quantity of each key  
  std::int64_t count{};
  std::vector<double> squares;
  squares.reserve(values_.size());
  for(auto const& kv : values_)
  {
    squares.push_back(kv.first * kv.first * kv.second);
    count += kv.second;
  }
  if(count == 0)
    return result_type{};
  // perform summation starting from smallest number to eliminate error
  std::sort(squares.begin(), squares.end());
  return std::accumulate(squares.begin(), squares.end(), 0) / count;
}

void connection::check_quit(size_t nbytes)
{
  if(nbytes > 0)
  {
    std::string str(bytes.data(), bytes.data()+nbytes);
    if(str.find(quit_str_) != std::string::npos)
    {
      if(write_timer_)
        write_timer_->cancel();
      if(acc)
        acc->cancel();
      please_stop_ = true;
    }
  }
}

void connection::on_timer(boost::system::error_code const& e)
{
  write_results();
  if(!write_timer_ || e)
    return;
  write_timer_->expires_from_now(boost::posix_time::seconds(write_timeout_));
  write_timer_->async_wait([this](boost::system::error_code const& e)
  {
    on_timer(e);
  });
}

void connection::write_results() const
{
  if(output_file_.empty())
    return;
  logger_ns::logger(logger_ns::message_type::M_INFO,
                    " writing results to file ", output_file_, " ...");
  std::vector<bin_values_struct> to_disk;
  to_disk.reserve(values_.size());
  std::transform(values_.begin(), values_.end(),
                 std::back_inserter(to_disk),
                 [](std::pair<value_type, counter_type> const& kv)
  {
    return bin_values_struct{kv.first, kv.second};
  });
  std::ofstream of(output_file_, std::ios::out);
  if(!of)
    return;
  auto const* buffer = reinterpret_cast<char const*>(to_disk.data());
  auto size = to_disk.size() * sizeof(bin_values_struct);
  of.write(buffer, size);
}

} // namespace server_ns
