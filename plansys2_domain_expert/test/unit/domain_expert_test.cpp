// Copyright 2019 Intelligent Robotics Lab
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <string>
#include <vector>
#include <regex>
#include <iostream>
#include <memory>

#include "ament_index_cpp/get_package_share_directory.hpp"

#include "gtest/gtest.h"
#include "plansys2_domain_expert/DomainExpert.hpp"


std::string getReducedString(const std::string & expr)  // Same function in Types.cpp
{
  std::regex nts_chars("[\n\t]*", std::regex_constants::ECMAScript);
  std::string ret = std::regex_replace(expr, nts_chars, "");
  std::regex open_paren("\\( ", std::regex_constants::ECMAScript);
  ret = std::regex_replace(ret, open_paren, "(");
  std::regex close_paren(" \\)", std::regex_constants::ECMAScript);
  ret = std::regex_replace(ret, close_paren, ")");
  return ret;
}

TEST(domain_expert, functions)
{
  std::string my_string("(and)");
  ASSERT_EQ(getReducedString(my_string), "(and)");

  my_string = "( and)";
  ASSERT_EQ(getReducedString(my_string), "(and)");

  my_string = "( \tand)";
  ASSERT_EQ(getReducedString(my_string), "(and)");

  my_string = "( \tand\t)";
  ASSERT_EQ(getReducedString(my_string), "(and)");

  my_string = "( and\n)";
  ASSERT_EQ(getReducedString(my_string), "(and)");
  my_string = "( and\n\t)";
  ASSERT_EQ(getReducedString(my_string), "(and)");
  my_string = "( ( and\n\t ) )";
  ASSERT_EQ(getReducedString(my_string), "((and))");
}

TEST(domain_expert, get_domain)
{
  std::string pkgpath = ament_index_cpp::get_package_share_directory("plansys2_domain_expert");
  std::ifstream domain_ifs(pkgpath + "/pddl/domain_simple.pddl");
  std::string domain_str((
      std::istreambuf_iterator<char>(domain_ifs)),
    std::istreambuf_iterator<char>());

  std::ifstream domain_ifs_p(pkgpath + "/pddl/domain_simple_processed.pddl");
  std::string domain_str_p((
      std::istreambuf_iterator<char>(domain_ifs_p)),
    std::istreambuf_iterator<char>());

  plansys2::DomainExpert domain_expert(domain_str);
  ASSERT_EQ(domain_expert.getDomain(), domain_str_p);
}

TEST(domain_expert, get_domain2)
{
  std::string pkgpath = ament_index_cpp::get_package_share_directory("plansys2_domain_expert");
  std::ifstream domain_ifs(pkgpath + "/pddl/factory.pddl");
  std::string domain_str((
      std::istreambuf_iterator<char>(domain_ifs)),
    std::istreambuf_iterator<char>());

  std::ifstream domain_ifs_p(pkgpath + "/pddl/factory_processed.pddl");
  std::string domain_str_p((
      std::istreambuf_iterator<char>(domain_ifs_p)),
    std::istreambuf_iterator<char>());

  plansys2::DomainExpert domain_expert(domain_str);
  ASSERT_EQ(domain_expert.getDomain(), domain_str_p);
}


TEST(domain_expert, get_types)
{
  std::string pkgpath = ament_index_cpp::get_package_share_directory("plansys2_domain_expert");
  std::ifstream domain_ifs(pkgpath + "/pddl/domain_simple.pddl");
  std::string domain_str((
      std::istreambuf_iterator<char>(domain_ifs)),
    std::istreambuf_iterator<char>());

  plansys2::DomainExpert domain_expert(domain_str);

  std::vector<std::string> types = domain_expert.getTypes();
  std::vector<std::string> test_types {"person", "message", "robot", "room", "teleporter_room"};

  ASSERT_EQ(types, test_types);
}


TEST(domain_expert, get_predicates)
{
  std::string pkgpath = ament_index_cpp::get_package_share_directory("plansys2_domain_expert");
  std::ifstream domain_ifs(pkgpath + "/pddl/domain_simple.pddl");
  std::string domain_str((
      std::istreambuf_iterator<char>(domain_ifs)),
    std::istreambuf_iterator<char>());

  plansys2::DomainExpert domain_expert(domain_str);

  std::vector<std::string> predicates = domain_expert.getPredicates();
  std::vector<std::string> predicates_types {"person_at", "robot_at", "robot_near_person",
    "robot_talk"};

  ASSERT_EQ(predicates, predicates_types);
}

