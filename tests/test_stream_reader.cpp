#include <cstddef>
#include <string>
#include <thread>
#include <chrono>
#include <gtest/gtest.h>
#include <cstdint>

#include "socket.hpp"
#include "stream/reader.hpp"
#include "stream/error.hpp"

using http::socket_wrapper::Socket;
using http::stream::StreamReader;
using http::stream::StreamError;

TEST(StreamReaderTest, ReadBytesTest) {
    std::string test_data = "Hello, World!";
    std::string host = "127.0.0.1";
    uint16_t actual_port = 0;
    
    std::thread server_thread([&]{
        Socket server_sock;
        server_sock.Bind(0, host); // Порт 0 - система выберет свободный
        server_sock.Listen();
        
        // Получаем реальный порт
        auto [server_ip, server_port] = server_sock.GetLocalAddress();
        actual_port = server_port;
        
        std::string client_host;
        uint16_t client_port = 0;
        Socket client_socket = server_sock.Accept(client_host, client_port);
        
        client_socket.Send(test_data);
    });

    std::thread client_thread([&]{
        // Ждем, пока сервер получит порт
        while (actual_port == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        
        Socket client_sock;
        client_sock.Connect(host, actual_port);
        
        StreamReader reader(client_sock);
        std::string result = reader.ReadBytes(test_data.size());
        
        EXPECT_EQ(result, test_data);
    });

    server_thread.join();
    client_thread.join();
}

TEST(StreamReaderTest, ReadLineTest) {
    std::string test_line = "HTTP/1.1 200 OK";
    std::string test_data = test_line + "\r\n";
    std::string host = "127.0.0.1";
    uint16_t actual_port = 0;
    
    std::thread server_thread([&]{
        Socket server_sock;
        server_sock.Bind(0, host);
        server_sock.Listen();
        
        auto [server_ip, server_port] = server_sock.GetLocalAddress();
        actual_port = server_port;
        
        std::string client_host;
        uint16_t client_port = 0;
        Socket client_socket = server_sock.Accept(client_host, client_port);
        
        client_socket.Send(test_data);
    });

    std::thread client_thread([&]{
        while (actual_port == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        
        Socket client_sock;
        client_sock.Connect(host, actual_port);
        
        StreamReader reader(client_sock);
        std::string result = reader.ReadLine();
        
        EXPECT_EQ(result, test_line);
    });

    server_thread.join();
    client_thread.join();
}

TEST(StreamReaderTest, ReadUntilTest) {
    std::string before_delimiter = "Content-Type: application/json";
    std::string delimiter = "\r\n\r\n";
    std::string after_delimiter = "{'key': 'value'}";
    std::string test_data = before_delimiter + delimiter + after_delimiter;
    std::string host = "127.0.0.1";
    uint16_t actual_port = 0;
    
    std::thread server_thread([&]{
        Socket server_sock;
        server_sock.Bind(0, host);
        server_sock.Listen();
        
        auto [server_ip, server_port] = server_sock.GetLocalAddress();
        actual_port = server_port;
        
        std::string client_host;
        uint16_t client_port = 0;
        Socket client_socket = server_sock.Accept(client_host, client_port);
        
        client_socket.Send(test_data);
    });

    std::thread client_thread([&]{
        while (actual_port == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        
        Socket client_sock;
        client_sock.Connect(host, actual_port);
        
        StreamReader reader(client_sock);
        std::string result = reader.ReadUntil(delimiter);
        
        EXPECT_EQ(result, before_delimiter);
    });

    server_thread.join();
    client_thread.join();
}

TEST(StreamReaderTest, PeekTest) {
    std::string test_data = "GET /api/test HTTP/1.1\r\n";
    std::string host = "127.0.0.1";
    uint16_t actual_port = 0;
    
    std::thread server_thread([&]{
        Socket server_sock;
        server_sock.Bind(0, host);
        server_sock.Listen();
        
        auto [server_ip, server_port] = server_sock.GetLocalAddress();
        actual_port = server_port;
        
        std::string client_host;
        uint16_t client_port = 0;
        Socket client_socket = server_sock.Accept(client_host, client_port);
        
        client_socket.Send(test_data);
    });

    std::thread client_thread([&]{
        while (actual_port == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        
        Socket client_sock;
        client_sock.Connect(host, actual_port);
        
        StreamReader reader(client_sock);
        
        // Peek первые 3 символа
        std::string peeked = reader.Peek(3);
        EXPECT_EQ(peeked, "GET");
        
        // Peek первые 4 символа
        std::string peeked4 = reader.Peek(4);
        EXPECT_EQ(peeked4, "GET ");
        
        // Убеждаемся, что данные не были извлечены
        std::string actual = reader.ReadBytes(3);
        EXPECT_EQ(actual, "GET");
    });

    server_thread.join();
    client_thread.join();
}

TEST(StreamReaderTest, SkipTest) {
    std::string test_data = "SKIP_THIS_PART_READ_THIS";
    std::string host = "127.0.0.1";
    uint16_t actual_port = 0;
    
    std::thread server_thread([&]{
        Socket server_sock;
        server_sock.Bind(0, host);
        server_sock.Listen();
        
        auto [server_ip, server_port] = server_sock.GetLocalAddress();
        actual_port = server_port;
        
        std::string client_host;
        uint16_t client_port = 0;
        Socket client_socket = server_sock.Accept(client_host, client_port);
        
        client_socket.Send(test_data);
    });

    std::thread client_thread([&]{
        while (actual_port == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        
        Socket client_sock;
        client_sock.Connect(host, actual_port);
        
        StreamReader reader(client_sock);
        
        // Пропускаем первые 15 символов "SKIP_THIS_PART_"
        std::string skipped_count = reader.Skip(15);
        EXPECT_EQ(skipped_count, "15");
        
        // Читаем оставшуюся часть
        std::string result = reader.ReadBytes(9); // "READ_THIS"
        EXPECT_EQ(result, "READ_THIS");
    });

    server_thread.join();
    client_thread.join();
}

TEST(StreamReaderTest, ReadBytesInsufficientDataTest) {
    std::string test_data = "Short";
    std::string host = "127.0.0.1";
    uint16_t actual_port = 0;
    
    std::thread server_thread([&]{
        Socket server_sock;
        server_sock.Bind(0, host);
        server_sock.Listen();
        
        auto [server_ip, server_port] = server_sock.GetLocalAddress();
        actual_port = server_port;
        
        std::string client_host;
        uint16_t client_port = 0;
        Socket client_socket = server_sock.Accept(client_host, client_port);
        
        client_socket.Send(test_data);
        // Закрываем соединение, чтобы симулировать конец данных
    });

    std::thread client_thread([&]{
        while (actual_port == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        
        Socket client_sock;
        client_sock.Connect(host, actual_port);
        
        StreamReader reader(client_sock);
        
        // Пытаемся прочитать больше данных, чем доступно
        EXPECT_THROW(reader.ReadBytes(100), StreamError);
    });

    server_thread.join();
    client_thread.join();
}

TEST(StreamReaderTest, ReadUntilNotFoundTest) {
    std::string test_data = "No delimiter here";
    std::string host = "127.0.0.1";
    uint16_t actual_port = 0;
    
    std::thread server_thread([&]{
        Socket server_sock;
        server_sock.Bind(0, host);
        server_sock.Listen();
        
        auto [server_ip, server_port] = server_sock.GetLocalAddress();
        actual_port = server_port;
        
        std::string client_host;
        uint16_t client_port = 0;
        Socket client_socket = server_sock.Accept(client_host, client_port);
        
        client_socket.Send(test_data);
        // Закрываем соединение
    });

    std::thread client_thread([&]{
        while (actual_port == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        
        Socket client_sock;
        client_sock.Connect(host, actual_port);
        
        StreamReader reader(client_sock);
        
        // Ищем несуществующий разделитель
        EXPECT_THROW(reader.ReadUntil("\r\n\r\n"), StreamError);
    });

    server_thread.join();
    client_thread.join();
}

TEST(StreamReaderTest, PeekPartialDataTest) {
    std::string test_data = "Hi";
    std::string host = "127.0.0.1";
    uint16_t actual_port = 0;
    
    std::thread server_thread([&]{
        Socket server_sock;
        server_sock.Bind(0, host);
        server_sock.Listen();
        
        auto [server_ip, server_port] = server_sock.GetLocalAddress();
        actual_port = server_port;
        
        std::string client_host;
        uint16_t client_port = 0;
        Socket client_socket = server_sock.Accept(client_host, client_port);
        
        client_socket.Send(test_data);
        // Закрываем соединение
    });

    std::thread client_thread([&]{
        while (actual_port == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        
        Socket client_sock;
        client_sock.Connect(host, actual_port);
        
        StreamReader reader(client_sock);
        
        // Пытаемся peek больше данных, чем доступно
        std::string result = reader.Peek(10);
        EXPECT_EQ(result, "Hi"); // Должно вернуть только доступные данные
        EXPECT_EQ(result.size(), 2);
    });

    server_thread.join();
    client_thread.join();
}

TEST(StreamReaderTest, MultipleOperationsTest) {
    std::string test_data = "GET /api HTTP/1.1\r\nHost: example.com\r\n\r\nBody content";
    std::string host = "127.0.0.1";
    uint16_t actual_port = 0;
    
    std::thread server_thread([&]{
        Socket server_sock;
        server_sock.Bind(0, host);
        server_sock.Listen();
        
        auto [server_ip, server_port] = server_sock.GetLocalAddress();
        actual_port = server_port;
        
        std::string client_host;
        uint16_t client_port = 0;
        Socket client_socket = server_sock.Accept(client_host, client_port);
        
        client_socket.Send(test_data);
    });

    std::thread client_thread([&]{
        while (actual_port == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        
        Socket client_sock;
        client_sock.Connect(host, actual_port);
        
        StreamReader reader(client_sock);
        
        // Peek метод
        std::string method = reader.Peek(3);
        EXPECT_EQ(method, "GET");
        
        // Читаем первую строку
        std::string first_line = reader.ReadLine();
        EXPECT_EQ(first_line, "GET /api HTTP/1.1");
        
        // Читаем вторую строку
        std::string second_line = reader.ReadLine();
        EXPECT_EQ(second_line, "Host: example.com");
        
        // Читаем пустую строку (разделитель заголовков и тела)
        std::string empty_line = reader.ReadLine();
        EXPECT_EQ(empty_line, "");
        
        // Читаем тело
        std::string body = reader.ReadBytes(12); // "Body content"
        EXPECT_EQ(body, "Body content");
    });

    server_thread.join();
    client_thread.join();
}

TEST(StreamReaderTest, LargeDataTest) {
    // Тест с большим объемом данных
    std::string large_data(8000, 'A'); // 8KB данных
    large_data += "\r\n";
    std::string host = "127.0.0.1";
    uint16_t actual_port = 0;
    
    std::thread server_thread([&]{
        Socket server_sock;
        server_sock.Bind(0, host);
        server_sock.Listen();
        
        auto [server_ip, server_port] = server_sock.GetLocalAddress();
        actual_port = server_port;
        
        std::string client_host;
        uint16_t client_port = 0;
        Socket client_socket = server_sock.Accept(client_host, client_port);
        
        client_socket.Send(large_data);
    });

    std::thread client_thread([&]{
        while (actual_port == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        
        Socket client_sock;
        client_sock.Connect(host, actual_port);
        
        StreamReader reader(client_sock);
        
        std::string result = reader.ReadLine();
        EXPECT_EQ(result.size(), 8000);
        EXPECT_EQ(result, std::string(8000, 'A'));
    });

    server_thread.join();
    client_thread.join();
}
