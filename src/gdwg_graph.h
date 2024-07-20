#ifndef GDWG_GRAPH_H
#define GDWG_GRAPH_H

#include <optional>
#include <string>
#include <utility>

// TODO: Make both graph and edge generic
//       ... this won't just compile
//       straight away
namespace gdwg {
	template<typename N, typename E>
	class edge {
	 public:
		virtual ~edge() = default;
		virtual auto print_edge() const -> std::string = 0;
		virtual auto is_weighted() const -> bool = 0;
		virtual auto get_weight() const -> std::optional<E> = 0;
		virtual auto get_nodes() const -> std::pair<N, N> = 0;

	 private:
		// You may need to add data members and member functions
		// friend class graph<N, E>;
	};

	template<typename N, typename E>
	class graph {
	 public:
		using edge = gdwg::edge<N, E>;

		// Your member functions go here
		graph() = default;
		graph(std::initializer_list<N> il);
		template<typename InputIt>
		graph(InputIt first, InputIt last);

		graph(graph&& other) noexcept = default;
		graph(graph const& other);
		auto operator=(graph&& other) noexcept -> graph& = default;
		auto operator=(graph const& other) -> graph&;

	 private:
	};
} // namespace gdwg

#endif // GDWG_GRAPH_H
