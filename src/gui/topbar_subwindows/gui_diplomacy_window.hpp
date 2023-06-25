#pragma once

#include "dcon_generated.hpp"
#include "gui_element_types.hpp"
#include "gui_graphics.hpp"
#include "text.hpp"
#include "parsers.hpp"
#include <algorithm>
#include <functional>

#include "gui_diplomacy_actions_window.hpp"
#include "gui_declare_war_window.hpp"
#include "gui_crisis_window.hpp"

namespace ui {

enum class diplomacy_window_tab : uint8_t { great_powers = 0x0, wars = 0x1, casus_belli = 0x2, crisis = 0x3 };

class diplomacy_nation_navies_text : public nation_num_ships {
public:
	tooltip_behavior has_tooltip(sys::state& state) noexcept override {
		return tooltip_behavior::variable_tooltip;
	}

	void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
		if(parent) {
			if(state.user_settings.use_new_ui) {
				auto n = retrieve<dcon::nation_id>(state, parent);
				text::add_line(state, contents, "diplomacy_ships", text::variable_type::value, military::total_ships(state, n));
				text::add_line_break_to_layout(state, contents);

				int32_t total = 0;
				int32_t discovered = 0;
				state.world.for_each_technology([&](dcon::technology_id id) {
					auto fat_id = dcon::fatten(state.world, id);
					if(state.culture_definitions.tech_folders[fat_id.get_folder_index()].category != culture::tech_category::navy)
						return;
					++total;
					if(state.world.nation_get_active_technologies(n, id))
						++discovered;
				});

				text::add_line(state, contents, "navy_technology_levels", text::variable_type::val, discovered, text::variable_type::max, total);
			} else {
				Cyto::Any payload = dcon::nation_id{};
				parent->impl_get(state, payload);
				auto nation_id = any_cast<dcon::nation_id>(payload);

				auto box = text::open_layout_box(contents, 0);
				text::localised_single_sub_box(state, contents, box, std::string_view("diplomacy_ships"), text::variable_type::value,
						military::total_ships(state, nation_id));
				text::add_divider_to_layout_box(state, contents, box);
				text::localised_format_box(state, contents, box, std::string_view("navy_technology_levels"));
				text::close_layout_box(contents, box);
			}
		}
	}
};

class diplomacy_nation_armies_text : public nation_num_regiments {
public:
	tooltip_behavior has_tooltip(sys::state& state) noexcept override {
		return tooltip_behavior::variable_tooltip;
	}

	void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
		if(parent) {
			if(state.user_settings.use_new_ui) {
				auto n = retrieve<dcon::nation_id>(state, parent);
				text::add_line(state, contents, "diplomacy_brigades", text::variable_type::value, military::total_regiments(state, n));
				text::add_line_break_to_layout(state, contents);

				int32_t total = 0;
				int32_t discovered = 0;
				state.world.for_each_technology([&](dcon::technology_id id) {
					auto fat_id = dcon::fatten(state.world, id);
					if(state.culture_definitions.tech_folders[fat_id.get_folder_index()].category != culture::tech_category::army)
						return;
					++total;
					if(state.world.nation_get_active_technologies(n, id))
						++discovered;
				});

				text::add_line(state, contents, "army_technology_levels", text::variable_type::val, discovered, text::variable_type::max, total);
			} else {
				Cyto::Any payload = dcon::nation_id{};
				parent->impl_get(state, payload);
				auto nation_id = any_cast<dcon::nation_id>(payload);

				auto num = dcon::fatten(state.world, nation_id).get_active_regiments();
				auto box = text::open_layout_box(contents, 0);
				text::localised_single_sub_box(state, contents, box, std::string_view("diplomacy_brigades"), text::variable_type::value,
						num);
				text::add_divider_to_layout_box(state, contents, box);
				text::localised_format_box(state, contents, box, std::string_view("army_technology_levels"));
				text::add_line_break_to_layout_box(state, contents, box);
				text::localised_format_box(state, contents, box, std::string_view("mil_tactics_tech"));
				text::close_layout_box(contents, box);
			}
		}
	}
};

class diplomacy_war_exhaustion : public standard_nation_text {
public:
	std::string get_text(sys::state& state, dcon::nation_id nation_id) noexcept override {
		auto fat_id = dcon::fatten(state.world, nation_id);
		return text::format_percentage(fat_id.get_war_exhaustion(), 1);
	}

	tooltip_behavior has_tooltip(sys::state& state) noexcept override {
		return tooltip_behavior::variable_tooltip;
	}

	void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {	
		auto n = retrieve<dcon::nation_id>(state, parent);
		if(state.user_settings.use_new_ui) {
			auto box = text::open_layout_box(contents);
			text::localised_format_box(state, contents, box, "diplomacy_wx_1");
			auto mod = state.world.nation_get_modifier_values(n, sys::national_mod_offsets::war_exhaustion);
			if(mod > 0) {
				text::add_to_layout_box(state, contents, box, std::string_view("+"), text::text_color::red);
				text::add_to_layout_box(state, contents, box, text::fp_two_places{mod}, text::text_color::red);
			} else if(mod == 0) {
				text::add_to_layout_box(state, contents, box, std::string_view("+0"));
			} else {
				text::add_to_layout_box(state, contents, box, text::fp_two_places{mod}, text::text_color::green);
			}
			text::close_layout_box(contents, box);

			active_modifiers_description(state, contents, n, 15, sys::national_mod_offsets::war_exhaustion, false);
		} else {
			auto num = dcon::fatten(state.world, n).get_war_exhaustion();
			auto box = text::open_layout_box(contents, 0);
			text::localised_single_sub_box(state, contents, box, std::string_view("diplomacy_war_exhaustion"),
					text::variable_type::value, text::fp_percentage{num});
			// TODO - check if the nation is at peace, if it is then we display stuff
			text::close_layout_box(contents, box);

			active_modifiers_description(state, contents, n, 0, sys::national_mod_offsets::war_exhaustion, false);
		}
	}
};

class diplomacy_country_select : public button_element_base {
public:
	void on_update(sys::state& state) noexcept override {
		if(!parent)
			return;
		auto pp = parent->parent;
		auto content = retrieve<dcon::nation_id>(state, parent);
		auto window_content = retrieve<dcon::nation_id>(state, pp);
		if(content == window_content)
			frame = 1;
		else
			frame = 0;
	}
	void button_action(sys::state& state) noexcept override {
		auto content = retrieve<dcon::nation_id>(state, parent);
		if(content)
			send(state, parent, element_selection_wrapper<dcon::nation_id>{content});
	}
};

