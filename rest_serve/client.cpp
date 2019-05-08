/// Example program demonstrates sending json messages using boost::asio
/// g++ client.cpp -o sync_http_client -lboost_system -lpthread

#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

int client1(std::string json)
{
  try
  {
    boost::asio::io_service io_service;

    // Get a list of endpoints corresponding to the server name.
    tcp::resolver resolver(io_service);
    tcp::resolver::query query("virt5.tra.ai", "8081");
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

    // Try each endpoint until we successfully establish a connection.
    tcp::socket socket(io_service);
    boost::asio::connect(socket, endpoint_iterator);

    // Form the request. We specify the "Connection: close" header so that the
    // server will close the socket after transmitting the response. This will
    // allow us to treat all data up until the EOF as the content.
    boost::asio::streambuf request;
    std::ostream request_stream(&request);

    request_stream << "POST /readData HTTP/1.1\r\n";
    request_stream << "Host: virt5.tra.ai:8081\r\n";
    //request_stream << "User-Agent: C/1.0\r\n";
    request_stream << "Content-Type: application/json; charset=utf-8 \r\n";
    request_stream << "Accept: */*\r\n";
    request_stream << "Content-Length: " << json.length() << "\r\n";
    request_stream << "Connection: close\r\n\r\n";
    request_stream << json;


    // Send the request.
    boost::asio::write(socket, request);

    // Read the response status line. The response streambuf will automatically
    // grow to accommodate the entire line. The growth may be limited by passing
    // a maximum size to the streambuf constructor.
    boost::asio::streambuf response;


    // Read until EOF, writing data to output as we go.
    boost::system::error_code error;
    while (boost::asio::read(socket, response, boost::asio::transfer_at_least(1), error))
    {
      std::cout << &response;

      //auto first = (boost::asio::buffer_cast<const char*>(response.data()));
      //auto last = first + (boost::asio::buffer_size(response.data()));
      //std::copy(first, last, std::ostream_iterator<char>(std::cout));
      //std::cout << std::endl;
    }
    if (error != boost::asio::error::eof)
    {
      throw boost::system::system_error(error);
    }
    return 0;
  }
  catch (std::exception& e)
  {
    std::cout << "Exception: " << e.what() << "\n";
  }
  return -1;
}

/// From https://stackoverflow.com/questions/10982717/get-html-without-header-with-boostasio
///
int client2(std::string host_,std::string port_, std::string url_path, const std::string& indata, std::ostream &out_,std::vector<std::string> &headers)
{
    try{
        using namespace boost::asio::ip;
        tcp::iostream request_stream;
        request_stream.connect(host_,port_);
        if(!request_stream){
            return -1;
        }
        request_stream << "POST " << url_path << " HTTP/1.1\r\n";
        request_stream << "Host: " << host_ << "\r\n";
        request_stream << "Content-Type: application/json; charset=utf-8 \r\n";
        request_stream << "Accept: */*\r\n";
        request_stream << "Content-Length: " << indata.length() << "\r\n";
        request_stream << "Connection: close\r\n\r\n";
        request_stream << indata;
        request_stream.flush();

        std::string line1;
        std::getline(request_stream,line1);
        if (!request_stream)
        {
            return -2;
        }

        std::stringstream response_stream(line1);
        std::string http_version;
        response_stream >> http_version;
        unsigned int status_code;
        response_stream >> status_code;
        std::string status_message;
        std::getline(response_stream,status_message);
        //if (!response_stream||http_version.substr(0,5)!="HTTP/")
        //{
        //   return -1;
        //}
        //if (status_code!=200)
        //{
        //    return (int)status_code;
        //}
        std::string header;
        while (std::getline(request_stream, header) && header != "\r")
        {
            headers.push_back(header);
        }
        out_ << request_stream.rdbuf();
        return status_code;
    }catch(std::exception &e){
        std::cout << e.what() << std::endl;
        return -3;
    }
}

int main()
{
  const std::string json = "{\"id\":\"hive_bot_integration_obj_id\", \"state\" : \"GoToCell\"}";

  //client1(json);

  std::ostringstream out;
  std::vector<std::string> header;
  auto ret = client2("virt5.tra.ai", "8081", "/readData", json, out, header );
  std::cout << out.str() << "\n";
  return 0;
}
