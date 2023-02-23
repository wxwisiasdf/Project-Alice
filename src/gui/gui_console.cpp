#include "gui_console.hpp"
#include "gui_fps_counter.hpp"

void set_active_tag(sys::state& state, std::string_view tag) noexcept {
    auto name_id = text::find_or_add_key(state, tag);

    state.world.for_each_nation([&](dcon::nation_id nat_id) {
        dcon::nation_fat_id fat_id = dcon::fatten(state.world, nat_id);
        if(fat_id.get_name() == name_id) {
            state.local_player_nation = nat_id;
        }
    });
}

void ui::console_edit::edit_box_enter(sys::state& state, std::string_view s) noexcept {
    if(s == "reload") {
        state.map_display.load_map(state);
    } else if(s == "abort") {
        std::abort();
    } else if(s == "fps") {
        if(!state.ui_state.fps_counter) {
			auto window = make_element_by_type<fps_counter_text_box>(state, "fps_counter");
			state.ui_state.fps_counter = window.get();
			state.ui_state.root->add_child_to_front(std::move(window));
		} else if(state.ui_state.fps_counter->is_visible()) {
			state.ui_state.fps_counter->set_visible(state, false);
		} else {
			state.ui_state.fps_counter->set_visible(state, true);
			state.ui_state.root->move_child_to_front(state.ui_state.fps_counter);
		}
    } else if(s.starts_with("tag ") && s.size() == 7) {
        set_active_tag(state, s.substr(4));
    }
    Cyto::Any output = std::string(s);
    parent->impl_get(state, output);
}

void ui::console_window::show_toggle(sys::state& state) {
	if(!state.ui_state.console_window) {
		auto window = make_element_by_type<console_window>(state, "console_wnd");
		state.ui_state.console_window = window.get();
		state.ui_state.root->add_child_to_front(std::move(window));
	} else if(state.ui_state.console_window->is_visible()) {
            state.ui_state.console_window->set_visible(state, false);
    } else {
        state.ui_state.console_window->set_visible(state, true);
        state.ui_state.root->move_child_to_front(state.ui_state.console_window);
    }
}
