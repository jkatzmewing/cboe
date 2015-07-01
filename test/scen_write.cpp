
#include <fstream>
#include <iostream>
#include "tinyprint.h"
#include "dialog.hpp"
#include "catch.hpp"
#include "scenario.hpp"
#include "regtown.hpp"

using namespace std;
using namespace ticpp;

extern Document xmlDocFromStream(istream& stream, string name);
extern void readScenarioFromXml(Document&& data, cScenario& scenario);
extern void writeScenarioToXml(Printer&& data, cScenario& scenario);

static void in_and_out(std::string name, cScenario& scen) {
	std::string fpath = "junk/";
	fpath += name;
	fpath += ".xml";
	ofstream fout;
	fout.exceptions(ios::badbit);
	fout.open(fpath);
	writeScenarioToXml(Printer(name, fout), scen);
	fout.close();
	// Reconstruct the scenario, to ensure that it doesn't just pass due to old data still being around
	scen.~cScenario();
	new(&scen) cScenario();
	ifstream fin;
	fin.exceptions(ios::badbit);
	fin.open(fpath);
	readScenarioFromXml(xmlDocFromStream(fin, name), scen);
}

// NOTE: The test cases in this file are written with the implicit assumption that the read routines are trustworthy.
// In other words, they depend on the test cases in scen_read.cpp.

TEST_CASE("Saving a scenario record") {
	cScenario scen;
	scen.reset_version();
	scen.format.ver[0] = 2;
	scen.format.ver[1] = 6;
	scen.format.ver[2] = 7;
	scenario_header_flags vers = scen.format;
	scen.scen_name = "Test Scenario";
	scen.intro_pic = 0;
	scen.campaign_id = "campaign";
	scen.who_wrote[0] = "Teaser 1";
	scen.who_wrote[1] = "Teaser 2";
	scen.contact_info[0] = "BoE Test Suite";
	scen.contact_info[1] = "nowhere@example.com";
	scen.intro_strs[0] = "Welcome to the test scenario!";
	scen.rating = 2;
	scen.difficulty = 2;
	scen.which_town_start = 7;
	scen.where_start = {24,28};
	scen.out_sec_start = {1,3};
	scen.out_start = {12, 21};
	SECTION("With basic header data") {
		in_and_out("basic", scen);
		CHECK(scen.format.prog_make_ver[0] == vers.prog_make_ver[0]);
		CHECK(scen.format.prog_make_ver[1] == vers.prog_make_ver[1]);
		CHECK(scen.format.prog_make_ver[2] == vers.prog_make_ver[2]);
		CHECK(scen.format.ver[0] == vers.ver[0]);
		CHECK(scen.format.ver[1] == vers.ver[1]);
		CHECK(scen.format.ver[2] == vers.ver[2]);
		CHECK(scen.scen_name == "Test Scenario");
		CHECK(scen.intro_pic == 0);
		CHECK(scen.campaign_id == "campaign");
		CHECK(scen.who_wrote[0] == "Teaser 1");
		CHECK(scen.who_wrote[1] == "Teaser 2");
		CHECK(scen.contact_info[0] == "BoE Test Suite");
		CHECK(scen.contact_info[1] == "nowhere@example.com");
		CHECK(scen.intro_strs[0] == "Welcome to the test scenario!");
		CHECK(scen.rating == 2);
		CHECK(scen.difficulty == 2);
		CHECK(scen.which_town_start == 7);
		CHECK(scen.where_start == loc(24,28));
		CHECK(scen.out_sec_start == loc(1,3));
		CHECK(scen.out_start == loc(12,21));
	}
	SECTION("With some towns and sectors") {
		scen.addTown<cTinyTown>();
		scen.addTown<cMedTown>();
		scen.addTown<cBigTown>();
		scen.outdoors.resize(2,5);
		in_and_out("town&sector", scen);
		CHECK(scen.towns.size() == 3);
		CHECK(scen.outdoors.width() == 2);
		CHECK(scen.outdoors.height() == 5);
	}
	SECTION("With some optional header data") {
		REQUIRE(scen.town_mods.size() >= 1); // A safety valve for if I ever make this array dynamic
		scen.town_mods[0] = loc(12,9);
		scen.town_mods[0].spec = 4;
		REQUIRE(scen.store_item_towns.size() >= 1); // A safety valve for if I ever make this array dynamic
		REQUIRE(scen.store_item_rects.size() >= 1); // A safety valve for if I ever make this array dynamic
		scen.store_item_rects[0] = rect(1,2,3,4);
		scen.store_item_towns[0] = 5;
		REQUIRE(scen.scenario_timers.size() >= 1); // A safety valve for if I ever make this array dynamic
		scen.scenario_timers[0].node = 3;
		scen.scenario_timers[0].node_type = 1;
		scen.scenario_timers[0].time = 30000;
		scen.spec_strs.push_back("This is a sample special string!");
		scen.journal_strs.push_back("This is a sample journal string!");
		in_and_out("optional", scen);
		CHECK(scen.town_mods[0] == loc(12,9));
		CHECK(scen.town_mods[0].spec == 4);
		CHECK(scen.store_item_rects[0] == rect(1,2,3,4));
		CHECK(scen.store_item_towns[0] == 5);
		CHECK(scen.scenario_timers[0].node == 3);
		CHECK(scen.scenario_timers[0].node_type == 0); // This is inferred by the fact that it's in the scenario file
		CHECK(scen.scenario_timers[0].time == 30000);
		CHECK(scen.spec_strs.size() == 1);
		CHECK(scen.spec_strs[0] == "This is a sample special string!");
		CHECK(scen.journal_strs.size() == 1);
		CHECK(scen.journal_strs[0] == "This is a sample journal string!");
	}
	SECTION("With a special item") {
		scen.special_items.emplace_back();
		scen.special_items[0].flags = 11;
		scen.special_items[0].special = 2;
		scen.special_items[0].name = "Test Special Item";
		scen.special_items[0].descr = "This is a special item description!";
		in_and_out("special item", scen);
		CHECK(scen.special_items.size() == 1);
		CHECK(scen.special_items[0].flags == 11);
		CHECK(scen.special_items[0].special == 2);
		CHECK(scen.special_items[0].name == "Test Special Item");
		CHECK(scen.special_items[0].descr == "This is a special item description!");
	}
}
