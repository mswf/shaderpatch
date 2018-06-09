#pragma once

#include <algorithm>
#include <atomic>
#include <exception>
#include <execution>
#include <functional>
#include <iterator>
#include <type_traits>

#include <gsl/gsl>

namespace sp {

template<class ExecutionPolicy, class ForwardIterable, class Function>
inline void for_each(ExecutionPolicy&& policy, ForwardIterable& iterable, Function&& func)
{
   std::for_each(std::forward<ExecutionPolicy>(policy), std::begin(iterable),
                 std::end(iterable), std::forward<Function>(func));
}

template<class ExecutionPolicy, class ForwardIterable, class Function>
inline void for_each_exception_capable(ExecutionPolicy&& policy,
                                       ForwardIterable&& iterable, Function&& func)
{
   std::atomic_bool exception_occured{false};
   std::exception_ptr exception_ptr;

   for_each(std::forward<ExecutionPolicy>(policy),
            std::forward<ForwardIterable>(iterable),
            [&func, &exception_occured, &exception_ptr](auto&& v) {
               try {
                  if (exception_occured.load()) return;

                  std::invoke(std::forward<Function>(func),
                              std::forward<decltype(v)>(v));
               }
               catch (...) {
                  if (exception_occured.exchange(true)) return;

                  exception_ptr = std::current_exception();
               }
            });

   if (exception_ptr) std::rethrow_exception(exception_ptr);
}

template<typename Contiguous_range_into, typename Contiguous_range_from>
inline std::size_t range_memcpy(Contiguous_range_into& into,
                                const Contiguous_range_from& from)
{
   const auto into_size =
      sizeof(typename Contiguous_range_into::value_type) * into.size();
   const auto from_size =
      sizeof(typename Contiguous_range_from::value_type) * from.size();

   Expects(into_size >= from_size);

   std::memcpy(into.data(), from.data(), from_size);

   return from_size;
}

template<typename Span_type, typename Contiguous_range_from>
inline std::size_t range_memcpy(gsl::span<Span_type> span,
                                const Contiguous_range_from& from)
{
   static_assert(!std::is_const_v<Span_type>, "Span type can not be const.");

   return range_memcpy<decltype(span)>(span, from);
}

}
