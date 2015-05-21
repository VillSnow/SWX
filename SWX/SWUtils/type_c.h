#pragma once
#include <memory>
#include <stdexcept>

/*
type_c.h
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

	template<class Ty, class Al = std::allocator<Ty>>
	class type_c
	{

	public:
		typedef type_c<Ty, Al> my_type;
		typedef Al allocator_type;
		typedef std::allocator_traits<Al> traits;
		typedef typename traits::template rebind_alloc<Ty>::other alloc;
		typedef typename traits::value_type value_type;
		typedef typename traits::pointer pointer;
		typedef typename traits::const_pointer const_pointer;
		typedef typename traits::size_type size_type;

		using iterator = pointer;
		using const_iterator = const_pointer;

#pragma region constructor
		type_c(const allocator_type &al = allocator_type()) :
			al_(al)
		{}

		type_c(size_type n, const value_type &val, const allocator_type &al = allocator_type()) :
			al_(al)
		{
			reserve(n);
			for (size_type i = 0; i<n; i++, end_++) {
				al_.construct(end_, val);
			}
		}

		type_c(const my_type &right)
		{
			*this = right;
		}

		type_c(my_type &&right)
		{
			*this = std::move(right);
		}

		type_c &operator=(const my_type &left)
		{
			if (alloc::propagate_on_container_copy_assignment::value) {
				al_ = left.al_;
			}
			reserve(left.reserved());
			auto src = left.begin();
			for (end_ = begin_; src!=left.end(); end_++, ++src) {
				al_.construct(end_, *src);
			}

			return *this;
		}

		type_c &operator=(my_type &&right)
		{
			if (alloc::propagate_on_container_move_assignment::value) {
				al_ = right.al_.select_on_container_copy_construction();
			}
			if (al_==right.al_) {
				begin_ = right.begin;
				end_ = right.end_;
				reserved_ = right.reserved_;

				right.begin_ = pointer();
				right.end_ = pointer();
				right.reserved_ = pointer();
			} else {
				*this = static_cast<const my_type &>(right);
			}

			return *this;
		}

		~type_c()
		{
			clear();
			al_.deallocate(begin_, reserved_-begin_);
		}
#pragma endregion

#pragma region iterators
		iterator begin() { return{ begin_ }; }
		iterator end() { return{ end_ }; }
		const_iterator begin() const { return{ begin_ }; }
		const_iterator end() const { return{ end_ }; }
#pragma endregion

#pragma region capacity
		size_type size() const
		{
			return end_-begin_;
		}

		void reserve(size_type n)
		{
			if (n<=reserved())
				return;

			pointer new_ptr = al_.allocate(n);
			pointer dst = new_ptr;
			for (pointer src = begin_; src<reserved_; src++, dst++) {
				al_.construct(dst, std::move(*src));
				al_.destroy(src);
			}
			al_.deallocate(begin_, reserved_-begin_);

			begin_ = new_ptr;
			end_ = dst;
			reserved_ = new_ptr+n;
		}

		size_type reserved() const { return reserved_-begin_; }
#pragma endregion

#pragma region access
		Ty &operator[](size_type pos) { return begin_[pos]; }
		const Ty &operator[](size_type pos) const { return begin_[pos]; }

		Ty &at(size_type pos)
		{
			if (pos<0||size()<=pos)
				throw std::out_of_range("type_c::at");
			return begin_[pos];
		}

		const Ty &at(size_type pos) const
		{
			if (pos<0||size()<=pos)
				throw std::out_of_range("type_c::at");
			return begin_[pos];
		}
#pragma endregion

#pragma region modifiers
		iterator insert(const value_type &cpy)
		{
			chk_reserved();
			al_.construct(end_, cpy);
			return{ end_++ };
		}

		iterator insert(value_type &&right)
		{
			chk_reserved();
			al_.construct(end_, std::move(right));
			return{ end_++ };
		}

		const_iterator erase(const_iterator ite)
		{
			al_.destroy(ite);
			al_.construct(ite, std::move(*--end_));
			al_.destroy(end_);
			return ite;
		}
		//iterator erase(const_iterator first, const_iterator last)

		template<class Pred>
		void erase_if(Pred pred)
		{
			const_pointer ite = begin_;
			while (ite!=end()) {
				if (pred(*ite)) {
					ite = erase(ite);
				} else {
					++ite;
				}
			}
		}

		void clear()
		{
			for (pointer ptr = begin_; ptr<end_; ptr++) {
				al_.destroy(ptr);
			}
			end_ = begin_;
		}

		template<class ...Types>
		iterator emplace(Types &&... params)
		{
			chk_reserved();
			al_.construct(end_, std::forward<Types>(params)...);
			return  end_++;
		}




#pragma endregion
	private:

		void chk_reserved()
		{
			if (end_==reserved_) {
				auto n = reserved();
				if (n==0)
					reserve(128);
				else
					reserve(n*2);
			}
		}

		alloc al_ = alloc();
		pointer begin_ = pointer();
		pointer end_ = pointer();
		pointer reserved_ = pointer();
	};

}
