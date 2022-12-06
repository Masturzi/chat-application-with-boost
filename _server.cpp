#include <iostream>
#include <string>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

int main()
{
  try
  {
    // Create the io_context object that will manage all of our network communication.
    boost::asio::io_context io_context;

    // Create a tcp::acceptor object and use it to listen for incoming connections on the specified port.
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 1337));

    while (true)
    {
      // Create a tcp::socket object to hold the incoming connection.
      tcp::socket socket(io_context);

      // Use the acceptor's accept function to wait for an incoming connection.
      acceptor.accept(socket);

      // Create a std::array to hold the data received from the client.
      std::array<char, 128> buffer;

      // Use the boost::asio::read function to read data from the client into the buffer.
      size_t bytes_transferred = boost::asio::read(socket, boost::asio::buffer(buffer));

      // Print out the message received from the client.
      std::cout << "Received message from client: ";
      std::cout.write(buffer.data(), bytes_transferred);
      std::cout << std::endl;

      // Create a std::string to hold the message we want to send back to the client.
      std::string message = "Hello, client!";

      // Use the boost::asio::write function to send the message back to the client.
      boost::asio::write(socket, boost::asio::buffer(message));
    }
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