void explain_influence(sys::state& state, dcon::nation_id target, text::columnar_layout& contents) {
	int32_t total_influence_shares = 0;
	auto n = fatten(state.world, state.local_player_nation);

	for(auto rel : state.world.nation_get_gp_relationship_as_great_power(state.local_player_nation)) {
		if(nations::can_accumulate_influence_with(state, state.local_player_nation, rel.get_influence_target(), rel)) {
			switch(rel.get_status() & nations::influence::priority_mask) {
			case nations::influence::priority_one:
				total_influence_shares += 1;
				break;
			case nations::influence::priority_two:
				total_influence_shares += 2;
				break;
			case nations::influence::priority_three:
				total_influence_shares += 3;
				break;
			default:
			case nations::influence::priority_zero:
				break;
			}
		}
	}

	auto rel = fatten(state.world, state.world.get_gp_relationship_by_gp_influence_pair(target, state.local_player_nation));

	if((state.world.gp_relationship_get_status(rel) & nations::influence::is_banned) != 0) {
		text::add_line(state, contents, "influence_explain_1");
		return;
	}
	if(military::has_truce_with(state, state.local_player_nation, target)) {
		text::add_line(state, contents, "influence_explain_2");
		return;
	}
	if(military::are_at_war(state, state.local_player_nation, target)) {
		text::add_line(state, contents, "influence_explain_3");
		return;
	}

	float total_gain = state.defines.base_greatpower_daily_influence * (1.0f + n.get_modifier_values(sys::national_mod_offsets::influence_modifier)) * (1.0f + n.get_modifier_values(sys::national_mod_offsets::influence));

	float gp_score = n.get_industrial_score() + n.get_military_score() + nations::prestige_score(state, n);
	
	float base_shares = [&]() {
		switch(rel.get_status() & nations::influence::priority_mask) {
			case nations::influence::priority_one:
				return total_gain / float(total_influence_shares);
			case nations::influence::priority_two:
				return 2.0f * total_gain / float(total_influence_shares);
			case nations::influence::priority_three:
				return 3.0f * total_gain / float(total_influence_shares);
			default:
			case nations::influence::priority_zero:
				return 0.0f;
			}
	}();

	bool has_sphere_neighbor = [&]() {
		for(auto g : state.world.nation_get_nation_adjacency(target)) {
			if(g.get_connected_nations(0) != target && g.get_connected_nations(0).get_in_sphere_of() == n)
				return true;
			if(g.get_connected_nations(1) != target && g.get_connected_nations(1).get_in_sphere_of() == n)
				return true;
			}
		return false;
	}();

	float total_fi = 0.0f;
	for(auto i : state.world.nation_get_unilateral_relationship_as_target(target)) {
		total_fi += i.get_foreign_investment();
	}
	auto gp_invest = state.world.unilateral_relationship_get_foreign_investment(
			state.world.get_unilateral_relationship_by_unilateral_pair(target, n));

	float discredit_factor =
		(rel.get_status() & nations::influence::is_discredited) != 0
		? state.defines.discredit_influence_gain_factor
		: 0.0f;
	float neighbor_factor =
			bool(state.world.get_nation_adjacency_by_nation_adjacency_pair(n, target))
		? state.defines.neighbour_bonus_influence_percent
		: 0.0f;
	float sphere_neighbor_factor =
		has_sphere_neighbor
		? state.defines.sphere_neighbour_bonus_influence_percent
		: 0.0f;
	float continent_factor =
		n.get_capital().get_continent() != state.world.nation_get_capital(target).get_continent()
		? state.defines.other_continent_bonus_influence_percent
		: 0.0f;
	float puppet_factor =
		fatten(state.world, state.world.nation_get_overlord_as_subject(target)).get_ruler() == n
		? state.defines.puppet_bonus_influence_percent
		: 0.0f;
	float relationship_factor =
		state.world.diplomatic_relation_get_value(state.world.get_diplomatic_relation_by_diplomatic_pair(n, target)) / state.defines.relation_influence_modifier;

	float investment_factor =
		total_fi > 0.0f
		? state.defines.investment_influence_defense * gp_invest / total_fi
		: 0.0f;
	float pop_factor =
			state.world.nation_get_demographics(target, demographics::total) > state.defines.large_population_limit
		? state.defines.large_population_influence_penalty * state.world.nation_get_demographics(target, demographics::total) / state.defines.large_population_influence_penalty_chunk
		: 0.0f;
	float score_factor =
		gp_score > 0.0f
		? std::max(1.0f - (state.world.nation_get_industrial_score(target) + state.world.nation_get_military_score(target) + nations::prestige_score(state, target)) / gp_score, 0.0f)
		: 0.0f;

	float total_multiplier = 1.0f + discredit_factor + neighbor_factor + sphere_neighbor_factor + continent_factor + puppet_factor + relationship_factor + investment_factor + pop_factor + score_factor;

	auto gain_amount = std::max(0.0f, base_shares * total_multiplier);

	text::add_line(state, contents, "remove_diplomacy_dailyinflulence_gain", text::variable_type::num, text::fp_two_places{gain_amount}, text::variable_type::country, target);
	text::add_line_break_to_layout(state, contents);

	text::add_line(state, contents, "influence_explain_4", text::variable_type::x, text::fp_two_places{total_gain});
	text::add_line(state, contents, "influence_explain_5", text::variable_type::x, text::fp_two_places{state.defines.base_greatpower_daily_influence}, text::variable_type::y, text::fp_percentage{1.0f + n.get_modifier_values(sys::national_mod_offsets::influence_modifier)}, text::variable_type::val, text::fp_percentage{1.0f + n.get_modifier_values(sys::national_mod_offsets::influence)});

	text::add_line(state, contents, "influence_explain_6", text::variable_type::x, text::fp_percentage{1.0f + n.get_modifier_values(sys::national_mod_offsets::influence_modifier)});
	active_modifiers_description(state, contents, n, 15, sys::national_mod_offsets::influence_modifier, false);

	text::add_line(state, contents, "influence_explain_7", text::variable_type::x, text::fp_percentage{1.0f + n.get_modifier_values(sys::national_mod_offsets::influence)});
	active_modifiers_description(state, contents, n, 15, sys::national_mod_offsets::influence, false);

	text::add_line_break_to_layout(state, contents);

	text::add_line(state, contents, "influence_explain_8", text::variable_type::x, text::fp_two_places{base_shares});

	if(discredit_factor != 0 || neighbor_factor != 0 || sphere_neighbor_factor != 0 || continent_factor != 0 || puppet_factor != 0 || relationship_factor != 0 || investment_factor != 0 ||  pop_factor != 0 || score_factor != 0) {

		text::add_line(state, contents, "influence_explain_9");

		if(discredit_factor != 0) {
			text::add_line(state, contents, "influence_explain_10", text::variable_type::x, text::fp_two_places{discredit_factor}, 15);
		}
		if(neighbor_factor != 0) {
			text::add_line(state, contents, "influence_explain_11", text::variable_type::x, text::fp_two_places{neighbor_factor}, 15);
		}
		if(sphere_neighbor_factor != 0) {
			text::add_line(state, contents, "influence_explain_12", text::variable_type::x, text::fp_two_places{sphere_neighbor_factor}, 15);
		}
		if(continent_factor != 0) {
			text::add_line(state, contents, "influence_explain_13", text::variable_type::x, text::fp_two_places{continent_factor}, 15);
		}
		if(puppet_factor != 0) {
			text::add_line(state, contents, "influence_explain_14", text::variable_type::x, text::fp_two_places{puppet_factor}, 15);
		}
		if(relationship_factor != 0) {
			text::add_line(state, contents, "influence_explain_15", text::variable_type::x, text::fp_two_places{relationship_factor}, 15);
		}
		if(investment_factor != 0) {
			text::add_line(state, contents, "influence_explain_16", text::variable_type::x, text::fp_two_places{investment_factor}, 15);
		}
		if(pop_factor != 0) {
			text::add_line(state, contents, "influence_explain_17", text::variable_type::x, text::fp_two_places{pop_factor}, 15);
		}
		if(score_factor != 0) {
			text::add_line(state, contents, "influence_explain_18", text::variable_type::x, text::fp_two_places{score_factor}, 15);
		}
	}
}

class diplomacy_priority_button : public right_click_button_element_base {
	static std::string_view get_prio_key(uint8_t flags) {
		switch(flags & nations::influence::priority_mask) {
		case nations::influence::priority_zero:
			return "diplomacy_prio_none";
		case nations::influence::priority_one:
			return "diplomacy_prio_low";
		case nations::influence::priority_two:
			return "diplomacy_prio_middle";
		case nations::influence::priority_three:
			return "diplomacy_prio_high";
		}
		return "diplomacy_prio_none";
	}

public:
	void on_update(sys::state& state) noexcept override {
		if(parent) {
			Cyto::Any payload = dcon::nation_id{};
			parent->impl_get(state, payload);
			auto nation_id = any_cast<dcon::nation_id>(payload);

			auto rel = state.world.get_gp_relationship_by_gp_influence_pair(nation_id, state.local_player_nation);
			uint8_t rel_flags = bool(rel) ? state.world.gp_relationship_get_status(rel) : 0;
			switch(rel_flags & nations::influence::priority_mask) {
			case nations::influence::priority_zero:
				frame = 0;
				disabled = !command::can_change_influence_priority(state, state.local_player_nation, nation_id, 1);
				break;
			case nations::influence::priority_one:
				frame = 1;
				disabled = !command::can_change_influence_priority(state, state.local_player_nation, nation_id, 2);
				break;
			case nations::influence::priority_two:
				frame = 2;
				disabled = !command::can_change_influence_priority(state, state.local_player_nation, nation_id, 3);
				break;
			case nations::influence::priority_three:
				frame = 3;
				disabled = !command::can_change_influence_priority(state, state.local_player_nation, nation_id, 0);
				break;
			}
		}
	}

	void button_action(sys::state& state) noexcept override {
		if(parent) {
			Cyto::Any payload = dcon::nation_id{};
			parent->impl_get(state, payload);
			auto nation_id = any_cast<dcon::nation_id>(payload);

			auto rel = state.world.get_gp_relationship_by_gp_influence_pair(nation_id, state.local_player_nation);
			uint8_t rel_flags = bool(rel) ? state.world.gp_relationship_get_status(rel) : 0;
			switch(rel_flags & nations::influence::priority_mask) {
			case nations::influence::priority_zero:
				command::change_influence_priority(state, state.local_player_nation, nation_id, 1);
				break;
			case nations::influence::priority_one:
				command::change_influence_priority(state, state.local_player_nation, nation_id, 2);
				break;
			case nations::influence::priority_two:
				command::change_influence_priority(state, state.local_player_nation, nation_id, 3);
				break;
			case nations::influence::priority_three:
				command::change_influence_priority(state, state.local_player_nation, nation_id, 0);
				break;
			}
		}
	}

	void button_right_action(sys::state& state) noexcept override {
		if(parent) {
			Cyto::Any payload = dcon::nation_id{};
			parent->impl_get(state, payload);
			auto nation_id = any_cast<dcon::nation_id>(payload);

			auto rel = state.world.get_gp_relationship_by_gp_influence_pair(nation_id, state.local_player_nation);
			uint8_t rel_flags = bool(rel) ? state.world.gp_relationship_get_status(rel) : 0;
			switch(rel_flags & nations::influence::priority_mask) {
			case nations::influence::priority_zero:
				command::change_influence_priority(state, state.local_player_nation, nation_id, 4);
				break;
			case nations::influence::priority_one:
				command::change_influence_priority(state, state.local_player_nation, nation_id, 0);
				break;
			case nations::influence::priority_two:
				command::change_influence_priority(state, state.local_player_nation, nation_id, 1);
				break;
			case nations::influence::priority_three:
				command::change_influence_priority(state, state.local_player_nation, nation_id, 2);
				break;
			}
		}
	}

	tooltip_behavior has_tooltip(sys::state& state) noexcept override {
		return tooltip_behavior::variable_tooltip;
	}

	void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
		if(parent) {
			Cyto::Any payload = dcon::nation_id{};
			parent->impl_get(state, payload);
			auto nation_id = any_cast<dcon::nation_id>(payload);

			
			if(!nations::is_great_power(state, state.local_player_nation)) {
				text::add_line(state, contents, "diplomacy_cannot_set_prio");
			} else if(nations::is_great_power(state, nation_id)) {
				text::add_line(state, contents, "diplomacy_cannot_set_prio_gp");
			} else {
				if(state.user_settings.use_new_ui) {
					explain_influence(state, nation_id, contents);
				} else {
					auto rel = state.world.get_gp_relationship_by_gp_influence_pair(nation_id, state.local_player_nation);
					uint8_t rel_flags = bool(rel) ? state.world.gp_relationship_get_status(rel) : 0;

					// DIPLOMACY_SET_PRIO;Current influence priority is §Y$VALUE$§W.;
					// DIPLOMACY_DAILYINFLULENCE_GAIN;We gain §Y$NUM$§W influence in §Y$COUNTRY$§W each day. Our base influence gain is §Y$BASE$§W

					auto box = text::open_layout_box(contents, 0);
					text::substitution_map sub{};
					if(auto k = state.key_to_text_sequence.find(get_prio_key(rel_flags)); k != state.key_to_text_sequence.end()) {
						text::add_to_substitution_map(sub, text::variable_type::value, k->second);
					}
					text::localised_format_box(state, contents, box, std::string_view("diplomacy_set_prio"), sub);
					text::add_line_break_to_layout_box(state, contents, box);
					text::add_to_substitution_map(sub, text::variable_type::country, nation_id);
					text::localised_format_box(state, contents, box, std::string_view("diplomacy_dailyinflulence_gain"));
					text::close_layout_box(contents, box);
				}
			}

			if(!state.user_settings.use_new_ui) {
				auto box = text::open_layout_box(contents, 0);
				text::add_divider_to_layout_box(state, contents, box);
				text::localised_format_box(state, contents, box, std::string_view("diplomacy_set_prio_desc"));
				text::close_layout_box(contents, box);
				active_modifiers_description(state, contents, state.local_player_nation, 0, sys::national_mod_offsets::influence, false);
			}
		}
	}
};

