#include <string_view>
#include "dcon_generated.hpp"
#include "system_state.hpp"
#include "text.hpp"
#include "script_constants.hpp"

namespace ui {

enum class modifier_display_type : uint8_t {
	integer,
	percent,
	fp_two_places,
	fp_three_places,
};
struct modifier_display_info {
	bool positive_is_green;
	modifier_display_type type;
	std::string_view name;
};

static const modifier_display_info province_modifier_names[sys::provincial_mod_offsets::count] = {
#define MOD_LIST_ELEMENT(num, name, green_is_negative, display_type, locale_name) \
	modifier_display_info{green_is_negative, display_type, locale_name},
    MOD_PROV_LIST
#undef MOD_LIST_ELEMENT
};
static const modifier_display_info national_modifier_names[sys::national_mod_offsets::count] = {
#define MOD_LIST_ELEMENT(num, name, green_is_negative, display_type, locale_name) \
	modifier_display_info{green_is_negative, display_type, locale_name},
    MOD_NAT_LIST
#undef MOD_LIST_ELEMENT
};

std::string format_modifier_value(sys::state& state, float value, modifier_display_type type) {
	switch(type) {
	case modifier_display_type::integer:
		return (value >= 0.f ? "+" : "") + text::prettify(int64_t(value));
	case modifier_display_type::percent:
		return (value >= 0.f ? "+" : "") + text::format_percentage(value, 1);
	case modifier_display_type::fp_two_places:
		return text::format_float(value, 2);
	case modifier_display_type::fp_three_places:
		return text::format_float(value, 3);
	}
	return "x%";
}

void modifier_description(sys::state& state, text::layout_base& layout, dcon::modifier_id mid, int32_t indentation) {
	auto fat_id = dcon::fatten(state.world, mid);

	const auto& prov_def = fat_id.get_province_values();
	for(uint32_t i = 0; i < prov_def.modifier_definition_size; ++i) {
		if(!bool(prov_def.offsets[i]))
			break;
		auto data = province_modifier_names[prov_def.offsets[i].index()];
		auto box = text::open_layout_box(layout, indentation);
		text::add_to_layout_box(layout, state, box, text::produce_simple_string(state, data.name), text::text_color::white);
		text::add_to_layout_box(layout, state, box, std::string_view{":"}, text::text_color::white);
		text::add_space_to_layout_box(layout, state, box);
		auto color = data.positive_is_green
		                 ? (prov_def.values[i] >= 0.f ? text::text_color::green : text::text_color::red)
		                 : (prov_def.values[i] >= 0.f ? text::text_color::red : text::text_color::green);
		text::add_to_layout_box(layout, state, box, format_modifier_value(state, prov_def.values[i], data.type), color);
		text::close_layout_box(layout, box);
	}

	const auto& nat_def = fat_id.get_national_values();
	for(uint32_t i = 0; i < nat_def.modifier_definition_size; ++i) {
		if(!bool(nat_def.offsets[i]))
			break;
		auto data = national_modifier_names[nat_def.offsets[i].index()];
		auto box = text::open_layout_box(layout, indentation);
		text::add_to_layout_box(layout, state, box, text::produce_simple_string(state, data.name), text::text_color::white);
		text::add_to_layout_box(layout, state, box, std::string_view{":"}, text::text_color::white);
		text::add_space_to_layout_box(layout, state, box);
		auto color = data.positive_is_green
		                 ? (nat_def.values[i] >= 0.f ? text::text_color::green : text::text_color::red)
		                 : (nat_def.values[i] >= 0.f ? text::text_color::red : text::text_color::green);
		text::add_to_layout_box(layout, state, box, format_modifier_value(state, nat_def.values[i], data.type), color);
		text::close_layout_box(layout, box);
	}
}

void acting_modifier_description(sys::state& state, text::layout_base& layout, dcon::modifier_id mid, int32_t indentation, dcon::national_modifier_value nmid, float scaled = 1.f) {
	if(scaled == 0.f)
		return;
	auto fat_id = dcon::fatten(state.world, mid);
	const auto& def = fat_id.get_national_values();
	for(uint32_t i = 0; i < def.modifier_definition_size; ++i) {
		if(!bool(def.offsets[i]))
			break;
		if(def.offsets[i] != nmid)
			continue;
		auto data = national_modifier_names[nmid.index()];
		auto box = text::open_layout_box(layout, indentation);
		text::add_to_layout_box(layout, state, box, text::produce_simple_string(state, fat_id.get_name()), text::text_color::white);
		text::add_to_layout_box(layout, state, box, std::string_view{":"}, text::text_color::white);
		text::add_space_to_layout_box(layout, state, box);
		auto value = def.values[i] * scaled;
		auto color = data.positive_is_green
		                 ? (value >= 0.f ? text::text_color::green : text::text_color::red)
		                 : (value >= 0.f ? text::text_color::red : text::text_color::green);
		text::add_to_layout_box(layout, state, box, format_modifier_value(state, value, data.type), color);
		text::close_layout_box(layout, box);
	}
}
void acting_modifier_description(sys::state& state, text::layout_base& layout, dcon::modifier_id mid, int32_t indentation, dcon::provincial_modifier_value pmid, float scaled = 1.f) {
	if(scaled == 0.f)
		return;
	auto fat_id = dcon::fatten(state.world, mid);
	const auto& def = fat_id.get_province_values();
	for(uint32_t i = 0; i < def.modifier_definition_size; ++i) {
		if(!bool(def.offsets[i]))
			break;
		if(def.offsets[i] != pmid)
			continue;
		auto data = national_modifier_names[pmid.index()];
		auto box = text::open_layout_box(layout, indentation);
		text::add_to_layout_box(layout, state, box, text::produce_simple_string(state, fat_id.get_name()), text::text_color::white);
		text::add_to_layout_box(layout, state, box, std::string_view{":"}, text::text_color::white);
		text::add_space_to_layout_box(layout, state, box);
		auto value = def.values[i] * scaled;
		auto color = data.positive_is_green
		                 ? (value >= 0.f ? text::text_color::green : text::text_color::red)
		                 : (value >= 0.f ? text::text_color::red : text::text_color::green);
		text::add_to_layout_box(layout, state, box, format_modifier_value(state, value, data.type), color);
		text::close_layout_box(layout, box);
	}
}

template<typename T>
void acting_modifiers_description_province(sys::state& state, text::layout_base& layout, dcon::province_id p, int32_t identation, T nmid) {
	if(state.national_definitions.land_province)
		acting_modifier_description(state, layout, state.national_definitions.land_province, identation, nmid);
	for(auto mpr : state.world.province_get_current_modifiers(p))
		acting_modifier_description(state, layout, mpr.mod_id, identation, nmid);
	if(auto m = state.world.province_get_terrain(p); m)
		acting_modifier_description(state, layout, m, identation, nmid);
	if(auto m = state.world.province_get_climate(p); m)
		acting_modifier_description(state, layout, m, identation, nmid);
	if(auto m = state.world.province_get_continent(p); m)
		acting_modifier_description(state, layout, m, identation, nmid);
	if(auto c = state.world.province_get_crime(p); c) {
		if(auto m = state.culture_definitions.crimes[c].modifier; m)
			acting_modifier_description(state, layout, m, identation, nmid);
	}
	if(state.economy_definitions.railroad_definition.province_modifier) {
		acting_modifier_description(state, layout, state.economy_definitions.railroad_definition.province_modifier, identation, nmid, state.world.province_get_railroad_level(p));
	}
	if(state.national_definitions.infrastructure) {
		acting_modifier_description(state, layout, state.national_definitions.infrastructure, identation, nmid, state.world.province_get_railroad_level(p) * state.economy_definitions.railroad_definition.infrastructure);
	}
	if(state.economy_definitions.fort_definition.province_modifier) {
		acting_modifier_description(state, layout, state.economy_definitions.fort_definition.province_modifier, identation, nmid, state.world.province_get_fort_level(p));
	}
	if(state.economy_definitions.naval_base_definition.province_modifier) {
		acting_modifier_description(state, layout, state.economy_definitions.naval_base_definition.province_modifier, identation, nmid, state.world.province_get_naval_base_level(p));
	}
	if(state.national_definitions.nationalism) {
		acting_modifier_description(state, layout, state.national_definitions.nationalism, identation, nmid, (state.world.province_get_is_owner_core(p) ? 1.f : 0.f) * state.world.province_get_nationalism(p));
	}
	if(state.national_definitions.non_coastal) {
		acting_modifier_description(state, layout, state.national_definitions.non_coastal, identation, nmid, !state.world.province_get_is_coast(p) ? 1.f : 0.f);
	}
	if(state.national_definitions.coastal) {
		acting_modifier_description(state, layout, state.national_definitions.coastal, identation, nmid, state.world.province_get_is_coast(p) ? 1.f : 0.f);
	}
	if(state.national_definitions.overseas) {
		acting_modifier_description(state, layout, state.national_definitions.overseas, identation, nmid, province::is_overseas(state, p) ? 1.f : 0.f);
	}
	if(state.national_definitions.core) {
		acting_modifier_description(state, layout, state.national_definitions.core, identation, nmid, state.world.province_get_is_owner_core(p) ? 1.f : 0.f);
	}
	if(state.national_definitions.has_siege) {
		acting_modifier_description(state, layout, state.national_definitions.has_siege, identation, nmid, military::province_is_under_siege(state, p) ? 1.f : 0.f);
	}
	if(state.national_definitions.blockaded) {
		acting_modifier_description(state, layout, state.national_definitions.blockaded, identation, nmid, military::province_is_blockaded(state, p) ? 1.f : 0.f);
	}
}

void acting_modifiers_description(sys::state& state, text::layout_base& layout, dcon::nation_id n, int32_t identation, dcon::national_modifier_value nmid) {
	if(auto ts = state.world.nation_get_tech_school(n); ts)
		acting_modifier_description(state, layout, ts, identation, nmid);
	if(auto nv = state.world.nation_get_national_value(n); nv)
		acting_modifier_description(state, layout, nv, identation, nmid);
	for(auto mpr : state.world.nation_get_current_modifiers(n))
		acting_modifier_description(state, layout, mpr.mod_id, identation, nmid);
	state.world.for_each_technology([&](dcon::technology_id t) {
		auto tmod = state.world.technology_get_modifier(t);
		if(tmod && state.world.nation_get_active_technologies(n, t))
			acting_modifier_description(state, layout, tmod, identation, nmid);
	});
	state.world.for_each_invention([&](dcon::invention_id i) {
		auto tmod = state.world.invention_get_modifier(i);
		if(tmod && state.world.nation_get_active_inventions(n, i))
			acting_modifier_description(state, layout, tmod, identation, nmid);
	});
	state.world.for_each_issue([&](dcon::issue_id i) {
		auto iopt = state.world.nation_get_issues(n, i);
		auto imod = state.world.issue_option_get_modifier(iopt);
		if(imod && (state.world.nation_get_is_civilized(n) || state.world.issue_get_issue_type(i) == uint8_t(culture::issue_type::party)))
			acting_modifier_description(state, layout, imod, identation, nmid);
	});
	if(!state.world.nation_get_is_civilized(n)) {
		state.world.for_each_reform([&](dcon::reform_id i) {
			auto iopt = state.world.nation_get_reforms(n, i);
			auto imod = state.world.reform_option_get_modifier(iopt);
			if(imod)
				acting_modifier_description(state, layout, imod, identation, nmid);
		});
	}

	auto in_wars = state.world.nation_get_war_participant(n);
	if(in_wars.begin() != in_wars.end()) {
		if(state.national_definitions.war)
			acting_modifier_description(state, layout, state.national_definitions.war, identation, nmid);
	} else {
		if(state.national_definitions.peace)
			acting_modifier_description(state, layout, state.national_definitions.peace, identation, nmid);
	}

	if(state.national_definitions.badboy) {
		acting_modifier_description(state, layout, state.national_definitions.badboy, identation, nmid, state.world.nation_get_infamy(n));
	}
	if(state.national_definitions.plurality) {
		acting_modifier_description(state, layout, state.national_definitions.plurality, identation, nmid, state.world.nation_get_plurality(n));
	}
	if(state.national_definitions.war_exhaustion) {
		acting_modifier_description(state, layout, state.national_definitions.war_exhaustion, identation, nmid, state.world.nation_get_war_exhaustion(n));
	}
	if(state.national_definitions.average_literacy) {
		auto total = state.world.nation_get_demographics(n, demographics::total);
		acting_modifier_description(state, layout, state.national_definitions.average_literacy, identation, nmid, total > 0 ? state.world.nation_get_demographics(n, demographics::literacy) / total : 0.0f);
	}
	if(state.national_definitions.total_blockaded) {
		auto bc = ve::to_float(state.world.nation_get_central_blockaded(n));
		auto c = ve::to_float(state.world.nation_get_central_ports(n));
		acting_modifier_description(state, layout, state.national_definitions.total_blockaded, identation, nmid, c > 0.0f ? bc / c : 0.0f);
	}
	if(state.national_definitions.total_occupation) {
		auto nid = fatten(state.world, n);
		auto cap_continent = nid.get_capital().get_continent();
		float total = 0.0f;
		float occupied = 0.0f;
		for(auto owned : nid.get_province_ownership()) {
			if(owned.get_province().get_continent() == cap_continent) {
				total += 1.0f;
				if(auto c = owned.get_province().get_nation_from_province_control().id; c && c != n) {
					occupied += 1.0f;
				}
			}
		}
		acting_modifier_description(state, layout, state.national_definitions.total_occupation, identation, nmid, total > 0.0f ? occupied / total : 0.0f);
	}

	if(state.world.nation_get_is_civilized(n) == false) {
		if(state.national_definitions.unciv_nation)
			acting_modifier_description(state, layout, state.national_definitions.unciv_nation, identation, nmid);
	} else if(nations::is_great_power(state, n)) {
		if(state.national_definitions.great_power)
			acting_modifier_description(state, layout, state.national_definitions.great_power, identation, nmid);
	} else if(state.world.nation_get_rank(n) <= uint16_t(state.defines.colonial_rank)) {
		if(state.national_definitions.second_power)
			acting_modifier_description(state, layout, state.national_definitions.second_power, identation, nmid);
	} else {
		if(state.national_definitions.civ_nation)
			acting_modifier_description(state, layout, state.national_definitions.civ_nation, identation, nmid);
	}

	if(state.national_definitions.disarming) {
		if(bool(state.world.nation_get_disarmed_until(n)) && state.world.nation_get_disarmed_until(n) > state.current_date)
			acting_modifier_description(state, layout, state.national_definitions.disarming, identation, nmid);
	}
	if(state.national_definitions.in_bankrupcy) {
		if(bool(state.world.nation_get_is_bankrupt(n)))
			acting_modifier_description(state, layout, state.national_definitions.in_bankrupcy, identation, nmid);
	}
	// TODO: debt

	for(auto tm : state.national_definitions.triggered_modifiers) {
		if(tm.trigger_condition && tm.linked_modifier) {
			auto trigger_condition_satisfied = trigger::evaluate(state, tm.trigger_condition, trigger::to_generic(n), trigger::to_generic(n), 0);
			if(trigger_condition_satisfied)
				acting_modifier_description(state, layout, tm.linked_modifier, identation, nmid);
		}
	}

	// Provinces of this nation
	for(auto pc : state.world.nation_get_province_ownership_as_nation(n)) {
		auto p = pc.get_province().id;
		acting_modifiers_description_province<dcon::national_modifier_value>(state, layout, p, identation, nmid);
	}
}

} // namespace ui
