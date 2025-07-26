#pragma once

#include <cstddef>
#include <iterator>
#include <memory>
#include <stdexcept>

/**
 * @brief Кольцевой буфер для эффективного управления данными
 * 
 * RingBuffer предоставляет эффективную реализацию кольцевого буфера
 * с поддержкой STL-совместимых итераторов. Буфер автоматически
 * перезаписывает старые данные при переполнении.
 * 
 * @tparam T Тип элементов, хранимых в буфере
 */
namespace http::utils {
    template <typename T>
    class RingBuffer {
    public:
        /**
         * @brief Итератор для обхода элементов буфера
         * 
         * Поддерживает стандартные операции итератора: разыменование,
         * инкремент, сравнение. Итератор корректно работает с
         * кольцевой структурой буфера.
         */
        class iterator {
        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = T;
            using difference_type = std::ptrdiff_t;
            using pointer = T*;
            using reference = T&;

            /**
             * @brief Конструктор итератора
             * @param container Указатель на контейнер
             * @param pos Текущая позиция в буфере
             * @param steps Количество шагов от начала
             */
            iterator(RingBuffer* container, size_t pos, size_t steps)
                : container_(container), pos_(pos), steps_(steps) {};

            /**
             * @brief Разыменование итератора
             * @return Ссылка на элемент
             */
            T& operator*() {
                return container_->data_[pos_];
            }

            /**
             * @brief Разыменование итератора
             * @return Константная ссылка на элемент
             */
            const T& operator*() const {
                return container_->data_[pos_];
            }

            /**
             * @brief Префиксный инкремент
             * @return Ссылка на итератор
             */
            iterator& operator++() {
                pos_ = (pos_ + 1) % container_->capacity_;
                ++steps_;
                return *this;
            }

            /**
             * @brief Сложение итератора с числом
             * @param n Количество позиций для сдвига
             * @return Новый итератор
             */
            iterator operator+(size_t n) const {
                if (n > container_->size_) {
                    n = container_->size_;
                }

                return iterator(
                    container_, (pos_ + n) % container_->capacity_, steps_ + n
                );
            }

            /**
             * @brief Вычитание итераторов для std::difference
             * @param other Второй итератор для подсчёта разницы
             * @return Кол-во шагов разницы
             */
            difference_type operator-(const iterator& other) const {
                return static_cast<difference_type>(steps_) - static_cast<difference_type>(other.steps_);
            }

            /**
             * @brief Сравнение итераторов
             * @param other Итератор для сравнения
             * @return true если итераторы равны
             */
            bool operator==(const iterator& other) const noexcept {
                return  pos_ == other.pos_ && steps_ == other.steps_;
            }

            /**
             * @brief Неравенство итераторов
             * @param other Итератор для сравнения
             * @return true если итераторы не равны
             */
            bool operator!=(const iterator& other) const noexcept {
                return !(*this == other);
            }

        private:
            RingBuffer<T>* container_;
            size_t pos_;
            size_t steps_;
        };

        /**
         * @brief Конструктор буфера
         * @param capacity Максимальная емкость буфера
         */
        RingBuffer(size_t capacity)
            : data_(std::make_unique<T[]>(capacity)), capacity_(capacity) {};
        
        /**
         * @brief Запрет копирования
         */
        RingBuffer(const RingBuffer&) = delete;
        
        /**
         * @brief Запрет присваивания
         */
        RingBuffer& operator=(const RingBuffer&) = delete;
        
        /**
         * @brief Перемещающий конструктор
         */
        RingBuffer(RingBuffer&& other) noexcept = default;
        
        /**
         * @brief Перемещающий оператор присваивания
         */
        RingBuffer& operator=(RingBuffer&& other) noexcept = default;
    
        /**
         * @brief Получить максимальную емкость буфера
         * @return Емкость буфера
         */
        size_t Capacity() const noexcept {
            return capacity_;
        }

        /**
         * @brief Получить количество элементов для чтения
         * @return Количество доступных для чтения элементов
         */
        size_t GetReadSize() const noexcept {
            return size_;
        }

        /**
         * @brief Получить количество свободного места для записи
         * @return Количество элементов, которые можно записать
         */
        size_t GetWriteSize() const noexcept{
            return capacity_ - size_;
        }

        /**
         * @brief Получить количество элементов для линейной записи
         * 
         * Возвращает количество элементов, которые можно записать
         * непрерывно от текущей позиции записи.
         * 
         * @return Количество элементов для линейной записи
         */
        size_t GetLinearWriteSize() const noexcept {
            if (IsFull()) return 0;
            
            if (write_pos_ < read_pos_) {
                return  read_pos_ - write_pos_;
            }
            return capacity_ - write_pos_;
        }

        /**
         * @brief Проверить, пуст ли буфер
         * @return true если буфер пуст
         */
        bool IsEmpty() const noexcept {
            return size_ == 0;
        }

        /**
         * @brief Проверить, полон ли буфер
         * @return true если буфер полон
         */
        bool IsFull() const noexcept {
            return size_ == capacity_;
        }

        /**
         * @brief Получить указатель на позицию записи
         * 
         * @warning Указатель действителен только до следующего вызова
         * методов, изменяющих состояние буфера (CommitWrite, Write, Clear и т.д.)
         * 
         * @return Указатель на позицию записи
         */
        T* GetWritePtr() {
            return data_.get() + write_pos_;
        }

