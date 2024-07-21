#ifndef GDWG_GRAPH_H
#define GDWG_GRAPH_H

#include <boost/functional/hash.hpp>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

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
	class weighted_edge : public edge<N, E> {
	 public:
		weighted_edge(N src, N dst, E weight)
		: src_(std::make_shared<N>(src))
		, dst_(std::make_shared<N>(dst))
		, weight_(weight) {}

		auto print_edge() const -> std::string override;
		auto is_weighted() const -> bool override;
		auto get_weight() const -> std::optional<E> override;
		auto get_nodes() const -> std::pair<N, N> override;

	 private:
		std::shared_ptr<N> src_;
		std::shared_ptr<N> dst_;
		E weight_;
	};

	template<typename N, typename E>
	class unweighted_edge : public edge<N, E> {
	 public:
		unweighted_edge(N src, N dst)
		: src_(std::make_shared<N>(src))
		, dst_(std::make_shared<N>(dst)) {}

		auto print_edge() const -> std::string override;
		auto is_weighted() const -> bool override;
		auto get_weight() const -> std::optional<int> override;
		auto get_nodes() const -> std::pair<N, N> override;

	 private:
		std::shared_ptr<N> src_;
		std::shared_ptr<N> dst_;
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

		graph(graph&& other) noexcept;
		graph(graph const& other);
		auto operator=(graph&& other) noexcept -> graph&;
		auto operator=(graph const& other) -> graph&;

		auto insert_node(N const& value) -> bool;
		auto insert_edge(N const& src, N const& dst, std::optional<E> weight = std::nullopt) -> bool;
		auto replace_node(N const& old_data, N const& new_data) -> bool;
		auto merge_replace_node(N const& old_data, N const& new_data) -> void;
		auto erase_node(N const& value) -> bool;
		auto erase_edge(N const& src, N const& dst, std::optional<E> weight = std::nullopt) -> bool;
		auto clear() noexcept -> void;

	 private:
		std::unordered_set<std::shared_ptr<N>> nodes_;
		std::unordered_map<std::pair<N, N>, std::vector<std::shared_ptr<edge>>, boost::hash<std::pair<N, N>>> edges_;
	};

	// Implementation of graph member functions
	template<typename N, typename E>
	graph<N, E>::graph(std::initializer_list<N> il)
	: graph(il.begin(), il.end()) {}

	template<typename N, typename E>
	template<typename InputIt>
	graph<N, E>::graph(InputIt first, InputIt last) {
		for (auto it = first; it != last; ++it) {
			insert_node(*it);
		}
	}

	template<typename N, typename E>
	graph<N, E>::graph(graph&& other) noexcept
	: nodes_(std::move(other.nodes_))
	, edges_(std::move(other.edges_)) {}

	template<typename N, typename E>
	graph<N, E>::graph(graph const& other)
	: nodes_(other.nodes_)
	, edges_(other.edges_) {}

	template<typename N, typename E>
	auto graph<N, E>::operator=(graph&& other) noexcept -> graph& {
		if (this != &other) {
			nodes_ = std::move(other.nodes_);
			edges_ = std::move(other.edges_);
		}
		return *this;
	}

	template<typename N, typename E>
	auto graph<N, E>::operator=(graph const& other) -> graph& {
		if (this != &other) {
			nodes_ = other.nodes_;
			edges_ = other.edges_;
		}
		return *this;
	}

} // namespace gdwg

#endif // GDWG_GRAPH_H
