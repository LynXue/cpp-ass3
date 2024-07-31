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
	auto g = gdwg::graph<std::string, int>{"A", "B"};

	REQUIRE(g.insert_edge("A", "B", 5) == true);
	REQUIRE(g.insert_edge("A", "B", 10) == true);
	REQUIRE(g.insert_edge("A", "B", 5) == false);
}

TEST_CASE("Insert edge with non-existent nodes throws runtime_error") {
	auto g = gdwg::graph<std::string, int>{"A", "B"};

	REQUIRE_THROWS_WITH(g.insert_edge("X", "B", 5),
	                    "Cannot call gdwg::graph<N, E>::insert_edge when either src or dst node does not exist");

	REQUIRE_THROWS_WITH(g.insert_edge("A", "Y", 10),
	                    "Cannot call gdwg::graph<N, E>::insert_edge when either src or dst node does not exist");

	REQUIRE_THROWS_WITH(g.insert_edge("X", "Y", 15),
	                    "Cannot call gdwg::graph<N, E>::insert_edge when either src or dst node does not exist");

	REQUIRE_NOTHROW(g.insert_edge("A", "B", 5));
}

TEST_CASE("replace non-existing node") {
	auto g = gdwg::graph<int, std::string>{1, 2, 3};

	REQUIRE_THROWS_WITH(g.replace_node(4, 5), "Cannot call gdwg::graph<N, E>::replace_node on a node that doesn't exist");
}

TEST_CASE("replace existing node and update edges") {
	auto g = gdwg::graph<int, std::string>{1, 2, 3};
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
	auto g = gdwg::graph<int, std::string>{1, 2, 3};

	REQUIRE(g.replace_node(1, 2) == false);
}

TEST_CASE("merge_replace_node: old_data or new_data does not exist") {
	auto g = gdwg::graph<std::string, int>{"A", "B"};

	g.insert_edge("A", "B", 1);

	REQUIRE_THROWS_WITH(g.merge_replace_node("C", "A"),
	                    "Cannot call gdwg::graph<N, E>::merge_replace_node on old or new data if they don't exist in "
	                    "the graph");

	REQUIRE_THROWS_WITH(g.merge_replace_node("A", "C"),
	                    "Cannot call gdwg::graph<N, E>::merge_replace_node on old or new data if they don't exist in "
	                    "the graph");
}

TEST_CASE("merge_replace_node: duplicate edges") {
	auto g = gdwg::graph<std::string, int>{"A", "B", "C", "D"};

	g.insert_edge("A", "B", 1);
	g.insert_edge("A", "C", 2);
	g.insert_edge("B", "C", 3);
	g.insert_edge("D", "B", 1);

	g.merge_replace_node("A", "D");

	REQUIRE(g.is_connected("D", "B") == true);
	REQUIRE(g.is_connected("D", "C") == true);

	auto edges = g.edges("D", "B");
	REQUIRE(edges.size() == 1);
}

TEST_CASE("merge_replace_node: no duplicate edges") {
	auto g = gdwg::graph<std::string, int>{"A", "B", "C", "D"};

	g.insert_edge("A", "B", 1);
	g.insert_edge("A", "C", 2);
	g.insert_edge("B", "C", 3);
	g.insert_edge("D", "B", 2);

	g.merge_replace_node("A", "D");

	REQUIRE(g.is_connected("D", "B") == true);
	REQUIRE(g.is_connected("D", "C") == true);

	auto edges = g.edges("D", "B");
	REQUIRE(edges.size() == 2);
	REQUIRE(edges[0]->get_weight().value() == 1);
	REQUIRE(edges[1]->get_weight().value() == 2);
}

TEST_CASE("Erase a weighted edge that exists") {
	auto g = gdwg::graph<std::string, int>{"A", "B"};

	g.insert_edge("A", "B", 1);

	REQUIRE(g.erase_edge("A", "B", 1) == true);
	REQUIRE(g.is_connected("A", "B") == false);
}

TEST_CASE("Erase an unweighted edge that exists") {
	auto g = gdwg::graph<std::string, int>{"A", "B"};
	g.insert_edge("A", "B");

	REQUIRE(g.erase_edge("A", "B") == true);
	REQUIRE(g.is_connected("A", "B") == false);
}

