#pragma once
#include <memory>
#include <stdexcept>
#include <vector>
#include <array>

/*
multi_dim.h
Under 'zlib License'
Copyright (c) 2014 Vill. Snow

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1.	The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.

2.	Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.

3.	This notice may not be removed or altered from any source distribution.
*/

// last-modified 2015-03-24

namespace SWUtils {

	template<class Ty, size_t D, class Al = std::allocator<Ty>>
	class multi_dim
	{
	public:

		typedef multi_dim<Ty, D, Al> my_type;

		typedef Al allocator_type;
		typedef std::allocator_traits<Al> traits;
		typedef typename traits::value_type value_type;
		typedef typename traits::pointer pointer;
		typedef typename traits::const_pointer const_pointer;
		typedef typename traits::size_type size_type;

		typedef std::vector<value_type, allocator_type> buf_type;

		using iterator = typename buf_type::iterator;
		using const_iterator = typename buf_type::const_iterator;

		static const size_t dimension = D;

#pragma region constructor
		multi_dim(allocator_type al = allocator_type()) :
			buf_(al)
		{}

		multi_dim(const multi_dim &other) :
			buf_(left.buf_), count_(left.count_)
		{}

		multi_dim(const multi_dim &other, allocator_type al) :
			buf_(left.buf_, al), count_(left.count_)
		{}

		multi_dim(multi_dim &&other) :
			buf_(std::move(left.buf_)), count_(std::(left.count_))
		{}

		multi_dim(multi_dim &&other, allocator_type al) :
			buf_(std::move(left.buf_), al), count_(std::(left.count_))
		{}
#pragma endregion

		allocator_type get_allocator() const
		{
			return buf_.get_allocator();
		}

		template<class... Size>
		void resize(Size... count)
		{
			static_assert(sizeof...(count)==dimension, "dimension should be matched");
			auto tmp_buf = std::move(buf_);
			auto tmp_count_ = std::move(count_);

			setcount_(0, static_cast<size_type>(count)...);
			buf_.resize(std::accumulate(count_.begin(), count_.end(),
				1, [](size_type lhs, size_type rhs) { return lhs*rhs; }));
		}

		template<class... Indicis>
		value_type &at(Indicis... indicis)
		{
			static_assert(sizeof...(indicis)==dimension, "dimension should be matched");
			return buf_.at(offset_(0, 0, static_cast<size_type>(indicis)...));
		}

		template<class... Indicis>
		const value_type &at(Indicis... indicis) const
		{
			static_assert(sizeof...(indicis)==dimension, "dimension should be matched");
			return buf_.at(offset_(0, 0, static_cast<size_type>(indicis)...));
		}


		iterator begin() { return buf_.begin(); }
		const_iterator begin() const { return buf_.begin(); }
		iterator end() { return buf_.end(); }
		const_iterator end() const { return buf_.end(); }

		template<class... Indicis>
		iterator bound(Indicis... indicis)
		{
			//{
			// {{000,001,002,003,004},{010,011,012,013,014},{020 ... }},
			// {{100,101,102,103,004},{110,111,112,113,114},{120 ... }},
			//   ^ here is bound(1)  and  bound(1,2) is here ^
			// ...
			//}

			return bound_(begin(), 0, static_cast<size_type>(indicis)...);
		}
		template<class... Indicis>
		const_iterator bound(Indicis... indicis) const
		{
			return bound_(begin(), 0, static_cast<size_type>(indicis)...);
		}

		pointer data()
		{
			return buf_.data();
		}

		const_pointer data() const
		{
			return buf_.data();
		}


	private:
		template<class Head, class... Rest>
		size_type offset_(size_type ret, size_t dim, Head head, Rest... rest) const
		{
			// count : 2, 3, 4 =>
			// 0 2 * i + 3 * j + 4 * k +
			if (count_.at(dim)<=head)
				throw std::out_of_range("multi_dim out_of_range");
			ret *= count_.at(dim);
			ret += head;
			return offset_(ret, ++dim, rest...);
		}

		size_type offset_(size_type ret, size_t dim) const
		{
			return ret;
		}

		template<class Head, class... Rest>
		void setcount_(size_t dim, Head head, Rest... rest)
		{
			count_.at(dim) = head;
			setcount_(++dim, rest...);
		}
		void setcount_(size_t dim) {}

		template<class Head, class... Rest>
		iterator bound_(iterator ite, size_t dim, Head head, Rest... rest)
		{
			if (dim<dimension) {
				if (count_.at(dim)<=head)
					throw std::out_of_range("multi_dim out_of_range");
				size_type s = 1;
				for (size_t i = dim+1; i<dimension; i++)
					s *= count_.at(i);
				ite += s*head;
				return bound_(ite, ++dim, rest...);
			} else {
				return ite;
			}
		}
		template<class Head, class... Rest>
		const_iterator bound_(iterator ite, size_t dim, Head head, Rest... rest) const
		{
			if (dim<dimension) {
				if (count_.at(dim)<=head)
					throw std::out_of_range("multi_dim out_of_range");
				size_type s = 1;
				for (size_t i = dim+1; i<dimension; i++)
					s *= count_.at(i);
				ite += s*head;
				return bound_(ite, ++dim, rest...);
			} else {
				return ite;
			}
		}

		iterator bound_(iterator ite, size_type order)
		{
			return ite;
		}

		buf_type buf_;
		std::array<size_type, dimension> count_;
	};

}