TEST(domain_expert, get_predicate_params)
{
  std::string pkgpath = ament_index_cpp::get_package_share_directory("plansys2_domain_expert");
  std::ifstream domain_ifs(pkgpath + "/pddl/domain_simple.pddl");
  std::string domain_str((
      std::istreambuf_iterator<char>(domain_ifs)),
    std::istreambuf_iterator<char>());

  plansys2::DomainExpert domain_expert(domain_str);

  auto params_1 = domain_expert.getPredicate("robot_talk");
  ASSERT_TRUE(params_1);
  ASSERT_EQ(params_1.value().name, "robot_talk");
  ASSERT_EQ(params_1.value().parameters.size(), 3);
  ASSERT_EQ(params_1.value().parameters[0].name, "?robot0");
  ASSERT_EQ(params_1.value().parameters[0].type, "robot");
  ASSERT_EQ(params_1.value().parameters[1].name, "?message1");
  ASSERT_EQ(params_1.value().parameters[1].type, "message");
  ASSERT_EQ(params_1.value().parameters[2].name, "?person2");
  ASSERT_EQ(params_1.value().parameters[2].type, "person");

  auto params_2 = domain_expert.getPredicate("ROBOT_TALK");
  ASSERT_TRUE(params_2);

  auto params_3 = domain_expert.getPredicate("person_at");
  ASSERT_TRUE(params_3);
  ASSERT_EQ(params_3.value().parameters.size(), 2);
  ASSERT_EQ(params_3.value().parameters[0].name, "?person0");
  ASSERT_EQ(params_3.value().parameters[0].type, "person");
  ASSERT_EQ(params_3.value().parameters[1].name, "?room1");
  ASSERT_EQ(params_3.value().parameters[1].type, "room");
}

TEST(domain_expert, get_actions)
{
  std::string pkgpath = ament_index_cpp::get_package_share_directory("plansys2_domain_expert");
  std::ifstream domain_ifs(pkgpath + "/pddl/domain_simple.pddl");
  std::string domain_str((
      std::istreambuf_iterator<char>(domain_ifs)),
    std::istreambuf_iterator<char>());

  plansys2::DomainExpert domain_expert(domain_str);

  auto actions = domain_expert.getActions();
  ASSERT_EQ(actions.size(), 1);
  ASSERT_EQ(actions[0], "move_person");

  auto durative_actions = domain_expert.getDurativeActions();
  ASSERT_EQ(durative_actions.size(), 3);
  ASSERT_EQ(durative_actions[0], "move");
  ASSERT_EQ(durative_actions[1], "talk");
  ASSERT_EQ(durative_actions[2], "approach");
}


TEST(domain_expert, get_action_params)
{
  std::string pkgpath = ament_index_cpp::get_package_share_directory("plansys2_domain_expert");
  std::ifstream domain_ifs(pkgpath + "/pddl/domain_simple.pddl");
  std::string domain_str((
      std::istreambuf_iterator<char>(domain_ifs)),
    std::istreambuf_iterator<char>());

  plansys2::DomainExpert domain_expert(domain_str);

  domain_expert.getAction("move");
  auto no_action = domain_expert.getAction("noexist");
  ASSERT_FALSE(no_action);

  auto move_action = domain_expert.getDurativeAction("move");
  auto at_start = move_action.value().at_start_requirements;
  ASSERT_EQ(at_start.toString(), "(and (robot_at ?0 ?1))");

  ASSERT_TRUE(move_action);
  ASSERT_EQ(move_action.value().name, "move");
  ASSERT_EQ(move_action.value().parameters.size(), 3);
  ASSERT_EQ(move_action.value().parameters[0].name, "?0");
  ASSERT_EQ(move_action.value().parameters[0].type, "robot");
  ASSERT_EQ(move_action.value().parameters[1].name, "?1");
  ASSERT_EQ(move_action.value().parameters[1].type, "room");
  ASSERT_EQ(move_action.value().parameters[2].name, "?2");
  ASSERT_EQ(move_action.value().parameters[2].type, "room");

  ASSERT_FALSE(at_start.empty());
  ASSERT_TRUE(move_action.value().over_all_requirements.empty());
  ASSERT_TRUE(move_action.value().at_end_requirements.empty());

  ASSERT_EQ(
    move_action.value().at_start_requirements.toString(),
    "(and (robot_at ?0 ?1))");
  ASSERT_EQ(
    move_action.value().at_start_effects.toString(),
    "(and (not (robot_at ?0 ?1)))");
  ASSERT_EQ(
    move_action.value().at_end_effects.toString(),
    "(and (robot_at ?0 ?2))");
}

