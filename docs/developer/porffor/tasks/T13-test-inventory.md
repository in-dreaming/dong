# T13 — 全量测试迁移盘点

> 自动生成：`node dong/scripts/porffor_test_inventory.mjs`（勿手改统计表，改生成器或 HTML 标记后重跑）

## 摘要

| 指标 | 值 |
|------|-----|
| 测试总数 | 286 |
| ready | 280 (97.9%) |
| pending | 0 |
| blocked | 0 |
| dropped | 6 |
| 无 script（静态渲染） | 138 |
| 有 script | 148 |

## blocked 分批（显式标记）

| 阻塞任务 | 数量 |
|----------|------|
| blocked(T20) | 0 |

| dropped 原因 | 数量 |
|-------------|------|
| skin/theme experiment | 1 |
| Promise microtask ordering | 1 |
| FormData object API per T20 | 1 |
| gamepad platform API | 1 |
| comprehensive demo page | 1 |
| dong.scene API | 1 |

详见 [T13-pending-batches.md](./T13-pending-batches.md)。

## 构建组织选型（阶段 0 定稿）

**选定方案：a) 单 test runner 链接全部 Porffor 模块**

| 方案 | 说明 | 结论 |
|------|------|------|
| a) 单 runner + 全量 registry | manifest → `porffor_compile.mjs` → `generated/porffor/registry.c`；`html_render_test` / CI 一次链接 | **采用** — 实现简单，当前模块数 <20，链接增量可接受 |
| b) 按目录分组多 registry | 多二进制，按 tests 子集跑 | 暂缓 — 模块上百后再评估 |
| c) 每测试独立编译 + 缓存 | 增量友好，管线复杂 | 不采用 — 维护成本高 |

**实测（本机 node 计时，仅 compile 步）**：`node scripts/porffor_compile.mjs` 在 ~10 个模块时 <3s；全量 150 模块预估 <30s compile + 链接时间随模块线性增长，CI 可接受。

**合入门槛**：CI Porffor job 只跑 `ready` 集；`pending`/`blocked` 计入覆盖率摘要，不阻塞合入。

## 迁移分类（script 测试）

| 分类 | 数量 |
|------|------|
| static-render | 138 |
| direct | 88 |
| blocked(T20) | 44 |
| blocked(T12) | 6 |
| pending-review | 6 |
| blocked | 2 |
| T14-snippet | 1 |
| blocked(T19) | 1 |

## 全量清单

