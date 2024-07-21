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

		class iterator {
		 public:
			using value_type = struct {
				N from;
				N to;
				std::optional<E> weight;
			};
			using reference = value_type;
			using pointer = void;
			using difference_type = std::ptrdiff_t;
			using iterator_category = std::bidirectional_iterator_tag;

			iterator() = default;
			auto operator*() -> reference;
			auto operator++() -> iterator&;
			auto operator++(int) -> iterator;
			auto operator--() -> iterator&;
			auto operator--(int) -> iterator;
			auto operator==(iterator const& other) const -> bool;

		 private:
		};

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

		[[nodiscard]] auto is_node(N const& value) const noexcept -> bool;
		[[nodiscard]] auto empty() const noexcept -> bool;
		[[nodiscard]] auto is_connected(N const& src, N const& dst) const -> bool;
		[[nodiscard]] auto nodes() const -> std::vector<N>;
		[[nodiscard]] auto edges(N const& src, N const& dst) const -> std::vector<std::shared_ptr<edge>>;
		[[nodiscard]] auto find(N const& src, N const& dst, std::optional<E> weight = std::nullopt) const -> iterator;
		[[nodiscard]] auto connections(N const& src) const -> std::vector<N>;

		[[nodiscard]] auto begin() const -> iterator;
		[[nodiscard]] auto end() const -> iterator;

	 private:
		std::unordered_set<std::shared_ptr<N>> nodes_;
		std::unordered_set<N> node_values_;
		std::unordered_map<std::pair<N, N>, std::vector<std::shared_ptr<edge>>, boost::hash<std::pair<N, N>>> edges_;
	};

	// Implementation of edge class member functions

	template<typename N, typename E>
	auto weighted_edge<N, E>::print_edge() const -> std::string {
		return *src_ + " -> " + *dst_ + " | W | " + std::to_string(weight_);
	}

	template<typename N, typename E>
	auto weighted_edge<N, E>::is_weighted() const -> bool {
		return true;
	}

	template<typename N, typename E>
	auto weighted_edge<N, E>::get_weight() const -> std::optional<E> {
		return weight_;
	}

	template<typename N, typename E>
	auto weighted_edge<N, E>::get_nodes() const -> std::pair<N, N> {
		return {*src_, *dst_};
	}

	template<typename N, typename E>
	auto unweighted_edge<N, E>::print_edge() const -> std::string {
		return *src_ + " -> " + *dst_ + " | U";
	}

	template<typename N, typename E>
	auto unweighted_edge<N, E>::is_weighted() const -> bool {
		return false;
	}

	template<typename N, typename E>
	auto unweighted_edge<N, E>::get_weight() const -> std::optional<int> {
		return std::nullopt;
	}

	template<typename N, typename E>
	auto unweighted_edge<N, E>::get_nodes() const -> std::pair<N, N> {
		return {*src_, *dst_};
	}

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

	template<typename N, typename E>
	auto graph<N, E>::insert_node(N const& value) -> bool {
		if (node_values_.find(value) != node_values_.end()) {
			return false;
		}
		auto node = std::make_shared<N>(value);
		nodes_.insert(node);
		node_values_.insert(value);
		return true;
	}

} // namespace gdwg

#endif // GDWG_GRAPH_H
