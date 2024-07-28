#include "gdwg_graph.h"

#include <catch2/catch.hpp>

TEST_CASE("basic test") {
	auto g = gdwg::graph<int, std::string>{};
	auto n = 5;
	g.insert_node(n);
	REQUIRE(g.is_node(n) == true);
}

TEST_CASE("Insert nodes") {
	auto g = gdwg::graph<std::string, int>{};
	REQUIRE(g.insert_node("A") == true);
	REQUIRE(g.insert_node("B") == true);
	REQUIRE(g.insert_node("A") == false);
}

TEST_CASE("Insert edges") {
	auto g = gdwg::graph<std::string, int>{};
	g.insert_node("A");
	g.insert_node("B");

	REQUIRE(g.insert_edge("A", "B", 5) == true);
	REQUIRE(g.insert_edge("A", "B", 10) == true);
	REQUIRE(g.insert_edge("A", "B", 5) == false);
}

TEST_CASE("Insert edge with non-existent nodes throws runtime_error") {
	auto g = gdwg::graph<std::string, int>{};
	g.insert_node("A");
	g.insert_node("B");

	REQUIRE_THROWS_WITH(g.insert_edge("X", "B", 5),
	                    "Cannot call gdwg::graph<N, E>::insert_edge when either src or dst node does not exist");

	REQUIRE_THROWS_WITH(g.insert_edge("A", "Y", 10),
	                    "Cannot call gdwg::graph<N, E>::insert_edge when either src or dst node does not exist");

	REQUIRE_THROWS_WITH(g.insert_edge("X", "Y", 15),
	                    "Cannot call gdwg::graph<N, E>::insert_edge when either src or dst node does not exist");

	REQUIRE_NOTHROW(g.insert_edge("A", "B", 5));
}

TEST_CASE("replace non-existing node") {
	auto g = gdwg::graph<int, std::string>{};
	g.insert_node(1);
	g.insert_node(2);
	g.insert_node(3);

	REQUIRE_THROWS_WITH(g.replace_node(4, 5), "Cannot call gdwg::graph<N, E>::replace_node on a node that doesn't exist");
}

TEST_CASE("replace existing node and update edges") {
	auto g = gdwg::graph<int, std::string>{};
	g.insert_node(1);
	g.insert_node(2);
	g.insert_node(3);
	g.insert_edge(1, 2, "edge1");
	g.insert_edge(2, 3, "edge2");
	g.insert_edge(1, 3, "edge3");

	REQUIRE(g.replace_node(2, 4) == true);
	REQUIRE(g.is_node(2) == false);
	REQUIRE(g.is_node(4) == true);
	REQUIRE(g.is_connected(1, 4) == true);
	REQUIRE(g.is_connected(4, 3) == true);
}

TEST_CASE("replace node with existing node") {
	auto g = gdwg::graph<int, std::string>{};
	g.insert_node(1);
	g.insert_node(2);
	g.insert_node(3);

	REQUIRE(g.replace_node(1, 2) == false);
}

TEST_CASE("merge_replace_node: old_data or new_data does not exist") {
	auto g = gdwg::graph<std::string, int>{};
	g.insert_node("A");
	g.insert_node("B");

	g.insert_edge("A", "B", 1);

	REQUIRE_THROWS_WITH(g.merge_replace_node("C", "A"),
	                    "Cannot call gdwg::graph<N, E>::merge_replace_node on old or new data if they don't exist in "
	                    "the graph");

	REQUIRE_THROWS_WITH(g.merge_replace_node("A", "C"),
	                    "Cannot call gdwg::graph<N, E>::merge_replace_node on old or new data if they don't exist in "
	                    "the graph");
}

TEST_CASE("Erase a weighted edge that exists") {
	auto g = gdwg::graph<std::string, int>{};
	g.insert_node("A");
	g.insert_node("B");
	g.insert_edge("A", "B", 1);

	REQUIRE(g.erase_edge("A", "B", 1) == true);
	REQUIRE(g.is_connected("A", "B") == false);
}

TEST_CASE("Erase an unweighted edge that exists") {
	auto g = gdwg::graph<std::string, int>{};
	g.insert_node("A");
	g.insert_node("B");
	g.insert_edge("A", "B");

	REQUIRE(g.erase_edge("A", "B") == true);
	REQUIRE(g.is_connected("A", "B") == false);
}

TEST_CASE("Erase an unweighted edge that does not exist") {
	auto g = gdwg::graph<std::string, int>{};
	g.insert_node("A");
	g.insert_node("B");

	REQUIRE(g.erase_edge("A", "B") == false);
}

TEST_CASE("Erase an edge when either src or dst node does not exist", "[graph][erase_edge]") {
	auto g = gdwg::graph<std::string, int>{};
	g.insert_node("A");

	REQUIRE_THROWS_WITH(g.erase_edge("A", "B"),
	                    "Cannot call gdwg::graph<N, E>::erase_edge on src or dst if they don't exist in the graph");
	REQUIRE_THROWS_WITH(g.erase_edge("B", "A"),
	                    "Cannot call gdwg::graph<N, E>::erase_edge on src or dst if they don't exist in the graph");
}
