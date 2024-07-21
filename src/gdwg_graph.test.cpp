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