class diplomacy_country_info : public listbox_row_element_base<dcon::nation_id> {
public:
	void on_create(sys::state& state) noexcept override {
		listbox_row_element_base<dcon::nation_id>::on_create(state);
		base_data.position.x -= 14;
		base_data.position.y -= 524;
	}

	std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override {
		if(name == "country_select") {
			return make_element_by_type<diplomacy_country_select>(state, id);
		} else if(name == "country_flag") {
			auto ptr = make_element_by_type<flag_button>(state, id);
			ptr->base_data.position.y -= 2; // Nudge
			return ptr;
		} else if(name == "country_name") {
			return make_element_by_type<generic_name_text<dcon::nation_id>>(state, id);
		} else if(name == "country_prio") {
			return make_element_by_type<diplomacy_priority_button>(state, id);
		} else if(name == "country_boss_flag") {
			return make_element_by_type<nation_overlord_flag>(state, id);
		} else if(name == "country_prestige") {
			return make_element_by_type<nation_prestige_rank_text>(state, id);
		} else if(name == "country_economic") {
			return make_element_by_type<nation_industry_rank_text>(state, id);
		} else if(name == "country_military") {
			return make_element_by_type<nation_military_rank_text>(state, id);
		} else if(name == "country_total") {
			return make_element_by_type<nation_rank_text>(state, id);
		} else if(name == "country_opinion") {
			return make_element_by_type<nation_player_opinion_text>(state, id);
		} else if(name == "country_relation") {
			return make_element_by_type<nation_player_relations_text>(state, id);
		} else if(name.substr(0, 10) == "country_gp") {
			auto ptr = make_element_by_type<nation_gp_opinion_text>(state, id);
			ptr->rank = uint16_t(std::stoi(std::string{name.substr(10)}));
			return ptr;
		} else {
			return nullptr;
		}
	}
};

class diplomacy_country_listbox : public listbox_element_base<diplomacy_country_info, dcon::nation_id> {
protected:
	std::string_view get_row_element_name() override {
		return "diplomacy_country_info";
	}
};

class cb_icon : public image_element_base {
public:
	void on_update(sys::state& state) noexcept override {
		auto cb = retrieve<military::available_cb>(state, parent);
		dcon::cb_type_id content = cb.cb_type;
		frame = state.world.cb_type_get_sprite_index(content) - 1;
	}

	tooltip_behavior has_tooltip(sys::state& state) noexcept override {
		return tooltip_behavior::variable_tooltip;
	}

	void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
		auto cb = retrieve<military::available_cb>(state, parent);
		text::add_line(state, contents, state.world.cb_type_get_name(cb.cb_type));
		if(cb.expiration) {
			text::add_line(state, contents, "until_date", text::variable_type::x, cb.expiration);
		}
	}
};

class overlapping_cb_icon : public listbox_row_element_base<military::available_cb> {
public:
	std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override {
		if(name == "wargoal_icon") {
			return make_element_by_type<cb_icon>(state, id);
		} else {
			return nullptr;
		}
	}
};

class overlapping_wargoals : public overlapping_listbox_element_base<overlapping_cb_icon, military::available_cb> {
protected:
	std::string_view get_row_element_name() override {
		return "wargoal";
	}

public:
	void on_update(sys::state& state) noexcept override {
		row_contents.clear();

		auto content = retrieve<dcon::nation_id>(state, parent);
		if(!content || content == state.local_player_nation)
			return;

		auto one_cbs = state.world.nation_get_available_cbs(state.local_player_nation);
		for(auto& cb : one_cbs)
			if(cb.target == content)
				row_contents.push_back(cb);

		for(auto cb : state.world.in_cb_type) {
			if((cb.get_type_bits() & military::cb_flag::always) != 0) {
				if(military::cb_conditions_satisfied(state, state.local_player_nation, content, cb))
					row_contents.push_back(military::available_cb{content, sys::date{}, cb});
			}
		}
		update(state);
		
	}
};

class diplomacy_action_add_wargoal_button : public button_element_base {
public:
	void on_update(sys::state& state) noexcept override {
		auto content = retrieve<dcon::nation_id>(state, parent);
		if(!content)
			return;

		disabled = true;

		if(content == state.local_player_nation) {
			return;
		}

		if(state.world.nation_get_diplomatic_points(state.local_player_nation) < state.defines.addwargoal_diplomatic_cost) {
			return;
		}

		auto w = military::find_war_between(state, state.local_player_nation, content);
		if(!w) {
			return;
		}

		for(auto cb_type : state.world.in_cb_type) {
			if((state.world.cb_type_get_type_bits(cb_type) & military::cb_flag::always) == 0) {
				bool cb_fabbed = false;
				for(auto& fab_cb : state.world.nation_get_available_cbs(state.local_player_nation)) {
					if(fab_cb.cb_type == cb_type && fab_cb.target == content) {
						cb_fabbed = true;
						break;
					}
				}
				if(!cb_fabbed) {
					if((state.world.cb_type_get_type_bits(cb_type) & military::cb_flag::is_not_constructing_cb) == 0)
						continue; // can only add a constructable cb this way

					if(state.world.war_get_is_great(w)) {
						if(state.world.nation_get_demographics(state.local_player_nation, demographics::to_key(state, state.culture_definitions.jingoism)) >=
								state.defines.wargoal_jingoism_requirement * state.defines.gw_wargoal_jingoism_requirement_mod) {
							disabled = false;
							return;
						}
					} else {
						if(state.world.nation_get_demographics(state.local_player_nation, demographics::to_key(state, state.culture_definitions.jingoism)) >= state.defines.wargoal_jingoism_requirement) {
							disabled = false;
							return;
						}
					}
				} else {
					disabled = false;
					return;
				}
			} else { // this is an always CB
				// prevent duplicate war goals
				if(military::can_add_always_cb_to_war(state, state.local_player_nation, content, cb_type, w)) {
					disabled = false;
					return;
				}
			}
		}
	}

	void button_action(sys::state& state) noexcept override {
		if(parent) {
			Cyto::Any payload = diplomacy_action::add_wargoal;
			parent->impl_get(state, payload);
		}
	}

	tooltip_behavior has_tooltip(sys::state& state) noexcept override {
		return tooltip_behavior::variable_tooltip;
	}

	void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
		if(state.user_settings.use_new_ui) {
			auto content = retrieve<dcon::nation_id>(state, parent);
			if(content == state.local_player_nation) {
				text::add_line(state, contents, "add_wg_1");
				return;
			}

			if(state.world.nation_get_diplomatic_points(state.local_player_nation) < state.defines.addwargoal_diplomatic_cost) {
				text::add_line(state, contents, "add_wg_3", text::variable_type::x, int64_t(state.defines.addwargoal_diplomatic_cost));
				return;
			}

			auto w = military::find_war_between(state, state.local_player_nation, content);
			if(!w) {
				text::add_line(state, contents, "add_wg_2");
				return;
			}

			for(auto cb_type : state.world.in_cb_type) {
				if((state.world.cb_type_get_type_bits(cb_type) & military::cb_flag::always) == 0) {
					for(auto& fab_cb : state.world.nation_get_available_cbs(state.local_player_nation)) {
						if(fab_cb.cb_type == cb_type && fab_cb.target == content) {
							return;
						}
					}
				} else { // this is an always CB
					// prevent duplicate war goals
					if(military::can_add_always_cb_to_war(state, state.local_player_nation, content, cb_type, w)) {
						return;
					}
				}
			}

			// if we hit this, it means no existing cb is ready to be applied
			if(state.world.war_get_is_great(w)) {
				if(state.world.nation_get_demographics(state.local_player_nation, demographics::to_key(state, state.culture_definitions.jingoism)) <
						state.defines.wargoal_jingoism_requirement * state.defines.gw_wargoal_jingoism_requirement_mod) {

					text::add_line(state, contents, "add_wg_4", text::variable_type::x, text::fp_percentage_one_place{state.world.nation_get_demographics(state.local_player_nation, demographics::to_key(state, state.culture_definitions.jingoism))}, text::variable_type::y, text::fp_percentage_one_place{state.defines.wargoal_jingoism_requirement * state.defines.gw_wargoal_jingoism_requirement_mod});

					return;
				}
			} else {
				if(state.world.nation_get_demographics(state.local_player_nation, demographics::to_key(state, state.culture_definitions.jingoism)) < state.defines.wargoal_jingoism_requirement) {

					text::add_line(state, contents, "add_wg_4", text::variable_type::x, text::fp_percentage_one_place{state.world.nation_get_demographics(state.local_player_nation, demographics::to_key(state, state.culture_definitions.jingoism))}, text::variable_type::y, text::fp_percentage_one_place{state.defines.wargoal_jingoism_requirement});

					return;
				}
			}
		} else {
			if(parent) {
				auto content = retrieve<dcon::nation_id>(state, parent);

				auto box = text::open_layout_box(contents, 0);
				text::localised_format_box(state, contents, box, std::string_view("act_wardesc"));
				text::add_divider_to_layout_box(state, contents, box);
				if(content == state.local_player_nation) {
					text::localised_format_box(state, contents, box, std::string_view("act_no_self"));
				} else {
					text::substitution_map dp_map{};
					text::add_to_substitution_map(dp_map, text::variable_type::current,
							text::fp_two_places{state.world.nation_get_diplomatic_points(state.local_player_nation)});
					text::add_to_substitution_map(dp_map, text::variable_type::needed,
							text::fp_two_places{state.defines.addwargoal_diplomatic_cost});
					text::localised_format_box(state, contents, box,
							std::string_view(
									state.world.nation_get_diplomatic_points(state.local_player_nation) >= state.defines.addwargoal_diplomatic_cost
											? "dip_enough_diplo"
											: "dip_no_diplo"),
							dp_map);
				}
				text::close_layout_box(contents, box);
			}
		}
	}
};

class primary_culture : public simple_text_element_base {
public:
	void on_update(sys::state& state) noexcept override {
		auto pc = state.world.nation_get_primary_culture(retrieve<dcon::nation_id>(state, parent));
		set_text(state, text::produce_simple_string(state, pc.get_name()));
	}
};

class accepted_cultures : public simple_text_element_base {
	void on_update(sys::state& state) noexcept override {
		auto ac = state.world.nation_get_accepted_cultures(retrieve<dcon::nation_id>(state, parent));

		std::string t;
		if(ac.size() > 0) {
			t += text::produce_simple_string(state, state.world.culture_get_name(ac[0]));
		}
		for(uint32_t i = 1; i < ac.size(); ++i) {
			t += ", " ;
			t += text::produce_simple_string(state, state.world.culture_get_name(ac[i]));
		}

		set_text(state, text::produce_simple_string(state, t));
	}
};

