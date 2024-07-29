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
		auto get_weight() const -> std::optional<E> override;
		auto get_nodes() const -> std::pair<N, N> override;

	 private:
		std::shared_ptr<N> src_;
		std::shared_ptr<N> dst_;
	};

	template<typename N, typename E>
	class graph {
	 public:
		using edge = gdwg::edge<N, E>;

		struct edge_cmp {
			bool operator()(const std::unique_ptr<edge>& lhs, const std::unique_ptr<edge>& rhs) const {
				auto lhs_nodes = lhs->get_nodes();
				auto rhs_nodes = rhs->get_nodes();

				if (lhs_nodes.first != rhs_nodes.first) {
					return lhs_nodes.first < rhs_nodes.first;
				}

				if (lhs_nodes.second != rhs_nodes.second) {
					return lhs_nodes.second < rhs_nodes.second;
				}

				if (lhs->is_weighted() != rhs->is_weighted()) {
					return !lhs->is_weighted();
				}

				if (lhs->is_weighted() && rhs->is_weighted()) {
					return lhs->get_weight() < rhs->get_weight();
				}

				return lhs < rhs;
			}
		};

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
			explicit iterator(typename std::set<std::unique_ptr<edge>, edge_cmp>::iterator it)
			: it_(it) {}

			// Iterator source
			auto operator*() -> reference {
				auto nodes = (*it_)->get_nodes();
				return {nodes.first, nodes.second, (*it_)->get_weight()};
			}

			// Iterator traversal
			auto operator++() -> iterator& {
				++it_;
				return *this;
			}
			auto operator++(int) -> iterator {
				auto temp = *this;
				++it_;
				return temp;
			}
			auto operator--() -> iterator& {
				--it_;
				return *this;
			}
			auto operator--(int) -> iterator {
				auto temp = *this;
				--it_;
				return temp;
			}

			// Iterator comparison
			auto operator==(iterator const& other) const -> bool {
				return it_ == other.it_;
			}

		 private:
			typename std::set<std::unique_ptr<edge>, edge_cmp>::iterator it_;
			friend class graph<N, E>;
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
		auto erase_edge(iterator i) -> iterator;
		auto erase_edge(iterator i, iterator s) -> iterator;
		auto clear() noexcept -> void;

		[[nodiscard]] auto is_node(N const& value) const noexcept -> bool;
		[[nodiscard]] auto empty() const noexcept -> bool;
		[[nodiscard]] auto is_connected(N const& src, N const& dst) const -> bool;
		[[nodiscard]] auto nodes() const -> std::vector<N>;
		[[nodiscard]] auto edges(N const& src, N const& dst) const -> std::vector<std::unique_ptr<edge>>;
		[[nodiscard]] auto find(N const& src, N const& dst, std::optional<E> weight = std::nullopt) const -> iterator;
		[[nodiscard]] auto connections(N const& src) const -> std::vector<N>;

		[[nodiscard]] auto begin() const -> iterator;
		[[nodiscard]] auto end() const -> iterator;

	 private:
		struct node_cmp {
			using is_transparent = void;
			bool operator()(const std::shared_ptr<N>& lhs, const std::shared_ptr<N>& rhs) const {
				return *lhs < *rhs;
			}

			bool operator()(const std::shared_ptr<N>& lhs, const N& rhs) const {
				return *lhs < rhs;
			}

			bool operator()(const N& lhs, const std::shared_ptr<N>& rhs) const {
				return lhs < *rhs;
			}
		};

		std::set<std::shared_ptr<N>, node_cmp> nodes_;
		std::set<std::unique_ptr<edge>, edge_cmp> edges_;
	};

	// Implementation of edge class member functions

	template<typename N, typename E>
	auto weighted_edge<N, E>::print_edge() const -> std::string {
		std::ostringstream oss;
		oss << *src_ << " -> " << *dst_ << " | W | " << weight_;
		return oss.str();
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
		std::ostringstream oss;
		oss << *src_ << " -> " << *dst_ << " | U";
		return oss.str();
	}

	template<typename N, typename E>
	auto unweighted_edge<N, E>::is_weighted() const -> bool {
		return false;
	}

	template<typename N, typename E>
	auto unweighted_edge<N, E>::get_weight() const -> std::optional<E> {
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
	graph<N, E>::graph(graph const& other) {
		nodes_ = other.nodes_;

		for (const auto& edge_ptr : other.edges_) {
			if (edge_ptr->is_weighted()) {
				auto [src, dst] = edge_ptr->get_nodes();
				auto weight = edge_ptr->get_weight();
				edges_.insert(std::make_unique<weighted_edge<N, E>>(src, dst, *weight));
			}
			else {
				auto [src, dst] = edge_ptr->get_nodes();
				edges_.insert(std::make_unique<unweighted_edge<N, E>>(src, dst));
			}
		}
	}

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
		if (this == &other) {
			return *this;
		}
		nodes_ = other.nodes_;
		edges_.clear();
		for (const auto& edge_ptr : other.edges_) {
			if (edge_ptr->is_weighted()) {
				auto [src, dst] = edge_ptr->get_nodes();
				auto weight = edge_ptr->get_weight();
				edges_.insert(std::make_unique<weighted_edge<N, E>>(src, dst, *weight));
			}
			else {
				auto [src, dst] = edge_ptr->get_nodes();
				edges_.insert(std::make_unique<unweighted_edge<N, E>>(src, dst));
			}
		}
		return *this;
	}

	template<typename N, typename E>
	[[nodiscard]] auto graph<N, E>::is_node(N const& value) const noexcept -> bool {
		return nodes_.find(value) != nodes_.end();
	}

	template<typename N, typename E>
	auto graph<N, E>::insert_node(N const& value) -> bool {
		// Check if the node already exists
		if (is_node(value)) {
			return false;
		}
		auto node = std::make_shared<N>(value);
		auto result = nodes_.insert(node);
		return result.second; // 如果插入成功，result.second 为 true，否则为 false
	}

	template<typename N, typename E>
	auto graph<N, E>::insert_edge(N const& src, N const& dst, std::optional<E> weight) -> bool {
		if (not is_node(src) or not is_node(dst)) {
			throw std::runtime_error("Cannot call gdwg::graph<N, E>::insert_edge when either src or dst node does not "
			                         "exist");
		}

		std::unique_ptr<edge> new_edge;
		if (weight) {
			new_edge = std::make_unique<weighted_edge<N, E>>(src, dst, *weight);
		}
		else {
			new_edge = std::make_unique<unweighted_edge<N, E>>(src, dst);
		}

		if (edges_.find(new_edge) != edges_.end()) {
			return false;
		}

		edges_.insert(std::move(new_edge));
		return true;
	}

	template<typename N, typename E>
	auto graph<N, E>::replace_node(N const& old_data, N const& new_data) -> bool {
		if (not is_node(old_data)) {
			throw std::runtime_error("Cannot call gdwg::graph<N, E>::replace_node on a node that doesn't exist");
		}

		if (is_node(new_data)) {
			return false;
		}

		auto old_node_it = nodes_.find(old_data);
		auto new_node = std::make_shared<N>(new_data);
		nodes_.erase(old_node_it);
		nodes_.insert(new_node);

		std::vector<std::unique_ptr<edge>> new_edges;
		for (auto it = edges_.begin(); it != edges_.end();) {
			auto& e = *it;
			auto nodes = e->get_nodes();
			bool modified = false;
			if (nodes.first == old_data) {
				nodes.first = new_data;
				modified = true;
			}
			if (nodes.second == old_data) {
				nodes.second = new_data;
				modified = true;
			}
			if (modified) {
				if (e->is_weighted()) {
					new_edges.push_back(
					    std::make_unique<weighted_edge<N, E>>(nodes.first, nodes.second, *e->get_weight()));
				}
				else {
					new_edges.push_back(std::make_unique<unweighted_edge<N, E>>(nodes.first, nodes.second));
				}
				it = edges_.erase(it);
			}
			else {
				++it;
			}
		}

		for (auto& new_edge : new_edges) {
			edges_.insert(std::move(new_edge));
		}

		return true;
	}

	template<typename N, typename E>
	[[nodiscard]] auto graph<N, E>::is_connected(N const& src, N const& dst) const -> bool {
		if (not is_node(src) or not is_node(dst)) {
			throw std::runtime_error("Cannot call gdwg::graph<N, E>::is_connected if src or dst node don't exist in "
			                         "the graph");
		}

		for (const auto& edge : edges_) {
			auto nodes = edge->get_nodes();
			if (nodes.first == src && nodes.second == dst) {
				return true;
			}
		}
		return false;
	}

	template<typename N, typename E>
	[[nodiscard]] auto graph<N, E>::empty() const noexcept -> bool {
		return nodes_.empty();
	}

	template<typename N, typename E>
	[[nodiscard]] auto graph<N, E>::nodes() const -> std::vector<N> {
		std::vector<N> result;
		result.reserve(nodes_.size());
		for (const auto& node_ptr : nodes_) {
			result.push_back(*node_ptr);
		}
		std::sort(result.begin(), result.end());
		return result;
	}

	template<typename N, typename E>
	auto graph<N, E>::merge_replace_node(N const& old_data, N const& new_data) -> void {
		if (not is_node(old_data) or not is_node(new_data)) {
			throw std::runtime_error("Cannot call gdwg::graph<N, E>::merge_replace_node on old or new data if they "
			                         "don't exist in the graph");
		}

		std::vector<std::unique_ptr<edge>> new_edges;
		for (auto it = edges_.begin(); it != edges_.end();) {
			auto& e = *it;
			auto nodes = e->get_nodes();
			bool modified = false;

			if (nodes.first == old_data) {
				nodes.first = new_data;
				modified = true;
			}
			if (nodes.second == old_data) {
				nodes.second = new_data;
				modified = true;
			}

			if (modified) {
				std::unique_ptr<edge> new_edge;
				if (e->is_weighted()) {
					new_edge = std::make_unique<weighted_edge<N, E>>(nodes.first, nodes.second, *e->get_weight());
				}
				else {
					new_edge = std::make_unique<unweighted_edge<N, E>>(nodes.first, nodes.second);
				}

				if (edges_.find(new_edge) == edges_.end()) {
					new_edges.push_back(std::move(new_edge));
				}

				it = edges_.erase(it);
			}
			else {
				++it;
			}
		}
		for (auto& new_edge : new_edges) {
			edges_.insert(std::move(new_edge));
		}
		auto old_node_it = nodes_.find(old_data);
		nodes_.erase(old_node_it);
	}

	template<typename N, typename E>
	auto graph<N, E>::erase_node(N const& value) -> bool {
		auto node_it = nodes_.find(value);
		if (node_it == nodes_.end()) {
			return false;
		}

		for (auto it = edges_.begin(); it != edges_.end();) {
			auto nodes = (*it)->get_nodes();
			if (nodes.first == value || nodes.second == value) {
				it = edges_.erase(it);
			}
			else {
				++it;
			}
		}

		nodes_.erase(node_it);

		return true;
	}

	template<typename N, typename E>
	auto graph<N, E>::erase_edge(N const& src, N const& dst, std::optional<E> weight) -> bool {
		if (not is_node(src) or not is_node(dst)) {
			throw std::runtime_error("Cannot call gdwg::graph<N, E>::erase_edge on src or dst if they don't exist in "
			                         "the graph");
		}

		for (auto it = edges_.begin(); it != edges_.end(); ++it) {
			auto& e = *it;
			auto nodes = e->get_nodes();
			if (nodes.first == src && nodes.second == dst) {
				if (!weight.has_value() && !e->is_weighted()) {
					edges_.erase(it);
					return true;
				}
				if (weight.has_value() && e->is_weighted() && e->get_weight() == weight) {
					edges_.erase(it);
					return true;
				}
			}
		}
		return false;
	}

	template<typename N, typename E>
	auto graph<N, E>::erase_edge(iterator i) -> iterator {
		auto it = i.it_;
		auto next_it = edges_.erase(it);

		return iterator(next_it);
	}

	template<typename N, typename E>
	auto graph<N, E>::erase_edge(iterator i, iterator s) -> iterator {
		auto it = i.it_;
		auto end_it = s.it_;
		auto next_it = edges_.erase(it, end_it);

		return iterator(next_it);
	}

	template<typename N, typename E>
	auto graph<N, E>::clear() noexcept -> void {
		nodes_.clear();
		edges_.clear();
	}

	template<typename N, typename E>
	[[nodiscard]] auto graph<N, E>::find(N const& src, N const& dst, std::optional<E> weight) const -> iterator {
		for (auto it = edges_.begin(); it != edges_.end(); ++it) {
			auto& e = *it;
			auto nodes = e->get_nodes();
			if (nodes.first == src && nodes.second == dst) {
				if (!weight.has_value() && !e->is_weighted()) {
					return iterator(it);
				}
				if (weight.has_value() && e->is_weighted() && e->get_weight() == weight) {
					return iterator(it);
				}
			}
		}

		return end();
	}

	template<typename N, typename E>
	[[nodiscard]] auto graph<N, E>::begin() const -> iterator {
		return iterator(edges_.begin());
	}

	template<typename N, typename E>
	[[nodiscard]] auto graph<N, E>::end() const -> iterator {
		return iterator(edges_.end());
	}

	template<typename N, typename E>
	[[nodiscard]] auto graph<N, E>::connections(N const& src) const -> std::vector<N> {
		if (not is_node(src)) {
			throw std::runtime_error("Cannot call gdwg::graph<N, E>::connections if src doesn't exist in the graph");
		}

		std::vector<N> connected_nodes;
		for (const auto& e : edges_) {
			auto nodes = e->get_nodes();
			if (nodes.first == src) {
				connected_nodes.push_back(nodes.second);
			}
		}

		std::sort(connected_nodes.begin(), connected_nodes.end());
		return connected_nodes;
	}

	template<typename N, typename E>
	[[nodiscard]] auto graph<N, E>::edges(N const& src, N const& dst) const -> std::vector<std::unique_ptr<edge>> {
		if (!is_node(src) || !is_node(dst)) {
			throw std::runtime_error("Cannot call gdwg::graph<N, E>::edges if src or dst node don't exist in the "
			                         "graph");
		}

		std::vector<std::unique_ptr<edge>> result;

		for (const auto& e : edges_) {
			auto nodes = e->get_nodes();
			if (nodes.first == src && nodes.second == dst) {
				if (e->is_weighted()) {
					result.push_back(std::make_unique<weighted_edge<N, E>>(nodes.first, nodes.second, *e->get_weight()));
				}
				else {
					result.push_back(std::make_unique<unweighted_edge<N, E>>(nodes.first, nodes.second));
				}
			}
		}

		return result;
	}

} // namespace gdwg

#endif // GDWG_GRAPH_H