| 文件 | porffor 状态 | 显式标记 | 阻塞原因 | script | 模块 | 迁移分类 |
|------|-------------|---------|---------|--------|------|---------|
| background_attachment_fixed_test.html | ready | yes |  | no |  | static-render |
| background_clip_test.html | ready | yes |  | no |  | static-render |
| background_origin_test.html | ready | yes |  | no |  | static-render |
| chinese_font_large_test.html | ready | yes |  | no |  | static-render |
| chinese_font_medium_test.html | ready | yes |  | no |  | static-render |
| chinese_font_sizes_test.html | ready | yes |  | no |  | static-render |
| chinese_font_small_test.html | ready | yes |  | no |  | static-render |
| chinese_font_tiny_test.html | ready | yes |  | no |  | static-render |
| complex.html | ready | yes |  | no |  | static-render |
| css_global_keywords_comprehensive.html | ready | yes |  | no |  | static-render |
| css_global_keywords_inherit.html | ready | yes |  | no |  | static-render |
| css_global_keywords_initial.html | ready | yes |  | no |  | static-render |
| css_global_keywords_unset.html | ready | yes |  | no |  | static-render |
| cursor_test.html | ready | yes |  | no |  | static-render |
| focus_visible_test.html | ready | yes |  | yes | t12_focus_visible_test | direct |
| font_style_test.html | ready | yes |  | no |  | static-render |
| getcomputedstyle_smoke_test.html | ready | yes |  | yes |  | direct |
| ifc_test.html | ready | yes |  | no |  | static-render |
| image_test.html | ready | yes |  | no |  | static-render |
| innerhtml_auto_test.html | ready | yes |  | yes |  | blocked(T12) |
| innerhtml_baseline.html | ready | yes |  | no |  | static-render |
| innerhtml_original.html | ready | yes |  | no |  | static-render |
| innerhtml_simple_test.html | ready | yes |  | yes |  | direct |
| innerhtml_test.html | ready | yes |  | yes |  | blocked(T12) |
| input_simple_test.html | ready | yes |  | no |  | static-render |
| outline_test.html | ready | yes |  | no |  | static-render |
| queryselector_complex_test.html | ready | yes |  | yes |  | pending-review |
| skin_test.html | dropped | yes | skin/theme experiment | yes |  | blocked(T20) |
| stylesheets_deleterule_test.html | ready | yes |  | yes |  | pending-review |
| stylesheets_insertrule_test.html | ready | yes |  | yes |  | pending-review |
| tabindex_test.html | ready | yes |  | yes |  | blocked(T20) |
| test_anchor_default_style.html | ready | yes |  | yes |  | direct |
| test_anchor_navigation.html.html | ready | yes |  | yes |  | direct |
| test_anon_wrapper_dom_iter_align.html | ready | yes |  | no |  | static-render |
| test_appearance.html | ready | yes |  | yes |  | direct |
| test_appearance_none.html | ready | yes |  | no |  | static-render |
| test_aspect_ratio_flex.html | ready | yes |  | no |  | static-render |
| test_aspect_ratio_min_max.html | ready | yes |  | no |  | static-render |
| test_aspect_ratio_width_auto_height.html | ready | yes |  | no |  | static-render |
| test_background_position_four_value.html | ready | yes |  | no |  | static-render |
| test_bold_dynamic.html | ready | yes |  | yes | t20_bold_dynamic |  |
| test_bold_layout_debug.html | ready | yes |  | yes | t20_bold_layout_debug |  |
| test_bold_prebaked.html | ready | yes |  | no |  | static-render |
| test_bold_whitespace.html | ready | yes |  | no |  | static-render |
| test_border_collapse.html | ready | yes |  | no |  | static-render |
| test_border_radius_percent.html | ready | yes |  | yes |  | direct |
| test_border_spacing.html | ready | yes |  | no |  | static-render |
| test_border_width_keywords.html | ready | yes |  | no |  | static-render |
| test_break_spaces_key_features.html | ready | yes |  | yes |  | direct |
| test_ce_debug.html | ready | yes |  | yes |  | blocked(T20) |
| test_ce_enter_offset.html | ready | yes |  | yes |  | blocked(T20) |
| test_ce_interactive_sim.html | ready | yes |  | yes | t20_ce_interactive_sim |  |
| test_ce_mixed_multiline.html | ready | yes |  | yes |  | blocked(T20) |
| test_ce_simulate.html | ready | yes |  | yes | t20_ce_simulate |  |
| test_checkbox_toggle.html | ready | yes |  | yes | test_checkbox_toggle | direct |
| test_class_demo.html | ready | yes |  | no | test_class_demo | static-render |
| test_clipboard_api.html | ready | yes |  | yes |  | blocked(T20) |
| test_color_mix.html | ready | yes |  | no |  | static-render |
| test_colr_emoji.html | ready | yes |  | no |  | static-render |
| test_colr_v0_emoji.html | ready | yes |  | no |  | static-render |
| test_conic_basic.html | ready | yes |  | no |  | static-render |
| test_conic_from_at.html | ready | yes |  | no |  | static-render |
| test_contenteditable_auto.html | ready | yes |  | yes | t20_contenteditable_auto |  |
| test_contenteditable_basic.html | ready | yes |  | yes |  | blocked(T20) |
| test_contenteditable_bold_auto.html | ready | yes |  | yes | t20_contenteditable_bold_auto |  |
| test_contenteditable_features.html | ready | yes |  | yes | t20_contenteditable_features |  |
| test_contenteditable_prebolded.html | ready | yes |  | yes |  | blocked(T20) |
| test_contenteditable_typing.html | ready | yes |  | yes |  | blocked(T20) |
| test_contextmenu.html | ready | yes |  | yes |  | direct |
| test_cooldown_circular.html | ready | yes |  | no |  | static-render |
| test_counter_basic.html | ready | yes |  | no |  | static-render |
| test_counter_reset_increment.html | ready | yes |  | no |  | static-render |
| test_counter_styles.html | ready | yes |  | no |  | static-render |
| test_counters_nested.html | ready | yes |  | no |  | static-render |
| test_css_accent_color.html | ready | yes |  | no |  | static-render |
| test_css_caption_side.html | ready | yes |  | no |  | static-render |
| test_css_caption_side_simple.html | ready | yes |  | no |  | static-render |
| test_css_color_scheme.html | ready | yes |  | no |  | static-render |
| test_css_color_scheme_controls_states.html | ready | yes |  | no |  | static-render |
| test_css_color_scheme_descendant_override.html | ready | yes |  | no |  | static-render |
| test_css_counters_quotes.html | ready | yes |  | no |  | static-render |
| test_css_global_keywords.html | ready | yes |  | yes |  | direct |
| test_css_hyphens_soft_hyphen.html | ready | yes |  | no |  | static-render |
| test_css_image_rendering.html | ready | yes |  | no |  | static-render |
| test_css_inheritance.html | ready | yes |  | yes |  | direct |
| test_css_layer_anonymous.html | ready | yes |  | no |  | static-render |
| test_css_layer_basic.html | ready | yes |  | no |  | static-render |
| test_css_layer_nested.html | ready | yes |  | no |  | static-render |
| test_css_layer_priority.html | ready | yes |  | no |  | static-render |
| test_css_layer_specificity.html | ready | yes |  | no |  | static-render |
| test_css_resize_textarea.html | ready | yes |  | no |  | static-render |
| test_css_supports.html | ready | yes |  | yes |  | direct |
| test_css_will_change.html | ready | yes |  | no |  | static-render |
| test_cssom_csstext.html | ready | yes |  | yes |  | direct |
| test_cssom_matchmedia.html | ready | yes |  | yes |  | blocked |
| test_cssom_methods.html | ready | yes |  | yes |  | direct |
| test_cssom_supports.html | ready | yes |  | yes |  | blocked |
| test_csstext_cssom.html | ready | yes |  | yes |  | direct |
| test_details_element.html | ready | yes |  | yes |  | direct |
| test_details_summary_basic.html | ready | yes |  | no |  | static-render |
| test_details_toggle_event.html | ready | yes |  | yes |  | direct |
| test_dialog_backdrop.html | ready | yes |  | yes |  | direct |
| test_dialog_modal.html | ready | yes |  | yes |  | blocked(T12) |
| test_dialog_show.html | ready | yes |  | yes |  | blocked(T12) |
| test_dir_attribute.html | ready | yes |  | no |  | static-render |
| test_dir_pseudo_class.html | ready | yes |  | no |  | static-render |
| test_disabled.html | ready | yes |  | yes | t12_test_disabled | blocked(T20) |
| test_display_contents_events.html | ready | yes |  | no |  | static-render |
| test_display_contents_layout.html | ready | yes |  | no |  | static-render |
| test_display_contents_pseudo.html | ready | yes |  | no |  | static-render |
| test_document_methods.html | ready | yes |  | yes | t12_test_document_methods | blocked(T20) |
| test_document_props.html | ready | yes |  | yes | t12_test_document_props | blocked(T20) |
| test_document_write.html | ready | yes |  | yes |  | direct |
| test_dom_content_loaded.html | ready | yes |  | yes | test_dom_content_loaded | direct |
| test_dom_dataset.html | ready | yes |  | yes |  | direct |
| test_dom_element_interface.html | ready | yes |  | yes |  | direct |
| test_dom_manipulation.html | ready | yes |  | yes | t12_test_dom_manipulation | direct |
| test_dom_node_interface.html | ready | yes |  | yes |  | direct |
| test_domparser_api.html | ready | yes |  | yes |  | blocked(T20) |
| test_domrect_api.html | ready | yes |  | yes |  | direct |
| test_drag_drop_basic.html | ready | yes |  | yes |  | direct |
| test_element_children.html | ready | yes |  | yes |  | direct |
| test_element_click_and_capture.html | ready | yes |  | yes |  | blocked(T20) |
| test_env_function.html | ready | yes |  | yes |  | direct |
| test_event_beforeinput.html | ready | yes |  | yes |  | direct |
| test_event_clipboard.html | ready | yes |  | yes |  | blocked(T20) |
| test_event_currenttarget.html | ready | yes |  | yes |  | blocked(T20) |
| test_event_dispatch_semantics.html | ready | yes |  | yes |  | direct |
| test_event_features.html | ready | yes |  | yes |  | blocked(T20) |
| test_event_features_p2.html | ready | yes |  | yes |  | blocked(T20) |
| test_event_istrusted.html | ready | yes |  | yes | t12_test_event_istrusted | direct |
| test_event_keyboard_repeat.html | ready | yes |  | yes |  | direct |
| test_event_mouse_extended.html | ready | yes |  | yes |  | direct |
| test_events_keyboard.html | ready | yes |  | yes |  | direct |
| test_events_mouse_detail.html | ready | yes |  | yes |  | direct |
| test_events_scroll.html | ready | yes |  | yes |  | direct |
| test_external_stylesheet_basic.html | ready | yes |  | no |  | static-render |
| test_external_stylesheet_missing.html | ready | yes |  | no |  | static-render |
| test_external_stylesheet_order.html | ready | yes |  | no |  | static-render |
| test_fetch_json_basic.html | ready | yes |  | no | t10_fetch_json | static-render |
| test_fetch_reject.html | ready | yes |  | no | t10_fetch_reject | static-render |
| test_fetch_text_basic.html | ready | yes |  | no | t10_fetch_text | static-render |
| test_fetch_timer_order.html | dropped | yes | Promise microtask ordering | yes |  | blocked(T19) |
| test_flex_basis_content.html | ready | yes |  | no |  | static-render |
| test_flow_root_margin_collapse.html | ready | yes |  | no |  | static-render |
| test_focus_related_target.html | ready | yes |  | yes |  | direct |
| test_font_size_keywords.html | ready | yes |  | no |  | static-render |
| test_font_weight.html | ready | yes |  | no |  | static-render |
| test_font_weight_100_900.html | ready | yes |  | no |  | static-render |
| test_font_weight_closest_match.html | ready | yes |  | no |  | static-render |
| test_font_weight_mapping.html | ready | yes |  | yes |  | blocked(T20) |
| test_form_element_bindings.html | ready | yes |  | yes |  | direct |
| test_form_validation_comprehensive.html | ready | yes |  | yes | t12_test_form_validation_comprehensive | direct |
| test_form_validation_pattern.html | ready | yes |  | yes |  | direct |
| test_form_validation_range.html | ready | yes |  | yes |  | blocked(T20) |
| test_formdata_api.html | dropped | yes | FormData object API per T20 | yes |  | blocked(T20) |
| test_formdata_name_serialization.html | ready | yes |  | yes |  | blocked(T20) |
| test_gamepad_full_flow.html | dropped | yes | gamepad platform API | yes |  | pending-review |
| test_gap_dual_value.html | ready | yes |  | no |  | static-render |
| test_grid_basic.html | ready | yes |  | no |  | static-render |
| test_hidden_attr.html | ready | yes |  | yes | test_hidden_attr | direct |
| test_host_view.html | ready | yes |  | no |  | static-render |
| test_html_element_props.html | ready | yes |  | yes | t12_test_html_element_props | blocked(T20) |
| test_hyphens_all_values.html | ready | yes |  | no |  | static-render |
| test_hyphens_manual.html | ready | yes |  | no |  | static-render |
| test_image_rendering.html | ready | yes |  | yes |  | direct |
| test_ime_composition.html | ready | yes |  | yes |  | blocked(T20) |
| test_img_alt_text.html | ready | yes |  | yes |  | pending-review |
| test_img_events_minimal.html | ready | yes |  | yes |  | direct |
| test_img_events_simple.html | ready | yes |  | yes |  | blocked(T20) |
| test_img_load_error.html | ready | yes |  | yes |  | blocked(T20) |
| test_important.html | ready | yes |  | no |  | static-render |
| test_important_simple.html | ready | yes |  | no |  | static-render |
| test_inert_attribute.html | ready | yes |  | yes |  | blocked(T12) |
| test_inline_font_weight.html | ready | yes |  | no |  | static-render |
| test_input_caret.html | ready | yes |  | yes |  | direct |
| test_input_element_bindings.html | ready | yes |  | yes |  | direct |
| test_input_maxlength.html | ready | yes |  | yes |  | direct |
| test_input_properties.html | ready | yes |  | yes |  | direct |
| test_input_readonly.html | ready | yes |  | yes |  | direct |
| test_input_type.html | ready | yes |  | yes |  | direct |
| test_input_value.html | ready | yes |  | no | test_input_value | static-render |
| test_js_dom_bindings_p2.html | ready | yes |  | yes |  | direct |
| test_keyboard_repeat.html | ready | yes |  | yes |  | direct |
| test_label_for.html | ready | yes |  | yes |  | direct |
| test_lang_attribute.html | ready | yes |  | yes |  | direct |
| test_lang_turkish_i_dot.html | ready | yes |  | yes |  | direct |
| test_light_dark_function.html | ready | yes |  | yes |  | direct |
| test_list_markers_basic.html | ready | yes |  | no |  | static-render |
| test_list_nested.html | ready | yes |  | no |  | static-render |
| test_list_position.html | ready | yes |  | no |  | static-render |
| test_list_style_parsing.html | ready | yes |  | no |  | static-render |
| test_list_style_shorthand.html | ready | yes |  | no |  | static-render |
| test_list_style_types.html | ready | yes |  | no |  | static-render |
| test_logical_properties.html | ready | yes |  | no |  | static-render |
| test_logical_properties_basic.html | ready | yes |  | no |  | static-render |
| test_marker_pseudo.html | ready | yes |  | no |  | static-render |
| test_mask_conic_cooldown.html | ready | yes |  | no |  | static-render |
| test_mask_general.html | ready | yes |  | no |  | static-render |
| test_matchmedia.html | ready | yes |  | yes |  | direct |
| test_max_height_overflow_auto.html | ready | yes |  | yes |  | direct |
| test_max_height_overflow_auto_inline.html | ready | yes |  | yes |  | direct |
| test_mouse_event_properties.html | ready | yes |  | yes |  | direct |
| test_mutation_observer.html | ready | yes |  | yes |  | direct |
| test_nineslice_basic.html | ready | yes |  | no |  | static-render |
| test_node_sibling.html | ready | yes |  | yes |  | direct |
| test_object_position.html | ready | yes |  | no |  | static-render |
| test_offscreen_readback_smoke.html | ready | yes |  | no |  | static-render |
| test_oklab_color.html | ready | yes |  | no |  | static-render |
| test_oklch_color.html | ready | yes |  | no |  | static-render |
| test_ol_start.html | ready | yes |  | no |  | static-render |
| test_opacity_percent.html | ready | yes |  | no |  | static-render |
| test_optgroup_basic.html | ready | yes |  | yes |  | blocked(T12) |
| test_overflow_clip.html | ready | yes |  | yes |  | direct |
| test_overscroll_behavior_axis.html | ready | yes |  | no |  | static-render |
| test_overscroll_behavior_contain.html | ready | yes |  | no |  | static-render |
| test_overscroll_behavior_none.html | ready | yes |  | no |  | static-render |
| test_p0_comprehensive.html | dropped | yes | comprehensive demo page | yes |  | pending-review |
| test_partial_repaint.html | ready | yes |  | yes |  | direct |
| test_performance_now.html | ready | yes |  | yes | t12_test_performance_now | blocked(T20) |
| test_place_shorthand.html | ready | yes |  | no |  | static-render |
| test_placeholder_pseudo.html | ready | yes |  | no |  | static-render |
| test_pointer_properties.html | ready | yes |  | yes |  | blocked(T20) |
| test_porffor_clipboard_cn.html | ready | yes |  | yes | hello_dom | blocked(T20) |
| test_porffor_greeting.html | ready | yes |  | no | test_porffor_greeting | static-render |
| test_porffor_mf_class.html | ready | yes |  | no | test_mf_class | static-render |
| test_porffor_mf_style.html | ready | yes |  | no | test_mf_style | static-render |
| test_porffor_mf_text.html | ready | yes |  | no | test_mf_text | static-render |
| test_position_fixed.html | ready | yes |  | no |  | static-render |
| test_pseudo_classes_new.html | ready | yes |  | yes |  | direct |
| test_quotes_basic.html | ready | yes |  | no |  | static-render |
| test_quotes_lang.html | ready | yes |  | no |  | static-render |
| test_quotes_nested.html | ready | yes |  | no |  | static-render |
| test_scene_compiler.html | ready | yes |  | yes |  | blocked(T20) |
| test_scene_graph.html | dropped | yes | dong.scene API | yes |  | blocked(T20) |
| test_scroll_autobottom_blocks.html | ready | yes |  | yes |  | blocked(T20) |
| test_scroll_behavior_auto_to_bottom.html | ready | yes |  | yes |  | direct |
| test_scroll_behavior_smooth.html | ready | yes |  | yes | t12_test_scroll_behavior_smooth | direct |
| test_scroll_behavior_smooth_autoscroll_auto_bottom.html | ready | yes |  | yes | t12_test_scroll_behavior_smooth_autoscroll_auto_bottom | blocked(T20) |
| test_scroll_container_mid_scroll.html | ready | yes |  | yes |  | direct |
| test_scroll_properties.html | ready | yes |  | yes | t12_test_scroll_properties | direct |
| test_scroll_scrollheight_margin_bottom.html | ready | yes |  | yes |  | direct |
| test_scroll_to_section2_metrics.html | ready | yes |  | yes |  | direct |
| test_scroll_top_clamp_scrollheight.html | ready | yes |  | yes |  | blocked(T20) |
| test_select_basic.html | ready | yes |  | yes | t12_test_select_basic | direct |
| test_select_block_flow_layout.html | ready | yes |  | no |  | static-render |
| test_select_dropdown_overlay.html | ready | yes |  | no |  | static-render |
| test_select_dropdown_scroll.html | ready | yes |  | no |  | static-render |
| test_select_event_log_growth.html | ready | yes |  | yes |  | direct |
| test_select_keyboard.html | ready | yes |  | yes |  | direct |
| test_select_option_click.html | ready | yes |  | yes |  | blocked(T20) |
| test_select_programmatic_setters.html | ready | yes |  | yes |  | direct |
| test_select_selected_attr_sync.html | ready | yes |  | yes |  | direct |
| test_selection_api.html | ready | yes |  | yes |  | blocked(T20) |
| test_selection_mouse.html | ready | yes |  | yes |  | direct |
| test_selection_pseudo.html | ready | yes |  | yes |  | direct |
| test_simple_bindings.html | ready | yes |  | yes |  | direct |
| test_spatial_nav_explicit.html | ready | yes |  | yes |  | direct |
| test_spatial_nav_grid.html | ready | yes |  | yes |  | direct |
| test_sticky_nested.html | ready | yes |  | no |  | static-render |
| test_sticky_parent_clamp.html | ready | yes |  | no |  | static-render |
| test_sticky_scroll_bottom.html | ready | yes |  | no |  | static-render |
| test_sticky_scroll_top.html | ready | yes |  | no |  | static-render |
| test_sticky_scroll_top_autoscroll.html | ready | yes |  | yes |  | blocked(T20) |
| test_structuredclone.html | ready | yes |  | yes |  | direct |
| test_style_render.html | ready | yes |  | no |  | static-render |
| test_submit_event.html | ready | yes |  | yes | t12_test_submit_event | blocked(T20) |
| test_tab_size.html | ready | yes |  | yes |  | direct |
| test_tabindex.html | ready | yes |  | yes | t12_test_tabindex | direct |
| test_table_layout.html | ready | yes |  | no |  | static-render |
| test_text_overflow_ellipsis.html | ready | yes |  | no |  | static-render |
| test_text_overlap_fix.html | ready | yes |  | no |  | static-render |
| test_textarea_element_bindings.html | ready | yes |  | yes |  | direct |
| test_textarea_multiline.html | ready | yes |  | yes |  | direct |
| test_textarea_rows_cols.html | ready | yes |  | no |  | static-render |
| test_textarea_rows_cols_override.html | ready | yes |  | no |  | static-render |
| test_title_tooltip.html | ready | yes |  | no |  | static-render |
| test_visibility_collapse_non_table.html | ready | yes |  | no |  | static-render |
| test_visibility_collapse_table_row.html | ready | yes |  | no |  | static-render |
| test_visibility_collapse_table_tbody.html | ready | yes |  | no |  | static-render |
| test_white_space_pre_line_break_spaces.html | ready | yes |  | yes |  | direct |
| test_whitespace_prewrap.html | ready | yes |  | yes |  | direct |
| text_decoration_test.html | ready | yes |  | no |  | static-render |
| text_shadow_test.html | ready | yes |  | no |  | static-render |
| text_wrap_test.html | ready | yes |  | no |  | static-render |
| transform_test.html | ready | yes |  | no |  | static-render |

## dropped 候选（待评审）

| 文件 | 建议理由 |
|------|---------|
| complex.html | 综合演示页，非回归粒度 |
| skin_test.html | 皮肤/主题实验，非引擎契约 |

## T14 snippet 交叉引用

见 [T14-snippet-inventory.md](./T14-snippet-inventory.md)（`porffor_snippet_inventory.mjs` 生成）。