class diplomacy_country_facts : public window_element_base {
private:
	dcon::nation_id active_nation{};
	flag_button* country_flag = nullptr;
	nation_player_relations_text* country_relation = nullptr;
	image_element_base* country_relation_icon = nullptr;

public:
	std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override {
		if(name == "country_flag") {
			return make_element_by_type<flag_button>(state, id);
		} else if(name == "country_flag_overlay") {
			return make_element_by_type<nation_flag_frame>(state, id);
		} else if(name == "country_name") {
			return make_element_by_type<generic_name_text<dcon::nation_id>>(state, id);
		} else if(name == "country_status") {
			return make_element_by_type<nation_status_text>(state, id);
		} else if(name == "selected_nation_totalrank") {
			return make_element_by_type<nation_rank_text>(state, id);
		} else if(name == "ideology_icon") {
			return make_element_by_type<nation_ruling_party_ideology_plupp>(state, id);
		} else if(name == "country_party") {
			return make_element_by_type<nation_ruling_party_text>(state, id);
		} else if(name == "country_gov") {
			return make_element_by_type<nation_government_type_text>(state, id);
		} else if(name == "country_tech") {
			return make_element_by_type<national_tech_school>(state, id);
		} else if(name == "our_relation_icon") {
			auto ptr = make_element_by_type<image_element_base>(state, id);
			country_relation_icon = ptr.get();
			return ptr;
		} else if(name == "our_relation") {
			auto ptr = make_element_by_type<nation_player_relations_text>(state, id);
			country_relation = ptr.get();
			return ptr;
		} else if(name == "country_prestige") {
			return make_element_by_type<nation_prestige_text>(state, id);
		} else if(name == "selected_prestige_rank") {
			return make_element_by_type<nation_prestige_rank_text>(state, id);
		} else if(name == "country_economic") {
			return make_element_by_type<nation_industry_score_text>(state, id);
		} else if(name == "selected_industry_rank") {
			return make_element_by_type<nation_industry_rank_text>(state, id);
		} else if(name == "country_military") {
			return make_element_by_type<nation_military_score_text>(state, id);
		} else if(name == "selected_military_rank") {
			return make_element_by_type<nation_military_rank_text>(state, id);
		} else if(name == "country_total") {
			return make_element_by_type<nation_total_score_text>(state, id);
		} else if(name == "selected_total_rank") {
			return make_element_by_type<nation_rank_text>(state, id);
		} else if(name == "country_population") {
			return make_element_by_type<nation_population_text>(state, id);
		} else if(name == "country_primary_cultures") {
			return make_element_by_type<primary_culture>(state, id);
		} else if(name == "country_accepted_cultures") {
			return make_element_by_type<accepted_cultures>(state, id);
		} else if(name == "country_wars") {
			auto ptr = make_element_by_type<overlapping_enemy_flags>(state, id);
			ptr->base_data.position.y -= 8 - 1; // Nudge
			return ptr;
		} else if(name == "country_allies") {
			auto ptr = make_element_by_type<overlapping_ally_flags>(state, id);
			ptr->base_data.position.y -= 8 - 1; // Nudge
			return ptr;
		} else if(name == "country_protected") {
			auto ptr = make_element_by_type<overlapping_sphere_flags>(state, id);
			ptr->base_data.position.y -= 8 - 1; // Nudge
			return ptr;
		} else if(name == "country_truce") {
			auto ptr = make_element_by_type<overlapping_truce_flags>(state, id);
			ptr->base_data.position.y -= 8 - 1; // Nudge
			return ptr;
		} else if(name == "country_cb") {
			return make_element_by_type<overlapping_wargoals>(state, id);
		} else if(name == "infamy_text") {
			return make_element_by_type<nation_infamy_text>(state, id);
		} else if(name == "warexhastion_text") {
			return make_element_by_type<diplomacy_war_exhaustion>(state, id);
		} else if(name == "brigade_text") {
			return make_element_by_type<diplomacy_nation_navies_text>(state, id);
		} else if(name == "ships_text") {
			return make_element_by_type<diplomacy_nation_armies_text>(state, id);
		} else if(name == "add_wargoal") {
			return make_element_by_type<diplomacy_action_add_wargoal_button>(state, id);
		} else {
			return nullptr;
		}
	}

	void on_update(sys::state& state) noexcept override {
		if(parent) {
			Cyto::Any payload = dcon::nation_id{};
			parent->impl_get(state, payload);
			auto content = any_cast<dcon::nation_id>(payload);
			auto fat_id = dcon::fatten(state.world, content);
			country_relation->set_visible(state, content != state.local_player_nation);
			country_relation_icon->set_visible(state, content != state.local_player_nation);
		}
	}
};

class overlapping_attacker_flags : public overlapping_flags_box {
protected:
	void populate_flags(sys::state& state) override {
		row_contents.clear();
		if(parent) {
			Cyto::Any payload = dcon::war_id{};
			parent->impl_get(state, payload);
			dcon::war_id w = any_cast<dcon::war_id>(payload);
			auto war = dcon::fatten(state.world, w);
			for(auto o : war.get_war_participant())
				if(o.get_is_attacker() == true)
					row_contents.push_back(o.get_nation().get_identity_from_identity_holder().id);
		}
		update(state);
	}
};
class overlapping_defender_flags : public overlapping_flags_box {
protected:
	void populate_flags(sys::state& state) override {
		row_contents.clear();
		if(parent) {
			Cyto::Any payload = dcon::war_id{};
			parent->impl_get(state, payload);
			dcon::war_id w = any_cast<dcon::war_id>(payload);
			auto war = dcon::fatten(state.world, w);
			for(auto o : war.get_war_participant())
				if(o.get_is_attacker() == false)
					row_contents.push_back(o.get_nation().get_identity_from_identity_holder().id);
		}
		update(state);
	}
};

template<bool IsAttacker>
class war_side_strength_text : public button_element_base {
public:
	void on_update(sys::state& state) noexcept override {
		if(parent) {
			Cyto::Any payload = dcon::war_id{};
			parent->impl_get(state, payload);
			dcon::war_id content = any_cast<dcon::war_id>(payload);

			auto fat_id = dcon::fatten(state.world, content);
			int32_t strength = 0;
			for(auto o : fat_id.get_war_participant())
				if(o.get_is_attacker() == IsAttacker)
					strength += int32_t(o.get_nation().get_military_score());
			set_button_text(state, std::to_string(strength));
		}
	}

	tooltip_behavior has_tooltip(sys::state& state) noexcept override {
		return tooltip_behavior::variable_tooltip;
	}

	void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
		if(parent) {
			Cyto::Any payload = dcon::war_id{};
			parent->impl_get(state, payload);
			dcon::war_id content = any_cast<dcon::war_id>(payload);

			auto fat_id = dcon::fatten(state.world, content);
			for(auto o : fat_id.get_war_participant())
				if(o.get_is_attacker() == IsAttacker) {
					auto name = o.get_nation().get_name();
					auto box = text::open_layout_box(contents, 0);
					text::add_to_layout_box(state, contents, box, text::produce_simple_string(state, name), text::text_color::yellow);
					text::add_to_layout_box(state, contents, box, std::string{":"}, text::text_color::yellow);
					text::add_space_to_layout_box(state, contents, box);
					auto strength = int32_t(o.get_nation().get_military_score());
					text::add_to_layout_box(state, contents, box, std::to_string(strength), text::text_color::white);
					text::close_layout_box(contents, box);
				}
		}
	}
};

template<bool B>
class diplomacy_join_war_button : public button_element_base {
public:
	void on_create(sys::state& state) noexcept override {
		button_element_base::on_create(state);
		set_button_text(state, "");
	}

	void on_update(sys::state& state) noexcept override {
		if(parent) {
			dcon::war_id war_id = retrieve<dcon::war_id>(state, parent);

			disabled = !command::can_intervene_in_war(state, state.local_player_nation, war_id, B);
		}
	}

	void button_action(sys::state& state) noexcept override {
		if(parent) {
			dcon::war_id war_id = retrieve<dcon::war_id>(state, parent);

			command::intervene_in_war(state, state.local_player_nation, war_id, B);
		}
	}

	tooltip_behavior has_tooltip(sys::state& state) noexcept override {
		return tooltip_behavior::variable_tooltip;
	}

