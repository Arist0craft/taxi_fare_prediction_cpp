#include <cstddef>
#include <list>
#include <gtest/gtest.h>
#include "utils/ring_buffer.hpp"

using http::utils::RingBuffer;

TEST(RingBufferTest, CapacityTest) {
    size_t capacity = 10;
    RingBuffer<int> buffer(capacity);

    EXPECT_EQ(buffer.Capacity(), capacity);
}

TEST(RingBufferTest, WriteAndReadSingleElement) {
    RingBuffer<int> buffer(3);
    int value = 42;
    EXPECT_EQ(buffer.Write(&value, 1), 1);

    int output = 0;
    EXPECT_EQ(buffer.Read(&output, 1), 1);
    EXPECT_EQ(output, 42);
}

TEST(RingBufferTest, WriteMultipleAndReadInOrder) {
    RingBuffer<int> buffer(3);
    int values[] = {1, 2, 3};
    EXPECT_EQ(buffer.Write(values, 3), 3);

    int output[3];
    EXPECT_EQ(buffer.Read(output, 3), 3);
    EXPECT_EQ(output[0], 1);
    EXPECT_EQ(output[1], 2);
    EXPECT_EQ(output[2], 3);
}

TEST(RingBufferTest, WriteMoreThanCapacity) {
    RingBuffer<int> buffer(2);
    std::vector<int> input = {1, 2, 3};
    EXPECT_EQ(buffer.Write(input), 2);
}

TEST(RingBufferTest, ReadFromEmptyBufferReturnsZero) {
    RingBuffer<int> buffer(2);
    int out = -1;
    EXPECT_EQ(buffer.Read(&out, 1), 0);
    EXPECT_EQ(out, -1); // не изменяется
}

TEST(RingBufferTest, FrontAndBackReturnCorrectValues) {
    RingBuffer<int> buffer(3);
    buffer.Write(std::vector<int>{10, 20, 30});
    EXPECT_EQ(buffer.front(), 10);
    EXPECT_EQ(buffer.back(), 30);
}

TEST(RingBufferTest, ClearResetsBuffer) {
    RingBuffer<int> buffer(3);
    buffer.Write(std::vector<int>{1, 2});
    buffer.Clear();
    EXPECT_TRUE(buffer.IsEmpty());
    int out = 0;
    EXPECT_EQ(buffer.Read(&out, 1), 0);
}

TEST(RingBufferTest, SkipAdvancesReadPosition) {
    RingBuffer<int> buffer(5);
    buffer.Write(std::vector<int>{1, 2, 3, 4});
    EXPECT_EQ(buffer.Skip(2), 2);
    int out[2];
    buffer.Read(out, 2);
    EXPECT_EQ(out[0], 3);
    EXPECT_EQ(out[1], 4);
}

TEST(RingBufferTest, WriteFromIteratorRange) {
    RingBuffer<int> buffer(3);
    std::list<int> input = {11, 12, 13};
    EXPECT_EQ(buffer.Write(input.begin(), input.end()), 3);
    int out[3];
    buffer.Read(out, 3);
    EXPECT_EQ(out[0], 11);
    EXPECT_EQ(out[1], 12);
    EXPECT_EQ(out[2], 13);
}

TEST(RingBufferTest, ReadToOutputIterator) {
    RingBuffer<int> buffer(3);
    buffer.Write(std::vector<int>{4, 5, 6});
    std::vector<int> result(3);
    EXPECT_EQ(buffer.Read(result.begin(), 3), 3);
    EXPECT_EQ(result[0], 4);
    EXPECT_EQ(result[1], 5);
    EXPECT_EQ(result[2], 6);
}

TEST(RingBufferTest, WriteUsingContainerOverload) {
    RingBuffer<int> buffer(3);
    std::array<int, 3> arr = {7, 8, 9};
    EXPECT_EQ(buffer.Write(arr), 3);
    int out[3];
    buffer.Read(out, 3);
    EXPECT_EQ(out[0], 7);
    EXPECT_EQ(out[1], 8);
    EXPECT_EQ(out[2], 9);
}

TEST(RingBufferTest, IteratorWorksCorrectly) {
    RingBuffer<int> buffer(3);
    buffer.Write(std::vector<int>{100, 200});
    std::vector<int> result;
    for (auto it = buffer.begin(); it != buffer.end(); ++it) {
        result.push_back(*it);
    }
    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 100);
    EXPECT_EQ(result[1], 200);
}

TEST(RingBufferTest, FrontAndBackThrowOnEmpty) {
    RingBuffer<int> buffer(2);
    EXPECT_THROW(buffer.front(), std::runtime_error);
    EXPECT_THROW(buffer.back(), std::runtime_error);
}

TEST(RingBufferTest, IteratorAdditionSkipsElementsCorrectly) {
    RingBuffer<int> buffer(5);  // capacity = 5

    // Пишем данные: 10, 20, 30
    int input[] = {10, 20, 30};
    buffer.Write(input, 3);

    // Получаем итератор на начало
    auto it = buffer.begin();

    // Сдвигаем на 2
    auto it2 = it + 2;

    // Проверяем, что значение верное (30)
    EXPECT_EQ(*it2, 30);

    // Проверяем, что it + 3 == end()
    auto it3 = it + 3;
    EXPECT_EQ(it3, buffer.end());

    // Проверяем, что it + 4 не выйдет за буфер
    auto it4 = it + 10;  // будет обрезано до size == 3
    EXPECT_EQ(it4, buffer.end());
}