        /**
         * @brief Подтвердить запись данных
         * 
         * Используется после прямой записи в буфер через GetWritePtr().
         * Обновляет позицию записи и размер буфера.
         * 
         * @param count Количество записанных элементов
         */
        void CommitWrite(size_t count) {
            if (count > GetWriteSize()) {
                throw std::runtime_error("Cannot commit more than write size");
            }
            write_pos_ = (write_pos_ + count) % capacity_;
            size_ += count;
        }

        /**
         * @brief Записать данные из массива
         * @param input Указатель на входные данные
         * @param max_elements Максимальное количество элементов для записи
         * @return Количество фактически записанных элементов
         */
        size_t Write(const T* input, size_t max_elements) {
            size_t written = 0;
            for (; written < max_elements && GetWriteSize() > 0; ++written) {
                data_[write_pos_] = *(input + written);
                CommitWrite(1);
            }
            return written;
        }

        /**
         * @brief Записать данные из итераторов
         * @param begin Начальный итератор
         * @param end Конечный итератор
         * @return Количество фактически записанных элементов
         */
        template <typename InputIt>
        size_t Write(InputIt begin, InputIt end) {
            size_t written = 0;
            for (; begin != end && GetWriteSize() > 0; ++begin) {
                data_[write_pos_] = *begin;
                CommitWrite(1);
                ++written;
            }
            return written;
        }

        /**
         * @brief Записать данные из контейнера
         * @param container Контейнер с данными
         * @return Количество фактически записанных элементов
         */
        template <typename Container>
        size_t Write(const Container& container) {
            return Write(std::begin(container), std::end(container));
        }

        /**
         * @brief Читать данные в массив
         * @param output Указатель на выходной массив
         * @param max_elements Максимальное количество элементов для чтения
         * @return Количество фактически прочитанных элементов
         */
        size_t Read(T* output, size_t max_elements) {
            size_t read = 0;
            for(; read < max_elements && GetReadSize() > 0; ++read) {
                *(output + read) = data_[read_pos_];
                read_pos_ = (read_pos_ + 1) %  capacity_;
                --size_;
            }
            return read;
        }

        /**
         * @brief Читать данные в итератор
         * @param it Выходной итератор
         * @param max_elements Максимальное количество элементов для чтения
         * @return Количество фактически прочитанных элементов
         */
        template <typename OutputIt>
        size_t Read(OutputIt it, size_t max_elements) {
            size_t read = 0;
            for (; read < max_elements && GetReadSize() > 0; ++it) {
                *it = data_[read_pos_];
                read_pos_ = (read_pos_ + 1) %  capacity_;
                --size_;
                ++read;
            }
            return read;
        }

        /**
         * @brief Очистить буфер
         * 
         * Сбрасывает все позиции и размер. Все данные теряются.
         */
        void Clear() {
            size_ = 0;
            read_pos_ = 0;
            write_pos_ = 0;
        }

        /**
         * @brief Пропустить элементы при чтении
         * @param count Количество элементов для пропуска
         * @return Количество фактически пропущенных элементов
         */
        size_t Skip(size_t count) {
            auto available_to_read = GetReadSize();
            if (count > available_to_read) {
                count = available_to_read;
            }
            read_pos_ = (read_pos_ + count) % capacity_;
            size_ -= count;
            return  count;
        }

        /**
         * @brief Получить первый элемент (константная версия)
         * @return Константная ссылка на первый элемент
         * @throws std::runtime_error если буфер пуст
         */
        const T& front() const {
            if (!IsEmpty()) {
                return data_[read_pos_];
            }
            throw std::runtime_error("Cannot call front() on empty buffer");
        }

        /**
         * @brief Получить первый элемент
         * @return Ссылка на первый элемент
         * @throws std::runtime_error если буфер пуст
         */
        T& front() {
            return const_cast<T&>(static_cast<const RingBuffer&>(*this).front());
        }

        /**
         * @brief Получить последний элемент (константная версия)
         * @return Константная ссылка на последний элемент
         * @throws std::runtime_error если буфер пуст
         */
        const T& back() const {
            if(!IsEmpty()) {
                return data_[(write_pos_ + capacity_ - 1) % capacity_];
            }
            throw std::runtime_error("Cannot call back() on empty buffer");
        }

        /**
         * @brief Получить последний элемент
         * @return Ссылка на последний элемент
         * @throws std::runtime_error если буфер пуст
         */
        T& back() {
            return const_cast<T&>(static_cast<const RingBuffer&>(*this).back());
        }

        /**
         * @brief Получить итератор на начало
         * @return Итератор на первый элемент
         */
        iterator begin() {
            return iterator(this, read_pos_, 0);
        }

        /**
         * @brief Получить итератор на конец
         * @return Итератор за последним элементом
         */
        iterator end() {
            return iterator(this, write_pos_, size_);
        }

    private:
        std::unique_ptr<T[]> data_;  ///< Массив данных
        size_t capacity_;            ///< Максимальная емкость
        size_t read_pos_ = 0;        ///< Позиция чтения
        size_t write_pos_ = 0;       ///< Позиция записи
        size_t size_ = 0;            ///< Текущий размер
    };
} // namespace http::utils