	void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
		if(state.user_settings.use_new_ui) {
			dcon::nation_id nation_id = retrieve<dcon::nation_id>(state, parent);
			dcon::war_id w = retrieve<dcon::war_id>(state, parent);

			if(!state.world.war_get_is_great(w)) {
				text::add_line(state, contents, "intervene_1");
			} else {
				text::add_line(state, contents, "intervene_2");
			}
			text::add_line_break_to_layout(state, contents);

			text::add_line_with_condition(state, contents, "intervene_3", nations::is_great_power(state, state.local_player_nation));
			text::add_line_with_condition(state, contents, "intervene_4", !nations::is_involved_in_crisis(state, state.local_player_nation));
			if(state.defines.min_months_to_intervene > 0) {
				text::add_line_with_condition(state, contents, "intervene_5", state.current_date >= state.world.war_get_start_date(w) + int32_t(30.0f * state.defines.min_months_to_intervene), text::variable_type::x, int64_t(state.defines.min_months_to_intervene));
			}
			text::add_line_with_condition(state, contents, "intervene_6", military::joining_war_does_not_violate_constraints(state, state.local_player_nation, w, B));

			if(!state.world.war_get_is_great(w)) {
				auto defender = state.world.war_get_primary_defender(w);
				auto rel_w_defender = state.world.get_gp_relationship_by_gp_influence_pair(defender, state.local_player_nation);
				auto inf = state.world.gp_relationship_get_status(rel_w_defender) & nations::influence::level_mask;

				text::add_line_with_condition(state, contents, "intervene_17", inf == nations::influence::level_friendly);

				text::add_line_with_condition(state, contents, "intervene_7", !state.world.war_get_is_crisis_war(w));
				text::add_line_with_condition(state, contents, "intervene_9", !B);
				text::add_line_with_condition(state, contents, "intervene_10", !military::defenders_have_non_status_quo_wargoal(state, w));
				text::add_line_with_condition(state, contents, "intervene_11", military::primary_warscore(state, w) >= -state.defines.min_warscore_to_intervene, text::variable_type::x, int64_t(-state.defines.min_warscore_to_intervene));

			} else {
				if constexpr(B) {
					text::add_line_with_condition(state, contents, "intervene_8", !military::joining_as_attacker_would_break_truce(state, state.local_player_nation, w));
				}

				text::add_line_with_condition(state, contents, "intervene_12", state.world.nation_get_war_exhaustion(state.local_player_nation) < state.defines.gw_intervene_max_exhaustion, text::variable_type::x, int64_t(state.defines.gw_intervene_max_exhaustion));

				auto primary_on_side = B ? state.world.war_get_primary_attacker(w) : state.world.war_get_primary_defender(w);
				auto rel = state.world.get_diplomatic_relation_by_diplomatic_pair(primary_on_side, state.local_player_nation);

				text::add_line_with_condition(state, contents, "intervene_13", state.world.diplomatic_relation_get_value(rel) >= state.defines.gw_intervene_min_relations, text::variable_type::x, int64_t(state.defines.gw_intervene_min_relations));

				bool any_in_sphere = false;
				bool any_allied = false;
				bool any_armies = false;

				for(auto p : state.world.war_get_war_participant(w)) {
					if(p.get_is_attacker() != B) { // scan nations on other side
						if(p.get_nation().get_in_sphere_of() == state.local_player_nation)
							any_in_sphere = true;

						auto irel = state.world.get_diplomatic_relation_by_diplomatic_pair(p.get_nation(), state.local_player_nation);
						if(state.world.diplomatic_relation_get_are_allied(irel))
							any_allied = true;

						for(auto prov : p.get_nation().get_province_ownership()) {
							for(auto arm : prov.get_province().get_army_location()) {
								if(arm.get_army().get_controller_from_army_control() == state.local_player_nation)
									any_armies = true;
							}
						}
					}
				}
				text::add_line_with_condition(state, contents, "intervene_14", !any_in_sphere);
				text::add_line_with_condition(state, contents, "intervene_15", !any_allied);
				text::add_line_with_condition(state, contents, "intervene_16", !any_armies);
			}

		}
	}
};

class wargoal_icon : public image_element_base {
public:
	void on_update(sys::state& state) noexcept override {
		auto wg = retrieve<dcon::wargoal_id>(state, parent);
		frame = state.world.cb_type_get_sprite_index(state.world.wargoal_get_type(wg)) - 1;
	}

	tooltip_behavior has_tooltip(sys::state& state) noexcept override {
		return tooltip_behavior::variable_tooltip;
	}

	void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
		auto wg = retrieve<dcon::wargoal_id>(state, parent);
		auto cb = state.world.wargoal_get_type(wg);
		text::add_line(state, contents, state.world.cb_type_get_name(cb));

		text::add_line_break_to_layout(state, contents);
		text::add_line(state, contents, "war_goal_1", text::variable_type::x, state.world.wargoal_get_added_by(wg));
		text::add_line(state, contents, "war_goal_2", text::variable_type::x, state.world.wargoal_get_target_nation(wg));
		if(state.world.wargoal_get_associated_state(wg)) {
			text::add_line(state, contents, "war_goal_3", text::variable_type::x, state.world.wargoal_get_associated_state(wg));
		}
		if(state.world.wargoal_get_associated_tag(wg)) {
			text::add_line(state, contents, "war_goal_4", text::variable_type::x, state.world.wargoal_get_associated_tag(wg));
		} else if(state.world.wargoal_get_secondary_nation(wg)) {
			text::add_line(state, contents, "war_goal_4", text::variable_type::x, state.world.wargoal_get_secondary_nation(wg));
		}
		if(state.world.wargoal_get_ticking_war_score(wg) != 0) {
			text::add_line(state, contents, "war_goal_5", text::variable_type::x, text::fp_one_place{state.world.wargoal_get_ticking_war_score(wg)});
		}
	}
};

class overlapping_wargoal_icon : public listbox_row_element_base<dcon::wargoal_id> {
public:
	std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override {
		if(name == "wargoal_icon") {
			return make_element_by_type<wargoal_icon>(state, id);
		} else {
			return nullptr;
		}
	}
};

template<bool B>
class diplomacy_war_overlapping_wargoals : public overlapping_listbox_element_base<overlapping_wargoal_icon, dcon::wargoal_id> {
protected:
	std::string_view get_row_element_name() override {
		return "wargoal";
	}

public:
	void on_update(sys::state& state) noexcept override {
		row_contents.clear();

		dcon::war_id content = retrieve<dcon::war_id>(state, parent);
		for(auto wg : state.world.war_get_wargoals_attached(content)) {
			if(military::is_attacker(state, content, wg.get_wargoal().get_added_by()) == B)
				row_contents.push_back(wg.get_wargoal().id);
		}

		update(state);
	}
};

class war_name_text : public generic_multiline_text<dcon::war_id> {
	void populate_layout(sys::state& state, text::endless_layout& contents, dcon::war_id id) noexcept override {
		contents.fixed_parameters.suppress_hyperlinks = true;

		auto war = dcon::fatten(state.world, id);
		dcon::nation_id primary_attacker = state.world.war_get_primary_attacker(war);
		dcon::nation_id primary_defender = state.world.war_get_primary_defender(war);

		for(auto wg : state.world.war_get_wargoals_attached(war)) {
			if(wg.get_wargoal().get_added_by() == primary_attacker && wg.get_wargoal().get_target_nation() == primary_defender) {
				auto box = text::open_layout_box(contents);
				text::substitution_map sub{};
				auto pa_adj = state.world.nation_get_adjective(primary_attacker);
				text::add_to_substitution_map(sub, text::variable_type::first, pa_adj);
				auto sdef = wg.get_wargoal().get_associated_state();
				auto bits = state.world.cb_type_get_type_bits(wg.get_wargoal().get_type());
				if(dcon::fatten(state.world, sdef).is_valid()) {
					text::add_to_substitution_map(sub, text::variable_type::second, sdef);
				} else if((bits & (military::cb_flag::po_annex | military::cb_flag::po_make_puppet | military::cb_flag::po_gunboat)) !=
									0) {
					text::add_to_substitution_map(sub, text::variable_type::second, primary_defender);
				} else if((bits & (military::cb_flag::po_transfer_provinces)) != 0) {
					auto niid = wg.get_wargoal().get_associated_tag();
					auto adj = state.world.national_identity_get_adjective(niid);
					text::add_to_substitution_map(sub, text::variable_type::second, adj);
				} else {
					auto adj = state.world.nation_get_adjective(primary_defender);
					text::add_to_substitution_map(sub, text::variable_type::second, adj);
				}

				// TODO: ordinal numbering, 1st, 2nd, 3rd, 4th, etc...
				text::add_to_substitution_map(sub, text::variable_type::order, std::string_view(""));

				text::add_to_layout_box(state, contents, box, state.world.war_get_name(war), sub);

				text::close_layout_box(contents, box);
				break;
			}
		}
	}
};

class war_score_progress_bar : public progress_bar {
public:
	void on_update(sys::state& state) noexcept override {
		auto war = retrieve<dcon::war_id>(state, parent);
		if(war) {
			auto ws = military::primary_warscore(state, war);
			progress = ws / 200.0f + 0.5f;
		}
	}

	tooltip_behavior has_tooltip(sys::state& state) noexcept override {
		return tooltip_behavior::variable_tooltip;
	}

	void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
		auto war = retrieve<dcon::war_id>(state, parent);
		text::add_line(state, contents, "war_score_1", text::variable_type::x, text::fp_one_place{military::primary_warscore_from_occupation(state, war)});
		text::add_line(state, contents, "war_score_2", text::variable_type::x, text::fp_one_place{military::primary_warscore_from_battles(state, war)});
		text::add_line(state, contents, "war_score_3", text::variable_type::x, text::fp_one_place{military::primary_warscore_from_war_goals(state, war)});
	}
};

struct war_bar_position {
	ui::xy_pair bottom_left = ui::xy_pair{0,0};
	int16_t width = 0;
};

class attacker_peace_goal : public image_element_base {
public:
	void on_update(sys::state& state) noexcept override {
		auto bar_pos = retrieve<war_bar_position>(state, parent);
		auto war = retrieve<dcon::war_id>(state, parent);

		auto attacker_cost = std::min(military::attacker_peace_cost(state, war), 100);
		auto x_pos = int16_t((float(attacker_cost) / 200.0f + 0.5f) * float(bar_pos.width));

		base_data.position.x = int16_t(x_pos + bar_pos.bottom_left.x - base_data.size.x / 2);
		base_data.position.y = bar_pos.bottom_left.y;
	}
};

class defender_peace_goal : public image_element_base {
public:
	void on_update(sys::state& state) noexcept override {
		auto bar_pos = retrieve<war_bar_position>(state, parent);
		auto war = retrieve<dcon::war_id>(state, parent);

		auto defender_cost = std::min(military::defender_peace_cost(state, war), 100);
		auto x_pos = int16_t((float(-defender_cost) / 200.0f + 0.5f) * float(bar_pos.width));

		base_data.position.x = int16_t(x_pos + bar_pos.bottom_left.x - base_data.size.x / 2);
		base_data.position.y = bar_pos.bottom_left.y;
	}
};

class war_bg : public image_element_base {
	void on_update(sys::state& state) noexcept override {
		auto war = retrieve<dcon::war_id>(state, parent);
		if(state.world.war_get_is_great(war)) {
			frame = 2;
		} else if(state.world.war_get_is_crisis_war(war)) {
			frame = 1;
		} else {
			frame = 0;
		}
	}
};

class war_score_text : public simple_text_element_base {
public:
	void on_update(sys::state& state) noexcept override {
		auto war = retrieve<dcon::war_id>(state, parent);
		if(war) {
			auto ws = military::primary_warscore(state, war) / 100.0f;
			set_text(state, text::format_percentage(ws, 0));
		}
	}
};

class diplomacy_war_info : public listbox_row_element_base<dcon::war_id> {
public:
	war_bar_position bar_position;

	void on_create(sys::state& state) noexcept override {
		listbox_row_element_base::on_create(state);
		base_data.position.x = base_data.position.y = 0;
	}