TEST(domain_expert, multidomain_get_types)
{
  std::string pkgpath = ament_index_cpp::get_package_share_directory("plansys2_domain_expert");
  std::ifstream domain_ifs(pkgpath + "/pddl/domain_simple.pddl");
  std::string domain_str((
      std::istreambuf_iterator<char>(domain_ifs)),
    std::istreambuf_iterator<char>());

  auto domain_expert = std::make_shared<plansys2::DomainExpert>(domain_str);

  std::ifstream domain_ext_ifs(pkgpath + "/pddl/domain_simple_ext.pddl");
  std::string domain_ext_str((
      std::istreambuf_iterator<char>(domain_ext_ifs)),
    std::istreambuf_iterator<char>());

  domain_expert->extendDomain(domain_ext_str);

  std::vector<std::string> types = domain_expert->getTypes();
  std::vector<std::string> test_types {"person", "message", "robot", "room", "teleporter_room",
    "pickable_object"};

  ASSERT_EQ(types, test_types);

  std::vector<std::string> predicates = domain_expert->getPredicates();
  std::vector<std::string> test_predicates {"object_at_robot", "object_at_room", "person_at",
    "robot_at", "robot_near_person", "robot_talk"};

  ASSERT_EQ(predicates, test_predicates);

  std::vector<std::string> actions = domain_expert->getActions();
  std::vector<std::string> test_actions {"move_person"};

  ASSERT_EQ(actions, test_actions);

  std::vector<std::string> dactions = domain_expert->getDurativeActions();
  std::vector<std::string> test_dactions {"move", "talk", "approach", "pick_object",
    "place_object"};

  ASSERT_EQ(dactions, test_dactions);
}

TEST(domain_expert, sub_types)
{
  std::string pkgpath = ament_index_cpp::get_package_share_directory("plansys2_domain_expert");
  std::ifstream domain_ifs(pkgpath + "/pddl/domain_simple.pddl");
  std::string domain_str((
      std::istreambuf_iterator<char>(domain_ifs)),
    std::istreambuf_iterator<char>());

  plansys2::DomainExpert domain_expert(domain_str);

  // Parameter subtypes with a durative-action
  std::optional<plansys2::DurativeAction> durative_action =
    domain_expert.getDurativeAction("move");
  if (durative_action.has_value()) {
    if (durative_action.value().parameters.size() == 3) {
      ASSERT_EQ(durative_action.value().parameters[1].type, "room");

      std::vector<std::string> subtypes {"teleporter_room"};
      ASSERT_EQ(durative_action.value().parameters[1].subTypes, subtypes);
    } else {
      FAIL() << "The `move` durative-action is expected to have 2 parameters";
    }
  } else {
    FAIL() << "No `move` durative-action found in the domain";
  }

  // Parameter subtypes with a predicate
  std::optional<plansys2::Predicate> predicate =
    domain_expert.getPredicate("robot_at");
  if (predicate.has_value()) {
    if (predicate.value().parameters.size() == 2) {
      ASSERT_EQ(predicate.value().parameters[1].type, "room");

      std::vector<std::string> subtypes {"teleporter_room"};
      ASSERT_EQ(predicate.value().parameters[1].subTypes, subtypes);
    } else {
      FAIL() << "The `robot_at` predicate is expected to have 3 parameters";
    }
  } else {
    FAIL() << "No `robot_at` predicate found in the domain";
  }

  // Parameter subtypes with as action
  std::optional<plansys2::Action> action =
    domain_expert.getAction("move_person");
  if (action.has_value()) {
    if (action.value().parameters.size() == 3) {
      ASSERT_EQ(action.value().parameters[1].type, "room");

      std::vector<std::string> subtypes {"teleporter_room"};
      ASSERT_EQ(action.value().parameters[1].subTypes, subtypes);
    } else {
      FAIL() << "The `move_person` action is expected to have 3 parameters";
    }
  } else {
    FAIL() << "No `move_person` action found in the domain";
  }

  // Parameter subtypes with as function
  std::optional<plansys2::Function> function =
    domain_expert.getFunction("teleportation_time");
  if (function.has_value()) {
    if (function.value().parameters.size() == 2) {
      ASSERT_EQ(function.value().parameters[0].type, "teleporter_room");
      ASSERT_EQ(function.value().parameters[1].type, "room");

      std::vector<std::string> emptySubtypes {};
      ASSERT_EQ(function.value().parameters[0].subTypes, emptySubtypes);

      std::vector<std::string> subtypes {"teleporter_room"};
      ASSERT_EQ(function.value().parameters[1].subTypes, subtypes);
    } else {
      FAIL() << "The `teleportation_time` function is expected to have 2 parameters";
    }
  } else {
    FAIL() << "No `teleportation_time` function found in the domain";
  }
}

int main(int argc, char ** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
