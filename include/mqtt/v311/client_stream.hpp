//          Copyright Andreas Wass 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <mqtt/stream.hpp>

#include "details/handshake.hpp"

#include "connect_opts.hpp"

#include <boost/asio/steady_timer.hpp>

namespace mqtt {
namespace v311 {

template<class NextLayer> class client_stream {
    mqtt::stream<NextLayer> next_;

public:
    using executor_type = typename mqtt::stream<NextLayer>::executor_type;
    using next_layer_type = typename mqtt::stream<NextLayer>::next_layer_type;

    template<class... Args>
    client_stream(std::size_t read_buffer_size, std::size_t write_buffer_size, Args &&...args)
        : next_(read_buffer_size, write_buffer_size, std::forward<Args>(args)...) {
    }

    template<class... Args> client_stream(Args &&...args) : next_(std::forward<Args>(args)...) {
    }

    executor_type get_executor() {
        return next_.get_executor();
    }

    next_layer_type &next_layer() {
        return next_.next_layer();
    }

    const next_layer_type &next_layer() const {
        return next_.next_layer();
    }

    template<class WriteHandler = boost::asio::default_completion_token_t<executor_type>>
    mqtt::async_result_t<WriteHandler, boost::system::error_code, bool>
    async_handshake(const connect_opts &opts,
                    WriteHandler &&handler = boost::asio::default_completion_token_t<executor_type>{}) {
        const size_t wire_size = opts.wire_size();
        std::unique_ptr<uint8_t[]> buffer_ = std::make_unique<uint8_t[]>(wire_size);
        opts.write_to(buffer_.get());
        return boost::asio::async_compose<WriteHandler, void(boost::system::error_code, bool)>(
            details::handshake_op<NextLayer>{next_, std::move(buffer_), wire_size, {}}, handler, next_);
    }
};
}// namespace v311
}// namespace mqtt