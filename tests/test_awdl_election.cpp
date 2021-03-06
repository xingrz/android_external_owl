/*
 * OWL: an open Apple Wireless Direct Link (AWDL) implementation
 * Copyright (C) 2018  The Open Wireless Link Project (https://owlink.org)
 * Copyright (C) 2018  Milan Stute
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

extern "C" {
#include "election.h"
#include "peers.h"
}

#include "gtest/gtest.h"

#define TEST_ADDR(i) static const struct ether_addr TEST_ADDR##i = {{ i, i, i, i, i, i }}

TEST_ADDR(0);
TEST_ADDR(1);

#define ASSERT_ETHEREQ(val1, val2) ASSERT_FALSE(compare_ether_addr(&val1, &val2))
#define ASSERT_ETHERNEQ(val1, val2) ASSERT_TRUE(compare_ether_addr(&val1, &val2))

TEST(awdl_election, init) {
	struct awdl_election_state s;
	awdl_election_state_init(&s, &TEST_ADDR0);

	ASSERT_EQ(s.self_counter, 0);
	ASSERT_EQ(s.self_metric, 60);
	ASSERT_EQ(s.master_counter, 0);
	ASSERT_EQ(s.master_metric, 60);
	ASSERT_EQ(s.height, 0);
	ASSERT_ETHEREQ(s.self_addr, TEST_ADDR0);

	ASSERT_ETHEREQ(s.master_addr, TEST_ADDR0);
	ASSERT_ETHEREQ(s.sync_addr, TEST_ADDR0);
	ASSERT_ETHEREQ(s.self_addr, TEST_ADDR0);
}

TEST(awdl_election, elect_simple) {
	struct awdl_election_state s;
	struct awdl_peer_state p;
	awdl_election_state_init(&s, &TEST_ADDR0);
	awdl_peer_state_init(&p);

	awdl_peer_add(p.peers, &TEST_ADDR1, 0, NULL, NULL);

	awdl_election_run(&s, &p);

	ASSERT_ETHEREQ(s.master_addr, TEST_ADDR1);
}

TEST(awdl_election, counter_metric) {
	struct awdl_election_state s;
	struct awdl_peer_state p;
	struct awdl_peer *peer;
	awdl_election_state_init(&s, &TEST_ADDR0);
	awdl_peer_state_init(&p);

	awdl_peer_add(p.peers, &TEST_ADDR1, 0, NULL, NULL);
	awdl_peer_get(p.peers, &TEST_ADDR1, &peer);

	awdl_election_run(&s, &p);
	ASSERT_ETHEREQ(s.master_addr, TEST_ADDR1);

	peer->election.master_metric = 1000;
	awdl_election_run(&s, &p);
	ASSERT_ETHEREQ(s.master_addr, TEST_ADDR1);

	s.self_metric = 1000;
	awdl_election_run(&s, &p);
	ASSERT_ETHEREQ(s.master_addr, TEST_ADDR1);

	s.self_metric = 1001;
	awdl_election_run(&s, &p);
	ASSERT_ETHEREQ(s.master_addr, TEST_ADDR0);

	peer->election.master_counter = 1;
	awdl_election_run(&s, &p);
	ASSERT_ETHEREQ(s.master_addr, TEST_ADDR1);

	s.self_counter = 1;
	awdl_election_run(&s, &p);
	ASSERT_ETHEREQ(s.master_addr, TEST_ADDR0);
}


TEST(awdl_election, elect_cycle) {
	struct awdl_election_state s;
	struct awdl_peer_state p;
	struct awdl_peer *peer;
	awdl_election_state_init(&s, &TEST_ADDR0);
	awdl_peer_state_init(&p);

	awdl_peer_add(p.peers, &TEST_ADDR1, 0, NULL, NULL);
	awdl_peer_get(p.peers, &TEST_ADDR1, &peer);

	peer->election.master_metric = 1000;
	peer->election.self_metric = 1000;
	/* add top master */
	peer->election.height = 1;
	peer->election.master_addr = TEST_ADDR0;

	awdl_election_run(&s, &p);

	ASSERT_ETHEREQ(s.master_addr, TEST_ADDR0);
}