TEST_CASE("Erase an unweighted edge that does not exist") {
	auto g = gdwg::graph<std::string, int>{"A", "B"};

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

TEST_CASE("erase_edge(iterator) function tests", "[graph][erase_edge]") {
	using graph = gdwg::graph<std::string, int>;

	SECTION("Erase the last edge") {
		auto g = graph{"A", "B"};

		g.insert_edge("A", "B", 2);
		g.insert_edge("A", "B", 1);

		auto it = g.find("A", "B", 2);
		auto next_it = g.erase_edge(it);

		REQUIRE(next_it == g.end());

		REQUIRE(g.is_connected("A", "B") == true);
	}
}

TEST_CASE("erase_edge(iterator, iterator) test") {
	auto g = gdwg::graph<std::string, int>{"A", "B", "C", "D"};

	g.insert_edge("A", "B", 1);
	g.insert_edge("A", "C", 2);
	g.insert_edge("A", "D", 3);
	g.insert_edge("B", "C", 4);
	g.insert_edge("C", "D", 5);

	auto it1 = g.find("A", "B", 1);
	auto it2 = g.find("B", "C", 4);

	REQUIRE(it1 != g.end());
	REQUIRE(it2 != g.end());

	auto next_it = g.erase_edge(it1, it2);

	REQUIRE(next_it == it2);

	auto edges = g.edges("A", "B");
	REQUIRE(edges.size() == 0);

	edges = g.edges("A", "D");
	REQUIRE(edges.size() == 0);

	edges = g.edges("B", "C");
	REQUIRE(edges.size() == 1);

	edges = g.edges("C", "D");
	REQUIRE(edges.size() == 1);
}

TEST_CASE("find() function tests", "[graph][find]") {
	using graph = gdwg::graph<std::string, int>;

	SECTION("Edge not found") {
		auto g = graph{};
		REQUIRE(g.find("A", "B") == g.end());
	}

	SECTION("Unweighted edge") {
		auto g = graph{"A", "B"};

		g.insert_edge("A", "B");

		auto it = g.find("A", "B");
		REQUIRE(it != g.end());
		REQUIRE((*it).from == "A");
		REQUIRE((*it).to == "B");
		REQUIRE(not(*it).weight.has_value());
	}

	SECTION("Weighted edge") {
		auto g = graph{"A", "B"};

		g.insert_edge("A", "B", 1);
		g.insert_edge("A", "B", 2);

		auto it1 = g.find("A", "B", 1);
		REQUIRE(it1 != g.end());
		REQUIRE((*it1).from == "A");
		REQUIRE((*it1).to == "B");
		REQUIRE((*it1).weight == 1);

		auto it2 = g.find("A", "B", 2);
		REQUIRE(it2 != g.end());
		REQUIRE((*it2).from == "A");
		REQUIRE((*it2).to == "B");
		REQUIRE((*it2).weight == 2);
	}
}

TEST_CASE("connections() function tests", "[graph][connections]") {
	using graph = gdwg::graph<std::string, int>;

	SECTION("Connections from a node that exists and connections are sorted in ascending order") {
		auto g = graph{"A", "B", "C", "D"};
		g.insert_edge("A", "D", 3);
		g.insert_edge("A", "C", 2);
		g.insert_edge("A", "B", 1);
		std::vector<std::string> expected = {"B", "C", "D"};
		auto result = g.connections("A");
		REQUIRE(result == expected);
	}

	SECTION("Connections from a node with no outgoing edges") {
		auto g = graph{"A", "B"};
		g.insert_edge("A", "B", 1);
		std::vector<std::string> expected = {};
		auto result = g.connections("B");
		REQUIRE(result == expected);
	}

	SECTION("Connections from a node that does not exist") {
		auto g = graph{"A"};
		REQUIRE_THROWS_WITH(g.connections("B"),
		                    "Cannot call gdwg::graph<N, E>::connections if src doesn't exist in the graph");
	}
}

TEST_CASE("Nodes function returns all stored nodes sorted in ascending order", "[graph][nodes]") {
	using graph = gdwg::graph<std::string, int>;

	SECTION("Empty graph") {
		auto g = graph{};
		std::vector<std::string> expected = {};
		auto result = g.nodes();
		REQUIRE(result == expected);
	}

	SECTION("Single node") {
		auto g = graph{"A"};
		std::vector<std::string> expected = {"A"};
		auto result = g.nodes();
		REQUIRE(result == expected);
	}

	SECTION("Multiple nodes, sorted in ascending order") {
		auto g = graph{"D", "C", "A", "B"};
		std::vector<std::string> expected = {"A", "B", "C", "D"};
		auto result = g.nodes();
		REQUIRE(result == expected);
	}

	SECTION("Nodes with edges") {
		auto g = graph{"C", "A", "B"};
		g.insert_edge("A", "B", 1);
		g.insert_edge("B", "C", 2);
		std::vector<std::string> expected = {"A", "B", "C"};
		auto result = g.nodes();
		REQUIRE(result == expected);
	}
}

TEST_CASE("edges function tests", "[graph][edges]") {
	using graph = gdwg::graph<std::string, int>;

	SECTION("Edges from existing nodes") {
		auto g = graph{"A", "B", "C"};

		g.insert_edge("A", "B", 2);
		g.insert_edge("A", "B", 1);
		g.insert_edge("A", "C", 3);

		auto result = g.edges("A", "B");
		REQUIRE(result.size() == 2);

		REQUIRE(result[0]->get_nodes() == std::make_pair(std::string("A"), std::string("B")));
		REQUIRE(result[0]->get_weight().value() == 1);

		REQUIRE(result[1]->get_nodes() == std::make_pair(std::string("A"), std::string("B")));
		REQUIRE(result[1]->get_weight().value() == 2);
	}

	SECTION("No edges between nodes") {
		auto g = graph{"A", "B", "C"};

		g.insert_edge("A", "C", 2);
		g.insert_edge("B", "C", 3);

		auto result = g.edges("A", "B");
		REQUIRE(result.empty());
	}

	SECTION("Edges from non-existing src node") {
		auto g = graph{"A", "B"};

		g.insert_edge("A", "B", 2);

		REQUIRE_THROWS_WITH(g.edges("C", "B"),
		                    "Cannot call gdwg::graph<N, E>::edges if src or dst node don't exist in the graph");
	}

	SECTION("Edges from non-existing dst node") {
		auto g = graph{"A", "B"};

		g.insert_edge("A", "B", 2);

		REQUIRE_THROWS_WITH(g.edges("A", "C"),
		                    "Cannot call gdwg::graph<N, E>::edges if src or dst node don't exist in the graph");
	}

	SECTION("Edges are sorted in ascending order by weight") {
		auto g = graph{"A", "B", "C"};

		g.insert_edge("A", "B", 10);
		g.insert_edge("A", "B", 1);
		g.insert_edge("A", "B", 5);

		auto result = g.edges("A", "B");
		REQUIRE(result.size() == 3);

		REQUIRE(result[0]->get_weight().value() == 1);
		REQUIRE(result[1]->get_weight().value() == 5);
		REQUIRE(result[2]->get_weight().value() == 10);
	}

	SECTION("Edges include unweighted and weighted edges sorted correctly") {
		auto g = graph{"A", "B"};

		g.insert_edge("A", "B", 5);
		g.insert_edge("A", "B", 1);
		g.insert_edge("A", "B");

		auto result = g.edges("A", "B");
		REQUIRE(result.size() == 3);

		REQUIRE(result[0]->is_weighted() == false);
		REQUIRE(result[1]->get_weight().value() == 1);
		REQUIRE(result[2]->get_weight().value() == 5);
	}
}

TEST_CASE("Test equality operator for gdwg::graph", "[graph][operator==]") {
	using graph = gdwg::graph<std::string, int>;

	SECTION("Test equality operator for empty graphs") {
		auto g1 = graph{};
		auto g2 = graph{};

		REQUIRE(g1 == g2);
	}

	SECTION("Test equality operator for graphs with same nodes and edges") {
		auto g1 = graph{};

		g1.insert_node("A");
		g1.insert_node("B");
		g1.insert_node("C");
		g1.insert_edge("A", "B", 1);
		g1.insert_edge("B", "C", 2);
		g1.insert_edge("C", "A", 3);

		auto g2 = graph{};

		g2.insert_node("A");
		g2.insert_node("B");
		g2.insert_node("C");
		g2.insert_edge("A", "B", 1);
		g2.insert_edge("B", "C", 2);
		g2.insert_edge("C", "A", 3);

		REQUIRE(g1 == g2);
	}

	SECTION("Test equality operator for graphs with different nodes") {
		auto g1 = graph{};

		g1.insert_node("A");
		g1.insert_node("B");

		auto g2 = graph{};

		g2.insert_node("A");
		g2.insert_node("B");
		g2.insert_node("C");

		REQUIRE_FALSE(g1 == g2);
	}

	SECTION("Test equality operator for graphs with different edge weights") {
		auto g1 = graph{"A", "B"};
		g1.insert_edge("A", "B", 1);

		auto g2 = graph{"A", "B"};
		g2.insert_edge("A", "B", 2);

		REQUIRE_FALSE(g1 == g2);
	}

	SECTION("Test equality operator for graphs with same nodes but different connections") {
		auto g1 = graph{"A", "B"};
		g1.insert_edge("A", "B", 1);

		auto g2 = graph{"A", "B"};
		g2.insert_edge("B", "A", 1);

		REQUIRE_FALSE(g1 == g2);
	}

	SECTION("Test equality operator for complex graphs") {
		auto g1 = graph{"A", "B", "C", "D"};
		g1.insert_edge("A", "B", 1);
		g1.insert_edge("A", "B", 2);
		g1.insert_edge("B", "C", 3);
		g1.insert_edge("C", "D", 4);

		auto g2 = graph{"A", "B", "C", "D"};
		g2.insert_edge("A", "B", 1);
		g2.insert_edge("A", "B", 2);
		g2.insert_edge("B", "C", 3);
		g2.insert_edge("C", "D", 4);

		REQUIRE(g1 == g2);
	}
}