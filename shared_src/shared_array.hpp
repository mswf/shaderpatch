#pragma once

#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <memory>
#include <stdexcept>
#include <type_traits>

#include <gsl/gsl>

namespace sp {

template<typename Type>
class Shared_array {
   static_assert(std::is_default_constructible_v<Type>,
                 "Type must be default constructable.");

   using value_type = Type;
   using size_type = gsl::index;
   using difference_type = gsl::index;
   using reference = value_type&;
   using const_reference = const value_type&;
   using pointer = value_type*;
   using const_pointer = const value_type*;
   using iterator = value_type*;
   using const_iterator = const value_type*;
   using reverse_iterator = std::reverse_iterator<iterator>;
   using const_reverse_iterator = std::reverse_iterator<const_iterator>;

   Shared_array() = default;

   Shared_array(const Shared_array&) = default;
   Shared_array& operator=(const Shared_array&) = default;

   Shared_array(Shared_array&&) = default;
   Shared_array& operator=(Shared_array&&) = default;

   template<typename Allocator = std::allocator<Type>>
   explicit Shared_array(size_type size, const_reference value = value_type{},
                         Allocator allocator = Allocator{})
      : _size{size}
   {
      Expects(size > 0);

      static_assert(std::is_copy_constructible_v<Allocator>,
                    "Allocator must be copy constructable.");

      gsl::owner<Type*> data = allocator.allocate(_size);

      if constexpr (std::is_nothrow_copy_constructible_v<Type>) {
         std::uninitialized_fill_n(data, _size, value);
      }
      else {
         try {
            std::uninitialized_fill_n(data, _size, value);
         }
         catch (...) {
            allocator.deallocate(data, _size);

            throw;
         }
      }

      _memory =
         std::shared_ptr<Type[]>{data, [_size, allocator{std::move(allocator)}](Type* data) {
                                    std::destroy_n(data, _size);
                                    allocator.deallocate(data, _size);
                                 }};
   }

   template<typename Allocator = std::allocator<Type>>
   Shared_array(size_type size, Allocator&& allocator)
      : Shared_array{size, value_type{}, std::move(allocator)}
   {
      static_assert(std::is_default_constructible_v<Type>,
                    "Type must be default constructable.");

      static_assert(std::is_copy_constructible_v<Allocator>,
                    "Allocator must be copy constructable.");
   }

   template<typename Allocator = std::allocator<Type>>
   Shared_array(size_type size, Allocator allocator)
      : Shared_array{size, value_type{}, std::move(allocator)}
   {
      static_assert(std::is_default_constructible_v<Type>,
                    "Type must be default constructable.");

      static_assert(std::is_copy_constructible_v<Allocator>,
                    "Allocator must be copy constructable.");
   }

   template<typename Input_iterator, typename Allocator = std::allocator<Type>>
   Shared_array(Input_iterator first, Input_iterator last,
                Allocator allocator = Allocator{})
   {
      const auto size = std::distance(first, last);

      *this = Shared_array{size, value_type{}, std::move(allocator)};

      std::copy(first, last, begin());
   }

   template<typename Allocator = std::allocator<Type>>
   Shared_array(std::initializer_list<Type> init_list,
                Allocator allocator = Allocator{})
   {
      const auto size = std::distance(first, last);

      *this = Shared_array{size, value_type{}, std::move(allocator)};

      std::copy(std::cbegin(init_list), std::cend(init_list), begin());
   }

   reference at(size_type pos)
   {
      if (pos > _size || pos < 0) {
         throw std::out_of_range{"out of bounds array access"};
      }

      return _memory[pos];
   }

   const_reference at(size_type pos) const
   {
      if (pos > _size || pos < 0) {
         throw std::out_of_range{"out of bounds array access"};
      }

      return _memory[pos];
   }

   reference operator[](size_type pos) noexcept
   {
      assert(pos < _size || pos > 0);

      return _memory[pos];
   }

   const_reference operator[](size_type pos) const noexcept
   {
      assert(pos < _size || pos > 0);

      return _memory[pos];
   }

   iterator begin() noexcept
   {
      return _memory.get();
   }

   iterator end() noexcept
   {
      return _memory.get() + _size;
   }

   const_iterator begin() const noexcept
   {
      return _memory.get();
   }

   const_iterator end() const noexcept
   {
      return _memory.get() + _size;
   }

   const_iterator cbegin() const noexcept
   {
      return _memory.get();
   }

   const_iterator cend() const noexcept
   {
      return _memory.get() + _size;
   }

   reverse_iterator rbegin() noexcept
   {
      return std::make_reverse_iterator(end());
   }

   reverse_iterator rend() noexcept
   {
      return std::make_reverse_iterator(begin());
   }

   const_reverse_iterator rbegin() const noexcept
   {
      return std::make_reverse_iterator(end());
   }

   const_reverse_iterator rend() const noexcept
   {
      return std::make_reverse_iterator(begin());
   }

   const_reverse_iterator crbegin() const noexcept
   {
      return std::make_reverse_iterator(cend());
   }

   const_reverse_iterator crend() const noexcept
   {
      return std::make_reverse_iterator(cbegin());
   }

   [[nodiscard]] bool empty() const noexcept
   {
      return (_size == 0);
   }

   size_type size() const noexcept
   {
      return _size;
   }

   constexpr size_type max_size() const noexcept
   {
      return std::numeric_limits<size_type>::max();
   }

   void fill(const_reference value) noexcept(std::is_nothrow_copy_assignable_v<value_type>)
   {
      for (auto& other : *this) other = value;
   }

   void swap(Shared_array& other) noexcept
   {
      using std::swap;

      swap(this->_memory, other._memory);
      swap(this->_size, other._size);
   }

private:
   std::shared_ptr<Type[]> _memory;
   size_type _size{0};
};

template<typename Type>
inline void swap(Shared_array<Type>& left, Shared_array<Type>& right)
{
   left.swap(right);
}

template<typename Type>
inline bool operator==(const Shared_array<Type>& left, const Shared_array<Type>& right)
{
   if (left.size() != right.size()) return false;

   return std::equal(std::cbegin(left), std::cend(left), std::cbegin(right),
                     std::cend(right));
}

template<typename Type>
inline bool operator!=(const Shared_array<Type>& left, const Shared_array<Type>& right)
{
   return !(left == right);
}

template<typename Type>
inline bool operator<(const Shared_array<Type>& left, const Shared_array<Type>& right)
{
   return std::lexicographical_compare(std::cbegin(left), std::cend(left),
                                       std::cbegin(right), std::cend(right));
}

template<typename Type>
inline bool operator>(const Shared_array<Type>& left, const Shared_array<Type>& right)
{
   return right < left;
}

template<typename Type>
inline bool operator<=(const Shared_array<Type>& left, const Shared_array<Type>& right)
{
   return !(left > right);
}

template<typename Type>
inline bool operator>=(const Shared_array<Type>& left, const Shared_array<Type>& right)
{
   return !(left < right);
}

}
