#ifndef SIGNAL_HPP
#define SIGNAL_HPP

#include <functional>
#include <list>
#include <type_traits>
#include <utility>

// Minimalist sigc::signal replacement, without slots (disconnection) or return values.

namespace sau
{
	template <class T> class signal;

	template <class R, class... Args> class signal<R(Args...)>
	{
		static_assert(std::is_same<R, void>::value, "Return type must be void");

		private:
			std::list<std::function<R(Args...)>> m_slots;

		public:
			void connect(std::function<R(Args...)>&& fn)
			{
				m_slots.push_back(std::move(fn));
			}

			void connect(const std::function<R(Args...)>& fn)
			{
				m_slots.push_back(fn);
			}

			template <class... CallArgs>
			void operator()(CallArgs&&... args) const
			{
				for (auto&& slot : m_slots)
					slot(std::forward<CallArgs>(args)...);
			}
	};
}

#endif // SIGNAL_HPP