TEST(RingBufferTest, HandlesWrapAroundCorrectly) {
    RingBuffer<int> buffer(3);
    
    // Заполняем буфер полностью
    EXPECT_EQ(buffer.Write(std::vector<int>{1, 2, 3}), 3);

    // Читаем два элемента → read_pos_ = 2
    std::vector<int> first_read(2);
    EXPECT_EQ(buffer.Read(first_read.begin(), 2), 2);
    EXPECT_EQ(first_read[0], 1);
    EXPECT_EQ(first_read[1], 2);

    // Записываем ещё два → write_pos_ должен обернуться (wrap-around)
    EXPECT_EQ(buffer.Write(std::vector<int>{4, 5}), 2);

    // Читаем оставшиеся
    std::vector<int> second_read(3);
    EXPECT_EQ(buffer.Read(second_read.begin(), 3), 3);
    EXPECT_EQ(second_read[0], 3); // последний из первой тройки
    EXPECT_EQ(second_read[1], 4);
    EXPECT_EQ(second_read[2], 5);
}

TEST(RingBufferTest, IteratorWorksWithStdSearch) {
    RingBuffer<char> buffer(10);
    
    // Записываем данные
    std::string data = "hello\r\nworld";
    buffer.Write(data.begin(), data.end());
    
    // Ищем \r\n
    std::array<char, 2> crlf = {'\r', '\n'};
    auto it = std::search(buffer.begin(), buffer.end(), crlf.begin(), crlf.end());
    
    EXPECT_NE(it, buffer.end());
    EXPECT_EQ(*it, '\r');
}

TEST(RingBufferTest, IteratorWorksWithStdSearchWrapped) {
    RingBuffer<char> buffer(8);
    
    // Записываем данные так, чтобы они "обернулись" вокруг буфера
    std::string data1 = "hello";
    std::string data2 = "\r\nworld";
    
    buffer.Write(data1.begin(), data1.end());
    buffer.Skip(3); // Освобождаем место в начале
    buffer.Write(data2.begin(), data2.end());
    
    // Теперь данные: "lo\r\nwo"
    std::array<char, 2> crlf = {'\r', '\n'};
    auto it = std::search(buffer.begin(), buffer.end(), crlf.begin(), crlf.end());
    
    EXPECT_NE(it, buffer.end());
    EXPECT_EQ(*it, '\r');
}

TEST(RingBufferTest, IteratorWorksWithStdSearchNotFound) {
    RingBuffer<char> buffer(10);
    
    // Записываем данные без \r\n
    std::string data = "hello world";
    buffer.Write(data.begin(), data.end());
    
    // Ищем \r\n
    std::array<char, 2> crlf = {'\r', '\n'};
    auto it = std::search(buffer.begin(), buffer.end(), crlf.begin(), crlf.end());
    
    EXPECT_EQ(it, buffer.end());
}

TEST(RingBufferTest, IteratorWorksWithStdSearchEmptyBuffer) {
    RingBuffer<char> buffer(10);
    
    // Пустой буфер
    std::array<char, 2> crlf = {'\r', '\n'};
    auto it = std::search(buffer.begin(), buffer.end(), crlf.begin(), crlf.end());
    
    EXPECT_EQ(it, buffer.end());
}

TEST(RingBufferTest, IteratorWorksWithStdSearchPartialMatch) {
    RingBuffer<char> buffer(10);
    
    // Записываем данные с частичным совпадением
    std::string data = "hello\rworld";
    buffer.Write(data.begin(), data.end());
    
    // Ищем \r\n (найдем только \r)
    std::array<char, 2> crlf = {'\r', '\n'};
    auto it = std::search(buffer.begin(), buffer.end(), crlf.begin(), crlf.end());
    
    EXPECT_EQ(it, buffer.end()); // Не должно найти полное совпадение
}

TEST(RingBufferTest, IteratorWorksWithStdSearchAtEnd) {
    RingBuffer<char> buffer(10);
    
    // Записываем данные с \r\n в конце
    std::string data = "hello\r\n";
    buffer.Write(data.begin(), data.end());
    
    std::array<char, 2> crlf = {'\r', '\n'};
    auto it = std::search(buffer.begin(), buffer.end(), crlf.begin(), crlf.end());
    
    EXPECT_NE(it, buffer.end());
    EXPECT_EQ(*it, '\r');
}

TEST(RingBufferTest, IteratorWorksWithStdSearchAtBeginning) {
    RingBuffer<char> buffer(10);
    
    // Записываем данные с \r\n в начале
    std::string data = "\r\nhello";
    buffer.Write(data.begin(), data.end());
    
    std::array<char, 2> crlf = {'\r', '\n'};
    auto it = std::search(buffer.begin(), buffer.end(), crlf.begin(), crlf.end());
    
    EXPECT_NE(it, buffer.end());
    EXPECT_EQ(*it, '\r');
}