	std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override {
		if(name == "diplo_war_entrybg") {
			return make_element_by_type<war_bg>(state, id);
		} else if(name == "war_name") {
			auto ptr = make_element_by_type<war_name_text>(state, id);
			//ptr->base_data.position.x += 90; // Nudge
			return ptr;
		} else if(name == "attackers_mil_strength") {
			auto ptr = make_element_by_type<war_side_strength_text<true>>(state, id);
			ptr->base_data.position.y -= 4; // Nudge
			return ptr;
		} else if(name == "defenders_mil_strength") {
			auto ptr = make_element_by_type<war_side_strength_text<false>>(state, id);
			ptr->base_data.position.y -= 4; // Nudge
			return ptr;
		} else if(name == "warscore") {
			auto ptr = make_element_by_type<war_score_progress_bar>(state, id);
			bar_position.width = ptr->base_data.size.x;
			bar_position.bottom_left = ui::xy_pair{ptr->base_data.position.x, int16_t(ptr->base_data.position.y + ptr->base_data.size.y)};
			return ptr;
		} if(name == "diplo_warscore_marker1") {
			return make_element_by_type<attacker_peace_goal>(state, id);
		} else if(name == "diplo_warscore_marker2") {
			return make_element_by_type<defender_peace_goal>(state, id);
		} else if(name == "warscore_text") {
			auto ptr = make_element_by_type<war_score_text>(state, id);
			ptr->base_data.position.y -= 2;
			return ptr;
		} else if(name == "attackers") {
			auto ptr = make_element_by_type<overlapping_attacker_flags>(state, id);
			ptr->base_data.position.y -= 8 - 2;
			return ptr;
		} else if(name == "defenders") {
			auto ptr = make_element_by_type<overlapping_defender_flags>(state, id);
			ptr->base_data.position.y -= 8 - 2;
			return ptr;
		} else if(name == "attackers_wargoals") {
			return make_element_by_type<diplomacy_war_overlapping_wargoals<true>>(state, id);
		} else if(name == "defenders_wargoals") {
			return make_element_by_type<diplomacy_war_overlapping_wargoals<false>>(state, id);
		} else if(name == "join_attackers") {
			return make_element_by_type<diplomacy_join_war_button<true>>(state, id);
		} else if(name == "join_defenders") {
			return make_element_by_type<diplomacy_join_war_button<false>>(state, id);
		} else {
			return nullptr;
		}
	}

	message_result get(sys::state& state, Cyto::Any& payload) noexcept override {
		if(payload.holds_type<war_bar_position>()) {
			payload.emplace<war_bar_position>(bar_position);
			return message_result::consumed;
		}
		return listbox_row_element_base<dcon::war_id>::get(state, payload);
	}
};

class justifying_cb_type_icon : public image_element_base {
public:
	void on_update(sys::state& state) noexcept override {
		if(parent) {
			Cyto::Any payload = dcon::nation_id{};
			parent->impl_get(state, payload);
			const dcon::nation_id content = any_cast<dcon::nation_id>(payload);
			auto fat = dcon::fatten(state.world, content);
			frame = fat.get_constructing_cb_type().get_sprite_index() - 1;
		}
	}
};

class justifying_cb_progress : public progress_bar {
public:
	void on_update(sys::state& state) noexcept override {
		if(parent) {
			Cyto::Any payload = dcon::nation_id{};
			parent->impl_get(state, payload);
			const dcon::nation_id content = any_cast<dcon::nation_id>(payload);
			auto fat = dcon::fatten(state.world, content);
			progress = (fat.get_constructing_cb_progress() / 100.0f);
		}
	}
};

class justifying_attacker_flag : public overlapping_flags_box {
protected:
	void populate_flags(sys::state& state) noexcept override {
		if(parent) {
			row_contents.clear();
			Cyto::Any payload = dcon::nation_id{};
			parent->impl_get(state, payload);
			const dcon::nation_id content = any_cast<dcon::nation_id>(payload);
			auto fat = dcon::fatten(state.world, content);
			row_contents.push_back(fat.get_identity_from_identity_holder().id);
			update(state);
		}
	}
};

class justifying_defender_flag : public overlapping_flags_box {
protected:
	void populate_flags(sys::state& state) noexcept override {
		if(parent) {
			row_contents.clear();
			Cyto::Any payload = dcon::nation_id{};
			parent->impl_get(state, payload);
			const dcon::nation_id content = any_cast<dcon::nation_id>(payload);
			auto fat = dcon::fatten(state.world, content);
			row_contents.push_back(fat.get_constructing_cb_target().get_identity_from_identity_holder().id);
			update(state);
		}
	}
};

class diplomacy_casus_belli_cancel_button : public button_element_base {
public:
	void on_update(sys::state& state) noexcept override {
		if(parent) {
			Cyto::Any payload = dcon::nation_id{};
			parent->impl_get(state, payload);
			const dcon::nation_id content = any_cast<dcon::nation_id>(payload);
			if(content != state.local_player_nation) {
				disabled = true;
			} else {
				disabled = !command::can_cancel_cb_fabrication(state, content);
			}
		}
	}

	void button_action(sys::state& state) noexcept override {
		if(parent) {
			Cyto::Any payload = dcon::nation_id{};
			parent->impl_get(state, payload);
			const dcon::nation_id content = any_cast<dcon::nation_id>(payload);
			command::cancel_cb_fabrication(state, content);
		}
	}
};

class diplomacy_casus_belli_entry : public listbox_row_element_base<dcon::nation_id> {
public:
	std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override {
		if(name == "diplo_cb_entrybg") {
			return make_element_by_type<image_element_base>(state, id);
		} else if(name == "cb_type_icon") {
			return make_element_by_type<justifying_cb_type_icon>(state, id);
		} else if(name == "cb_progress") {
			return make_element_by_type<justifying_cb_progress>(state, id);
		} else if(name == "cb_progress_overlay") {
			return make_element_by_type<image_element_base>(state, id);
		} else if(name == "cb_progress_text") {
			return make_element_by_type<simple_text_element_base>(state, id);
		} else if(name == "attackers") {
			auto ptr = make_element_by_type<justifying_attacker_flag>(state, id);
			ptr->base_data.position.y -= 7; // Nudge
			return ptr;
		} else if(name == "defenders") {
			auto ptr = make_element_by_type<justifying_defender_flag>(state, id);
			ptr->base_data.position.y -= 7; // Nudge
			return ptr;
		} else if(name == "cancel") {
			return make_element_by_type<diplomacy_casus_belli_cancel_button>(state, id);
		} else {
			return nullptr;
		}
	}
};

class diplomacy_casus_belli_listbox : public listbox_element_base<diplomacy_casus_belli_entry, dcon::nation_id> {
protected:
	std::string_view get_row_element_name() override {
		return "diplomacy_cb_info_player";
	}

public:
	void on_update(sys::state& state) noexcept override {
		row_contents.clear();
		state.world.for_each_nation([&](dcon::nation_id id) {
			if(dcon::fatten(state.world, id).get_constructing_cb_is_discovered() ||
					(id == state.local_player_nation &&
							dcon::fatten(state.world, state.local_player_nation).get_constructing_cb_type().is_valid())) {
				row_contents.push_back(id);
			}
		});
		update(state);
	}
};

class diplomacy_casus_belli_window : public window_element_base {
public:
	std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override {
		if(name == "cb_listbox") {
			auto ptr = make_element_by_type<diplomacy_casus_belli_listbox>(state, id);
			ptr->base_data.position.x -= 400; // Nudge
			return ptr;
		} else {
			return nullptr;
		}
	}
};

class diplomacy_war_listbox : public listbox_element_base<diplomacy_war_info, dcon::war_id> {
protected:
	std::string_view get_row_element_name() override {
		return "diplomacy_war_info";
	}

public:
	void on_update(sys::state& state) noexcept override {
		row_contents.clear();
		state.world.for_each_war([&](dcon::war_id id) { row_contents.push_back(id); });
		update(state);
	}
};

class diplomacy_greatpower_info : public window_element_base {
public:
	uint8_t rank = 0;

	std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override {
		if(name == "country_name") {
			return make_element_by_type<generic_name_text<dcon::nation_id>>(state, id);
		} else if(name == "country_flag") {
			return make_element_by_type<flag_button>(state, id);
		} else if(name == "country_puppets") {
			auto ptr = make_element_by_type<overlapping_sphere_flags>(state, id);
			ptr->base_data.position.y -= 8; // Nudge
			return ptr;
		} else if(name == "gp_prestige") {
			return make_element_by_type<nation_prestige_text>(state, id);
		} else if(name == "gp_economic") {
			return make_element_by_type<nation_industry_score_text>(state, id);
		} else if(name == "gp_military") {
			return make_element_by_type<nation_military_score_text>(state, id);
		} else if(name == "gp_total") {
			return make_element_by_type<nation_total_score_text>(state, id);
		} else {
			return nullptr;
		}
	}

	message_result get(sys::state& state, Cyto::Any& payload) noexcept override {
		if(payload.holds_type<dcon::nation_id>()) {
			payload.emplace<dcon::nation_id>(nations::get_nth_great_power(state, rank));
			return message_result::consumed;
		}
		return window_element_base::get(state, payload);
	}

	void on_update(sys::state& state) noexcept override {
		Cyto::Any payload = nations::get_nth_great_power(state, rank);
		impl_set(state, payload);
	}
};

class diplomacy_sort_nation_gp_flag : public nation_gp_flag {
public:
	message_result test_mouse(sys::state& state, int32_t x, int32_t y, mouse_probe_type type) noexcept override {
		if(has_tooltip(state) == tooltip_behavior::no_tooltip)
			return message_result::unseen;
		return type == mouse_probe_type::tooltip ? message_result::consumed : message_result::unseen;
	}
	void button_action(sys::state& state) noexcept override { }
};

class diplomacy_window : public generic_tabbed_window<diplomacy_window_tab> {
private:
	diplomacy_country_listbox* country_listbox = nullptr;
	diplomacy_war_listbox* war_listbox = nullptr;
	diplomacy_country_facts* country_facts = nullptr;
	diplomacy_action_dialog_window* action_dialog_win = nullptr;
	diplomacy_gp_action_dialog_window* gp_action_dialog_win = nullptr;
	diplomacy_declare_war_dialog* declare_war_win = nullptr;
	diplomacy_setup_peace_dialog* setup_peace_win = nullptr;
	diplomacy_make_cb_window* make_cb_win = nullptr;
	diplomacy_crisis_backdown_window* crisis_backdown_win = nullptr;
	diplomacy_casus_belli_window* casus_belli_window = nullptr;
	// element_base* casus_belli_window = nullptr;
	diplomacy_crisis_info_window* crisis_window = nullptr;

