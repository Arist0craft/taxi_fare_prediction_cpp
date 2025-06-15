#pragma once

#include <iterator>
#include <memory>
#include <stdexcept>


namespace http::utils {
    template <typename T>
    class RingBuffer {
    public:
        class iterator {
        public:
            iterator(RingBuffer* container, size_t pos, size_t steps)
                : container_(container), pos_(pos), steps_(steps) {};

            const T& operator*() const {
                return container_->data_[pos_];
            }

            iterator& operator++() {
                pos_ = (pos_ + 1) % container_->capacity_;
                ++steps_;
                return *this;
            }

            bool operator==(const iterator& other) const noexcept {
                return  pos_ == other.pos_ && steps_ == other.steps_;
            }

            bool operator!=(const iterator& other) const noexcept {
                return !(*this == other);
            }

        private:
            RingBuffer<T>* container_;
            size_t pos_;
            size_t steps_;
        };

        RingBuffer(size_t capacity)
            : data_(std::make_unique<T[]>(capacity)), capacity_(capacity) {};
        RingBuffer(const RingBuffer&) = delete;
        RingBuffer& operator=(const RingBuffer&) = delete;
        RingBuffer(RingBuffer&& other) noexcept = default;
        RingBuffer& operator=(RingBuffer&& other) noexcept = default;
    
        size_t Capacity() const noexcept {
            return capacity_;
        }

        size_t AvailableToRead() const noexcept {
            return size_;
        }

        size_t AvailableToWrite() const noexcept{
            return capacity_ - size_;
        }

        bool IsEmpty() const noexcept {
            return size_ == 0;
        }

        bool IsFull() const noexcept {
            return size_ == capacity_;
        }

        size_t Write(const T* input, size_t max_elements) {
            size_t written = 0;
            for (; written < max_elements && AvailableToWrite() > 0; ++written) {
                data_[write_pos_] = *(input + written);
                write_pos_ = (write_pos_ + 1) % capacity_;
                ++size_;
            }
            return written;
        }

        template <typename InputIt>
        size_t Write(InputIt begin, InputIt end) {
            size_t written = 0;
            for (; begin != end && AvailableToWrite() > 0; ++begin) {
                data_[write_pos_] = *begin;
                write_pos_ = (write_pos_ + 1) % capacity_;
                ++written;
                ++size_;
            }
            return written;
        }

        template <typename Container>
        size_t Write(const Container& container) {
            return Write(std::begin(container), std::end(container));
        }

        size_t Read(T* output, size_t max_elements) {
            size_t read = 0;
            for(; read < max_elements && AvailableToRead() > 0; ++read) {
                *(output + read) = data_[read_pos_];
                read_pos_ = (read_pos_ + 1) %  capacity_;
                --size_;
            }
            return read;
        }

        template <typename OutputIt>
        size_t Read(OutputIt it, size_t max_elements) {
            size_t read = 0;
            for (; read < max_elements && AvailableToRead() > 0; ++it) {
                *it = data_[read_pos_];
                read_pos_ = (read_pos_ + 1) %  capacity_;
                --size_;
                ++read;
            }
            return read;
        }

        void Clear() {
            size_ = 0;
            read_pos_ = 0;
            write_pos_ = 0;
        }

        size_t Skip(size_t count) {
            auto available_to_read = AvailableToRead();
            if (count > available_to_read) {
                count = available_to_read;
            }
            read_pos_ = (read_pos_ + count) % capacity_;
            size_ -= count;
            return  count;
        }

        const T& front() const {
            if (!IsEmpty()) {
                return data_[read_pos_];
            }
            throw std::runtime_error("Cannot call front() on empty buffer");
        }

        T& front() {
            return const_cast<T&>(static_cast<const RingBuffer&>(*this).front());
        }


        const T& back() const {
            if(!IsEmpty()) {
                return data_[(write_pos_ + capacity_ - 1) % capacity_];
            }
            throw std::runtime_error("Cannot call back() on empty buffer");
        }

        T& back() {
            return const_cast<T&>(static_cast<const RingBuffer&>(*this).back());
        }

        iterator begin() {
            return iterator(this, read_pos_, 0);
        }

        iterator end() {
            return iterator(this, write_pos_, size_);
        }

    private:
        std::unique_ptr<T[]> data_;
        size_t capacity_;
        size_t read_pos_ = 0;
        size_t write_pos_ = 0;
        size_t size_ = 0;
    };
} // namespace http::utils
