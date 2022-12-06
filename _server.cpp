#include <iostream>
#include <string>
#include <array>
#include <thread>
#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/log/trivial.hpp>

using boost::asio::ip::tcp;

class ChatSession : public std::enable_shared_from_this<ChatSession>
{
public:
  ChatSession(tcp::socket socket)
    : socket_(std::move(socket))
  {
  }

  void start()
  {
    do_read_header();
  }

private:
  void do_read_header()
  {
    auto self(shared_from_this());
    boost::asio::async_read(socket_,
                            boost::asio::buffer(header_, header_length),
                            [this, self](boost::system::error_code ec, std::size_t)
                            {
                              if (!ec)
                              {
                                do_read_body();
                              }
                            });
  }

  void do_read_body()
  {
    auto self(shared_from_this());
    boost::asio::async_read(socket_,
                            boost::asio::buffer(body_, body_length_),
                            [this, self](boost::system::error_code ec, std::size_t)
                            {
                              if (!ec)
                              {
                                do_handle_message();
                              }
                            });
  }

  void do_handle_message()
  {
    std::string message(body_, body_length_);
    BOOST_LOG_TRIVIAL(info) << "Received message from client: " << message;

    boost::property_tree::ptree pt;
    std::istringstream is(message);
    boost::property_tree::read_json(is, pt);

    std::string recipient = pt.get<std::string>("recipient");
    std::string text = pt.get<std::string>("text");

    if (recipient == "server")
    {
      std::string response = "Hello, client!";
      BOOST_LOG_TRIVIAL(info) << "Sending response to client: " << response;

      boost::property_tree::ptree response_pt;
      response_pt.put("sender", "server");
      response_pt.put("text", response);

      std::ostringstream os;
      boost::property_tree::write_json(os, response_pt);
      std::string response_message = os.str();

      do_write(response_message);
    }

    do_read_header();
  }

  void do_write(std::string message)
  {
    auto self(shared_from_this());
    > 
    boost::asio::async_write(socket_,
                             boost::asio::buffer(message),
                             [this, self](boost::system::error_code ec, std::size_t)
                             {
                               if (!ec)
                               {
                                 do_read_header();
                               }
                             });
  }

  tcp::socket socket_;
  std::array<char, 4> header_;
  uint32_t body_length_ = 0;
  std::array<char, 1024> body_;
};

class ChatServer
{
public:
  ChatServer(boost::asio::io_context& io_context, short port)
    : acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
  {
    do_accept();
  }

private:
  void do_accept()
  {
    acceptor_.async_accept(
      [this](boost::system::error_code ec, tcp::socket socket)
      {
        if (!ec)
        {
          BOOST_LOG_TRIVIAL(info) << "Accepted connection from client";
          std::make_shared<ChatSession>(std::move(socket))->start();
        }

        do_accept();
      });
  }

  tcp::acceptor acceptor_;
};

int main()
{
  try
  {
    // Create the io_context object that will manage all of our network communication.
    boost::asio::io_context io_context;

    // Create a chat server and start accepting connections.
    ChatServer server(io_context, 1337);

    // Create a separate thread to run the io_context.
    std::thread thread([&io_context]() { io_context.run(); });

    // Wait for the thread to finish.
    thread.join();
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}