	std::vector<diplomacy_greatpower_info*> gp_infos{};
	std::vector<element_base*> action_buttons{};

	country_list_sort sort = country_list_sort::country;
	bool sort_ascend = true;
	dcon::nation_id facts_nation_id{};

	void filter_countries(sys::state& state, std::function<bool(dcon::nation_id)> filter_fun) {
		if(country_listbox) {
			country_listbox->row_contents.clear();
			state.world.for_each_nation([&](dcon::nation_id id) {
				if(state.world.nation_get_owned_province_count(id) != 0 && filter_fun(id))
					country_listbox->row_contents.push_back(id);
			});
			sort_countries(state, country_listbox->row_contents, sort, sort_ascend);
			country_listbox->update(state);
		}
	}

	void filter_by_continent(sys::state& state, dcon::modifier_id mod_id) {
		filter_countries(state, [&](dcon::nation_id id) -> bool {
			dcon::nation_fat_id fat_id = dcon::fatten(state.world, id);
			auto cont_id = fat_id.get_capital().get_continent().id;
			return mod_id == cont_id;
		});
	}

	template<typename T>
	void add_action_button(sys::state& state, xy_pair offset) noexcept {
		auto ptr = make_element_by_type<T>(state, state.ui_state.defs_by_name.find("diplomacy_option")->second.definition);
		ptr->base_data.position = offset;
		action_buttons.push_back(ptr.get());
		add_child_to_front(std::move(ptr));
	}

public:
	void on_create(sys::state& state) noexcept override {
		generic_tabbed_window::on_create(state);
		set_visible(state, false);
		filter_countries(state, [](dcon::nation_id) { return true; });
		state.ui_state.diplomacy_subwindow = this;

		xy_pair base_gp_info_offset =
				state.ui_defs.gui[state.ui_state.defs_by_name.find("diplomacy_greatpower_pos")->second.definition].position;
		xy_pair gp_info_offset = base_gp_info_offset;
		for(uint8_t i = 0; i < uint8_t(state.defines.great_nations_count); i++) {
			auto ptr = make_element_by_type<diplomacy_greatpower_info>(state,
					state.ui_state.defs_by_name.find("diplomacy_greatpower_info")->second.definition);
			ptr->base_data.position = gp_info_offset;
			ptr->rank = i;
			// Increment gp offset
			gp_info_offset.y += ptr->base_data.size.y;
			if(i + 1 == uint8_t(state.defines.great_nations_count) / 2) {
				gp_info_offset.y = base_gp_info_offset.y;
				gp_info_offset.x += (ptr->base_data.size.x / 2) + 62; // Nudge
			}
			gp_infos.push_back(ptr.get());
			add_child_to_front(std::move(ptr));
		}

		// Fill out all the options for the diplomacy window
		xy_pair options_base_offset =
				state.ui_defs.gui[state.ui_state.defs_by_name.find("diplomacy_actions_pos")->second.definition].position;
		xy_pair options_size = state.ui_defs.gui[state.ui_state.defs_by_name.find("diplomacy_option")->second.definition].size;
		xy_pair options_offset = options_base_offset;
		add_action_button<diplomacy_action_window<diplomacy_action_ally_button>>(state, options_offset);
		options_offset.y += options_size.y;
		add_action_button<diplomacy_action_window<diplomacy_action_call_ally_button>>(state, options_offset);
		options_offset.y += options_size.y;
		add_action_button<diplomacy_action_window<diplomacy_action_military_access_button>>(state, options_offset);
		options_offset.y += options_size.y;
		add_action_button<diplomacy_action_window<diplomacy_action_give_military_access_button>>(state, options_offset);
		options_offset.y += options_size.y;
		add_action_button<diplomacy_action_window<diplomacy_action_increase_relations_button>>(state, options_offset);
		options_offset.y += options_size.y;
		add_action_button<diplomacy_action_window<diplomacy_action_decrease_relations_button>>(state, options_offset);
		options_offset.y += options_size.y;
		add_action_button<diplomacy_action_window<diplomacy_action_war_subisides_button>>(state, options_offset);
		options_offset.y += options_size.y;
		add_action_button<diplomacy_action_window<diplomacy_action_declare_war_button>>(state, options_offset);
		options_offset.y += options_size.y;
		add_action_button<diplomacy_action_window<diplomacy_action_command_units_button>>(state, options_offset);
		// Next row of actions...
		options_offset.x += options_size.x;
		options_offset.y = options_base_offset.y;
		add_action_button<diplomacy_action_window<diplomacy_action_discredit_button>>(state, options_offset);
		options_offset.y += options_size.y;
		add_action_button<diplomacy_action_window<diplomacy_action_expel_advisors_button>>(state, options_offset);
		options_offset.y += options_size.y;
		add_action_button<diplomacy_action_window<diplomacy_action_ban_embassy_button>>(state, options_offset);
		options_offset.y += options_size.y;
		add_action_button<diplomacy_action_window<diplomacy_action_increase_opinion_button>>(state, options_offset);
		options_offset.y += options_size.y;
		add_action_button<diplomacy_action_window<diplomacy_action_decrease_opinion_button>>(state, options_offset);
		options_offset.y += options_size.y;
		add_action_button<diplomacy_action_window<diplomacy_action_add_to_sphere_button>>(state, options_offset);
		options_offset.y += options_size.y;
		add_action_button<diplomacy_action_window<diplomacy_action_remove_from_sphere_button>>(state, options_offset);
		options_offset.y += options_size.y;
		add_action_button<diplomacy_action_window<diplomacy_action_justify_war_button>>(state, options_offset);

		auto new_win1 = make_element_by_type<diplomacy_action_dialog_window>(state,
				state.ui_state.defs_by_name.find("defaultdiplomacydialog")->second.definition);
		new_win1->set_visible(state, false);
		action_dialog_win = new_win1.get();
		add_child_to_front(std::move(new_win1));

		auto new_win2 = make_element_by_type<diplomacy_gp_action_dialog_window>(state,
				state.ui_state.defs_by_name.find("gpselectdiplomacydialog")->second.definition);
		new_win2->set_visible(state, false);
		gp_action_dialog_win = new_win2.get();
		add_child_to_front(std::move(new_win2));

		auto new_win3 = make_element_by_type<diplomacy_declare_war_dialog>(state,
				state.ui_state.defs_by_name.find("declarewardialog")->second.definition);
		new_win3->set_visible(state, false);
		declare_war_win = new_win3.get();
		add_child_to_front(std::move(new_win3));

		auto new_win4 = make_element_by_type<diplomacy_setup_peace_dialog>(state,
				state.ui_state.defs_by_name.find("setuppeacedialog")->second.definition);
		new_win4->set_visible(state, false);
		setup_peace_win = new_win4.get();
		add_child_to_front(std::move(new_win4));

		auto new_win5 = make_element_by_type<diplomacy_make_cb_window>(state,
				state.ui_state.defs_by_name.find("makecbdialog")->second.definition);
		new_win5->set_visible(state, false);
		make_cb_win = new_win5.get();
		add_child_to_front(std::move(new_win5));

		auto new_win6 = make_element_by_type<diplomacy_crisis_backdown_window>(state,
				state.ui_state.defs_by_name.find("setupcrisisbackdowndialog")->second.definition);
		new_win6->set_visible(state, false);
		crisis_backdown_win = new_win6.get();
		add_child_to_front(std::move(new_win6));

		facts_nation_id = state.local_player_nation;
	}

	std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override {
		if(name == "close_button") {
			return make_element_by_type<generic_close_button>(state, id);
		} else if(name == "gp_info") {
			auto ptr = make_element_by_type<generic_tab_button<diplomacy_window_tab>>(state, id);
			ptr->target = diplomacy_window_tab::great_powers;
			return ptr;
		} else if(name == "war_info") {
			auto ptr = make_element_by_type<generic_tab_button<diplomacy_window_tab>>(state, id);
			ptr->target = diplomacy_window_tab::wars;
			return ptr;
		} else if(name == "cb_info") {
			auto ptr = make_element_by_type<generic_tab_button<diplomacy_window_tab>>(state, id);
			ptr->target = diplomacy_window_tab::casus_belli;
			return ptr;
		} else if(name == "crisis_info") {
			auto ptr = make_element_by_type<generic_tab_button<diplomacy_window_tab>>(state, id);
			ptr->target = diplomacy_window_tab::crisis;
			return ptr;
		} else if(name == "filter_all") {
			auto ptr = make_element_by_type<generic_tab_button<country_list_filter>>(state, id);
			ptr->target = country_list_filter::all;
			return ptr;
		} else if(name == "filter_enemies") {
			auto ptr = make_element_by_type<generic_tab_button<country_list_filter>>(state, id);
			ptr->target = country_list_filter::enemies;
			return ptr;
		} else if(name == "filter_allies") {
			auto ptr = make_element_by_type<generic_tab_button<country_list_filter>>(state, id);
			ptr->target = country_list_filter::allies;
			return ptr;
		} else if(name == "filter_neighbours") {
			auto ptr = make_element_by_type<generic_tab_button<country_list_filter>>(state, id);
			ptr->target = country_list_filter::neighbors;
			return ptr;
		} else if(name == "filter_sphere") {
			auto ptr = make_element_by_type<generic_tab_button<country_list_filter>>(state, id);
			ptr->target = country_list_filter::sphere;
			return ptr;
		} else if(name == "cb_info_win") {
			auto ptr = make_element_by_type<diplomacy_casus_belli_window>(state, id);
			// auto ptr = make_element_immediate(state, id);
			casus_belli_window = ptr.get();
			ptr->set_visible(state, false);
			return ptr;
		} else if(name == "crisis_info_win") {
			auto ptr = make_element_by_type<diplomacy_crisis_info_window>(state, id);
			crisis_window = ptr.get();
			ptr->set_visible(state, false);
			return ptr;
		} else if(name == "country_listbox") {
			auto ptr = make_element_by_type<diplomacy_country_listbox>(state, id);
			country_listbox = ptr.get();
			return ptr;
		} else if(name == "war_listbox") {
			auto ptr = make_element_by_type<diplomacy_war_listbox>(state, id);
			war_listbox = ptr.get();
			war_listbox->set_visible(state, false);
			return ptr;
		} else if(name == "diplomacy_country_facts") {
			auto ptr = make_element_by_type<diplomacy_country_facts>(state, id);
			country_facts = ptr.get();
			return ptr;
		} else if(name == "sort_by_boss") {
			auto ptr = make_element_by_type<country_sort_button<country_list_sort::boss>>(state, id);
			ptr->base_data.position.y -= 1; // Nudge
			return ptr;
		} else if(name == "sort_by_prestige") {
			auto ptr = make_element_by_type<country_sort_button<country_list_sort::prestige_rank>>(state, id);
			ptr->base_data.position.y -= 1; // Nudge
			return ptr;
		} else if(name == "sort_by_economic") {
			auto ptr = make_element_by_type<country_sort_button<country_list_sort::economic_rank>>(state, id);
			ptr->base_data.position.y -= 1; // Nudge
			return ptr;
		} else if(name == "sort_by_military") {
			auto ptr = make_element_by_type<country_sort_button<country_list_sort::military_rank>>(state, id);
			ptr->base_data.position.y -= 1; // Nudge
			return ptr;
		} else if(name == "sort_by_total") {
			auto ptr = make_element_by_type<country_sort_button<country_list_sort::total_rank>>(state, id);
			ptr->base_data.position.y -= 1; // Nudge
			return ptr;
		} else if(name == "sort_by_relation") {
			auto ptr = make_element_by_type<country_sort_button<country_list_sort::relation>>(state, id);
			ptr->base_data.position.y -= 1; // Nudge
			return ptr;
		} else if(name == "sort_by_opinion") {
			return make_element_by_type<country_sort_button<country_list_sort::opinion>>(state, id);
		} else if(name == "sort_by_prio") {
			return make_element_by_type<country_sort_button<country_list_sort::priority>>(state, id);
		} else if(name.substr(0, 14) == "sort_by_gpflag") {
			auto ptr = make_element_by_type<diplomacy_sort_nation_gp_flag>(state, id);
			ptr->rank = uint16_t(std::stoi(std::string{name.substr(14)}));
			ptr->base_data.position.y -= 2; // Nudge
			return ptr;
		} else if(name.substr(0, 10) == "sort_by_gp") {
			auto ptr = make_element_by_type<country_sort_button<country_list_sort::gp_influence>>(state, id);
			ptr->offset = uint8_t(std::stoi(std::string{name.substr(10)}));
			ptr->base_data.position.y -= 2; // Nudge
			return ptr;
		} else if(name.length() >= 7 && name.substr(0, 7) == "filter_") {
			auto const filter_name = name.substr(7);
			auto ptr = make_element_by_type<generic_tab_button<dcon::modifier_id>>(state, id);
			ptr->target = ([&]() {
				dcon::modifier_id filter_mod_id{0};
				auto it = state.key_to_text_sequence.find(parsers::lowercase_str(filter_name));
				if(it != state.key_to_text_sequence.end())
					state.world.for_each_modifier([&](dcon::modifier_id mod_id) {
						auto fat_id = dcon::fatten(state.world, mod_id);
						if(it->second == fat_id.get_name())
							filter_mod_id = mod_id;
					});
				return filter_mod_id;
			})();
			return ptr;
		} else {
			return nullptr;
		}
	}

	void hide_tabs(sys::state& state) {
		war_listbox->set_visible(state, false);
		casus_belli_window->set_visible(state, false);
		crisis_window->set_visible(state, false);
		for(auto e : gp_infos)
			e->set_visible(state, false);
	}

	message_result get(sys::state& state, Cyto::Any& payload) noexcept override {
		if(payload.holds_type<diplomacy_window_tab>()) {
			auto enum_val = any_cast<diplomacy_window_tab>(payload);
			hide_tabs(state);
			switch(enum_val) {
			case diplomacy_window_tab::great_powers:
				for(auto e : gp_infos)
					e->set_visible(state, true);
				break;
			case diplomacy_window_tab::wars:
				war_listbox->set_visible(state, true);
				break;
			case diplomacy_window_tab::casus_belli:
				casus_belli_window->set_visible(state, true);
				break;
			case diplomacy_window_tab::crisis:
				crisis_window->set_visible(state, true);
				break;
			}
			active_tab = enum_val;
			return message_result::consumed;
		} else if(payload.holds_type<element_selection_wrapper<country_list_sort>>()) {
			auto new_sort = any_cast<element_selection_wrapper<country_list_sort>>(payload).data;
			sort_ascend = (new_sort == sort) ? !sort_ascend : true;
			sort = new_sort;
			sort_countries(state, country_listbox->row_contents, sort, sort_ascend);
			country_listbox->update(state);
			return message_result::consumed;
		} else if(payload.holds_type<country_list_filter>()) {
			auto filter = any_cast<country_list_filter>(payload);
			switch(filter) {
			case country_list_filter::all:
				filter_countries(state, [&](dcon::nation_id) { return true; });
				break;
			case country_list_filter::allies:
				filter_countries(state, [&](dcon::nation_id id) {
					if(id == state.local_player_nation)
						return false;
					auto rel = state.world.get_diplomatic_relation_by_diplomatic_pair(id, state.local_player_nation);
					return state.world.diplomatic_relation_get_are_allied(rel) ||
								 military::are_allied_in_war(state, state.local_player_nation, id);
				});
				break;
			case country_list_filter::enemies:
				filter_countries(state, [&](dcon::nation_id id) { return military::are_at_war(state, state.local_player_nation, id); });
				break;
			case country_list_filter::sphere:
				filter_countries(state,
						[&](dcon::nation_id id) { return state.world.nation_get_in_sphere_of(id) == state.local_player_nation; });
				break;
			case country_list_filter::neighbors:
				filter_countries(state, [&](dcon::nation_id id) {
					return bool(state.world.get_nation_adjacency_by_nation_adjacency_pair(state.local_player_nation, id));
				});
				break;
			default:
				break;
			}
			return message_result::consumed;
		} else if(payload.holds_type<dcon::modifier_id>()) {
			auto mod_id = any_cast<dcon::modifier_id>(payload);
			filter_by_continent(state, mod_id);
			return message_result::consumed;
		} else if(payload.holds_type<dcon::nation_id>()) {
			payload.emplace<dcon::nation_id>(facts_nation_id);
			return message_result::consumed;
		} else if(payload.holds_type<element_selection_wrapper<dcon::nation_id>>()) {
			facts_nation_id = any_cast<element_selection_wrapper<dcon::nation_id>>(payload).data;
			impl_on_update(state);
			return message_result::consumed;
		} else if(payload.holds_type<diplomacy_action>()) {
			auto v = any_cast<diplomacy_action>(payload);
			gp_action_dialog_win->set_visible(state, false);
			action_dialog_win->set_visible(state, false);
			declare_war_win->set_visible(state, false);
			setup_peace_win->set_visible(state, false);
			make_cb_win->set_visible(state, false);
			crisis_backdown_win->set_visible(state, false);
			Cyto::Any new_payload = facts_nation_id;
			auto fat = dcon::fatten(state.world, facts_nation_id);
			switch(v) {
			case diplomacy_action::add_to_sphere:
				command::add_to_sphere(state, state.local_player_nation, facts_nation_id);
				break;
			case diplomacy_action::military_access:
				command::ask_for_military_access(state, state.local_player_nation, facts_nation_id);
				break;
			case diplomacy_action::cancel_military_access:
				command::cancel_military_access(state, state.local_player_nation, facts_nation_id);
				break;
			case diplomacy_action::give_military_access:
				// TODO: Give military access
				break;
			case diplomacy_action::cancel_give_military_access:
				command::cancel_given_military_access(state, state.local_player_nation, facts_nation_id);
				break;
			case diplomacy_action::increase_relations:
				command::increase_relations(state, state.local_player_nation, facts_nation_id);
				break;
			case diplomacy_action::decrease_relations:
				command::decrease_relations(state, state.local_player_nation, facts_nation_id);
				break;
			case diplomacy_action::war_subsidies:
				command::give_war_subsidies(state, state.local_player_nation, facts_nation_id);
				break;
			case diplomacy_action::cancel_war_subsidies:
				command::cancel_war_subsidies(state, state.local_player_nation, facts_nation_id);
				break;
			case diplomacy_action::ally:
				command::ask_for_alliance(state, state.local_player_nation, facts_nation_id);
				break;
			case diplomacy_action::cancel_ally:
				command::cancel_alliance(state, state.local_player_nation, facts_nation_id);
				break;
			case diplomacy_action::call_ally:
				for(auto war_par : fat.get_war_participant()) {
					command::call_to_arms(state, state.local_player_nation, facts_nation_id,
							dcon::fatten(state.world, war_par).get_war().id);
				}
				break;
			case diplomacy_action::discredit:
			case diplomacy_action::expel_advisors:
			case diplomacy_action::ban_embassy:
			case diplomacy_action::decrease_opinion:
			case diplomacy_action::remove_from_sphere:
				gp_action_dialog_win->set_visible(state, true);
				gp_action_dialog_win->impl_set(state, new_payload);
				gp_action_dialog_win->impl_set(state, payload);
				gp_action_dialog_win->impl_on_update(state);
				break;
			case diplomacy_action::declare_war:
			case diplomacy_action::add_wargoal:
				declare_war_win->set_visible(state, true);
				declare_war_win->impl_set(state, new_payload);
				declare_war_win->impl_set(state, payload);
				declare_war_win->impl_on_update(state);
				break;
			case diplomacy_action::make_peace:
				setup_peace_win->set_visible(state, true);
				setup_peace_win->impl_set(state, new_payload);
				setup_peace_win->impl_set(state, payload);
				setup_peace_win->impl_on_update(state);
				break;
			case diplomacy_action::justify_war:
				make_cb_win->set_visible(state, true);
				make_cb_win->impl_set(state, new_payload);
				make_cb_win->impl_set(state, payload);
				make_cb_win->impl_on_update(state);
				break;
			case diplomacy_action::crisis_backdown:
				crisis_backdown_win->set_visible(state, true);
				crisis_backdown_win->impl_set(state, new_payload);
				crisis_backdown_win->impl_set(state, payload);
				crisis_backdown_win->impl_on_update(state);
				break;
			case diplomacy_action::crisis_support:
				break;
			default:
				action_dialog_win->set_visible(state, true);
				action_dialog_win->impl_set(state, new_payload);
				action_dialog_win->impl_set(state, payload);
				action_dialog_win->impl_on_update(state);
				break;
			}
			return message_result::consumed;
		}
		return generic_tabbed_window<diplomacy_window_tab>::get(state, payload);
	}
};

} // namespace ui
