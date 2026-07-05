# T13 — 全量测试迁移盘点

> 自动生成：`node dong/scripts/porffor_test_inventory.mjs`（勿手改统计表，改生成器或 HTML 标记后重跑）

## 摘要

| 指标 | 值 |
|------|-----|
| 测试总数 | 286 |
| ready | 138 (48.3%) |
| pending | 148 |
| blocked | 0 |
| dropped | 0 |
| 无 script（静态渲染） | 135 |
| 有 script | 151 |

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
| static-render | 135 |
| direct | 80 |
| blocked(T20) | 40 |
| blocked(T12) | 21 |
| pending-review | 6 |
| blocked | 2 |
| T14-snippet | 1 |
| blocked(T19) | 1 |

## 全量清单

| 文件 | porffor 状态 | 显式标记 | 阻塞原因 | script | 模块 | 迁移分类 |
|------|-------------|---------|---------|--------|------|---------|
| background_attachment_fixed_test.html | ready | no |  | no |  | static-render |
| background_clip_test.html | ready | no |  | no |  | static-render |
| background_origin_test.html | ready | no |  | no |  | static-render |
| chinese_font_large_test.html | ready | no |  | no |  | static-render |
| chinese_font_medium_test.html | ready | no |  | no |  | static-render |
| chinese_font_sizes_test.html | ready | no |  | no |  | static-render |
| chinese_font_small_test.html | ready | no |  | no |  | static-render |
| chinese_font_tiny_test.html | ready | no |  | no |  | static-render |
| complex.html | ready | no |  | no |  | static-render |
| css_global_keywords_comprehensive.html | ready | no |  | no |  | static-render |
| css_global_keywords_inherit.html | ready | no |  | no |  | static-render |
| css_global_keywords_initial.html | ready | no |  | no |  | static-render |
| css_global_keywords_unset.html | ready | no |  | no |  | static-render |
| cursor_test.html | ready | no |  | no |  | static-render |
| focus_visible_test.html | pending | no |  | yes |  | blocked(T12) |
| font_style_test.html | ready | no |  | no |  | static-render |
| getcomputedstyle_smoke_test.html | pending | no |  | yes |  | direct |
| ifc_test.html | ready | no |  | no |  | static-render |
| image_test.html | ready | no |  | no |  | static-render |
| innerhtml_auto_test.html | pending | no |  | yes |  | blocked(T12) |
| innerhtml_baseline.html | ready | no |  | no |  | static-render |
| innerhtml_original.html | ready | no |  | no |  | static-render |
| innerhtml_simple_test.html | pending | no |  | yes |  | direct |
| innerhtml_test.html | pending | no |  | yes |  | blocked(T12) |
| input_simple_test.html | ready | no |  | no |  | static-render |
| outline_test.html | ready | no |  | no |  | static-render |
| queryselector_complex_test.html | pending | no |  | yes |  | pending-review |
| skin_test.html | pending | no |  | yes |  | blocked(T20) |
| stylesheets_deleterule_test.html | pending | no |  | yes |  | pending-review |
| stylesheets_insertrule_test.html | pending | no |  | yes |  | pending-review |
| tabindex_test.html | pending | no |  | yes |  | blocked(T20) |
| test_anchor_default_style.html | pending | no |  | yes |  | direct |
| test_anchor_navigation.html.html | pending | no |  | yes |  | direct |
| test_anon_wrapper_dom_iter_align.html | ready | no |  | no |  | static-render |
| test_appearance.html | pending | no |  | yes |  | direct |
| test_appearance_none.html | ready | no |  | no |  | static-render |
| test_aspect_ratio_flex.html | ready | no |  | no |  | static-render |
| test_aspect_ratio_min_max.html | ready | no |  | no |  | static-render |
| test_aspect_ratio_width_auto_height.html | ready | no |  | no |  | static-render |
| test_background_position_four_value.html | ready | no |  | no |  | static-render |
| test_bold_dynamic.html | pending | no |  | yes |  | blocked(T20) |
| test_bold_layout_debug.html | pending | no |  | yes |  | blocked(T20) |
| test_bold_prebaked.html | ready | no |  | no |  | static-render |
| test_bold_whitespace.html | ready | no |  | no |  | static-render |
| test_border_collapse.html | ready | no |  | no |  | static-render |
| test_border_radius_percent.html | pending | no |  | yes |  | direct |
| test_border_spacing.html | ready | no |  | no |  | static-render |
| test_border_width_keywords.html | ready | no |  | no |  | static-render |
| test_break_spaces_key_features.html | pending | no |  | yes |  | direct |
| test_ce_debug.html | pending | no |  | yes |  | blocked(T20) |
| test_ce_enter_offset.html | pending | no |  | yes |  | blocked(T20) |
| test_ce_interactive_sim.html | pending | no |  | yes |  | blocked(T20) |
| test_ce_mixed_multiline.html | pending | no |  | yes |  | blocked(T20) |
| test_ce_simulate.html | pending | no |  | yes |  | blocked(T20) |
| test_checkbox_toggle.html | ready | yes |  | yes | test_checkbox_toggle | direct |
| test_class_demo.html | ready | yes |  | no | test_class_demo | static-render |
| test_clipboard_api.html | pending | no |  | yes |  | blocked(T20) |
| test_color_mix.html | ready | no |  | no |  | static-render |
| test_colr_emoji.html | ready | no |  | no |  | static-render |
| test_colr_v0_emoji.html | ready | no |  | no |  | static-render |
| test_conic_basic.html | ready | no |  | no |  | static-render |
| test_conic_from_at.html | ready | no |  | no |  | static-render |
| test_contenteditable_auto.html | pending | no |  | yes |  | blocked(T20) |
| test_contenteditable_basic.html | pending | no |  | yes |  | blocked(T20) |
| test_contenteditable_bold_auto.html | pending | no |  | yes |  | T14-snippet |
| test_contenteditable_features.html | pending | no |  | yes |  | blocked(T20) |
| test_contenteditable_prebolded.html | pending | no |  | yes |  | blocked(T20) |
| test_contenteditable_typing.html | pending | no |  | yes |  | blocked(T20) |
| test_contextmenu.html | pending | no |  | yes |  | direct |
| test_cooldown_circular.html | ready | no |  | no |  | static-render |
| test_counter_basic.html | ready | no |  | no |  | static-render |
| test_counter_reset_increment.html | ready | no |  | no |  | static-render |
| test_counter_styles.html | ready | no |  | no |  | static-render |
| test_counters_nested.html | ready | no |  | no |  | static-render |
| test_css_accent_color.html | ready | no |  | no |  | static-render |
| test_css_caption_side.html | ready | no |  | no |  | static-render |
| test_css_caption_side_simple.html | ready | no |  | no |  | static-render |
| test_css_color_scheme.html | ready | no |  | no |  | static-render |
| test_css_color_scheme_controls_states.html | ready | no |  | no |  | static-render |
| test_css_color_scheme_descendant_override.html | ready | no |  | no |  | static-render |
| test_css_counters_quotes.html | ready | no |  | no |  | static-render |
| test_css_global_keywords.html | pending | no |  | yes |  | direct |
| test_css_hyphens_soft_hyphen.html | ready | no |  | no |  | static-render |
| test_css_image_rendering.html | ready | no |  | no |  | static-render |
| test_css_inheritance.html | pending | no |  | yes |  | direct |
| test_css_layer_anonymous.html | ready | no |  | no |  | static-render |
| test_css_layer_basic.html | ready | no |  | no |  | static-render |
| test_css_layer_nested.html | ready | no |  | no |  | static-render |
| test_css_layer_priority.html | ready | no |  | no |  | static-render |
| test_css_layer_specificity.html | ready | no |  | no |  | static-render |
| test_css_resize_textarea.html | ready | no |  | no |  | static-render |
| test_css_supports.html | pending | no |  | yes |  | direct |
| test_css_will_change.html | ready | no |  | no |  | static-render |
| test_cssom_csstext.html | pending | no |  | yes |  | direct |
| test_cssom_matchmedia.html | pending | no |  | yes |  | blocked |
| test_cssom_methods.html | pending | no |  | yes |  | direct |
| test_cssom_supports.html | pending | no |  | yes |  | blocked |
| test_csstext_cssom.html | pending | no |  | yes |  | direct |
| test_details_element.html | pending | no |  | yes |  | direct |
| test_details_summary_basic.html | ready | no |  | no |  | static-render |
| test_details_toggle_event.html | pending | no |  | yes |  | direct |
| test_dialog_backdrop.html | pending | no |  | yes |  | direct |
| test_dialog_modal.html | pending | no |  | yes |  | blocked(T12) |
| test_dialog_show.html | pending | no |  | yes |  | blocked(T12) |
| test_dir_attribute.html | ready | no |  | no |  | static-render |
| test_dir_pseudo_class.html | ready | no |  | no |  | static-render |
| test_disabled.html | pending | no |  | yes |  | blocked(T12) |
| test_display_contents_events.html | ready | no |  | no |  | static-render |
| test_display_contents_layout.html | ready | no |  | no |  | static-render |
| test_display_contents_pseudo.html | ready | no |  | no |  | static-render |
| test_document_methods.html | pending | no |  | yes |  | blocked(T12) |
| test_document_props.html | pending | no |  | yes |  | blocked(T12) |
| test_document_write.html | pending | no |  | yes |  | direct |
| test_dom_content_loaded.html | ready | yes |  | yes | test_dom_content_loaded | direct |
| test_dom_dataset.html | pending | no |  | yes |  | direct |
| test_dom_element_interface.html | pending | no |  | yes |  | direct |
| test_dom_manipulation.html | pending | no |  | yes |  | blocked(T12) |
| test_dom_node_interface.html | pending | no |  | yes |  | direct |
| test_domparser_api.html | pending | no |  | yes |  | blocked(T20) |
| test_domrect_api.html | pending | no |  | yes |  | direct |
| test_drag_drop_basic.html | pending | no |  | yes |  | direct |
| test_element_children.html | pending | no |  | yes |  | direct |
| test_element_click_and_capture.html | pending | no |  | yes |  | blocked(T20) |
| test_env_function.html | pending | no |  | yes |  | direct |
| test_event_beforeinput.html | pending | no |  | yes |  | direct |
| test_event_clipboard.html | pending | no |  | yes |  | blocked(T20) |
| test_event_currenttarget.html | pending | no |  | yes |  | blocked(T20) |
| test_event_dispatch_semantics.html | pending | no |  | yes |  | direct |
| test_event_features.html | pending | no |  | yes |  | blocked(T20) |
| test_event_features_p2.html | pending | no |  | yes |  | blocked(T20) |
| test_event_istrusted.html | pending | no |  | yes |  | blocked(T12) |
| test_event_keyboard_repeat.html | pending | no |  | yes |  | direct |
| test_event_mouse_extended.html | pending | no |  | yes |  | direct |
| test_events_keyboard.html | pending | no |  | yes |  | direct |
| test_events_mouse_detail.html | pending | no |  | yes |  | direct |
| test_events_scroll.html | pending | no |  | yes |  | direct |
| test_external_stylesheet_basic.html | ready | no |  | no |  | static-render |
| test_external_stylesheet_missing.html | ready | no |  | no |  | static-render |
| test_external_stylesheet_order.html | ready | no |  | no |  | static-render |
| test_fetch_json_basic.html | pending | no |  | yes |  | blocked(T20) |
| test_fetch_reject.html | pending | no |  | yes |  | blocked(T20) |
| test_fetch_text_basic.html | pending | no |  | yes |  | blocked(T20) |
| test_fetch_timer_order.html | pending | no |  | yes |  | blocked(T19) |
| test_flex_basis_content.html | ready | no |  | no |  | static-render |
| test_flow_root_margin_collapse.html | ready | no |  | no |  | static-render |
| test_focus_related_target.html | pending | no |  | yes |  | direct |
| test_font_size_keywords.html | ready | no |  | no |  | static-render |
| test_font_weight.html | ready | no |  | no |  | static-render |
| test_font_weight_100_900.html | ready | no |  | no |  | static-render |
| test_font_weight_closest_match.html | ready | no |  | no |  | static-render |
| test_font_weight_mapping.html | pending | no |  | yes |  | blocked(T20) |
| test_form_element_bindings.html | pending | no |  | yes |  | direct |
| test_form_validation_comprehensive.html | pending | no |  | yes |  | blocked(T12) |
| test_form_validation_pattern.html | pending | no |  | yes |  | direct |
| test_form_validation_range.html | pending | no |  | yes |  | blocked(T20) |
| test_formdata_api.html | pending | no |  | yes |  | blocked(T20) |
| test_formdata_name_serialization.html | pending | no |  | yes |  | blocked(T20) |
| test_gamepad_full_flow.html | pending | no |  | yes |  | pending-review |
| test_gap_dual_value.html | ready | no |  | no |  | static-render |
| test_grid_basic.html | ready | no |  | no |  | static-render |
| test_hidden_attr.html | ready | yes |  | yes | test_hidden_attr | direct |
| test_host_view.html | ready | no |  | no |  | static-render |
| test_html_element_props.html | pending | no |  | yes |  | blocked(T12) |
| test_hyphens_all_values.html | ready | no |  | no |  | static-render |
| test_hyphens_manual.html | ready | no |  | no |  | static-render |
| test_image_rendering.html | pending | no |  | yes |  | direct |
| test_ime_composition.html | pending | no |  | yes |  | blocked(T20) |
| test_img_alt_text.html | pending | no |  | yes |  | pending-review |
| test_img_events_minimal.html | pending | no |  | yes |  | direct |
| test_img_events_simple.html | pending | no |  | yes |  | blocked(T20) |
| test_img_load_error.html | pending | no |  | yes |  | blocked(T20) |
| test_important.html | ready | no |  | no |  | static-render |
| test_important_simple.html | ready | no |  | no |  | static-render |
| test_inert_attribute.html | pending | no |  | yes |  | blocked(T12) |
| test_inline_font_weight.html | ready | no |  | no |  | static-render |
| test_input_caret.html | pending | no |  | yes |  | direct |
| test_input_element_bindings.html | pending | no |  | yes |  | direct |
| test_input_maxlength.html | pending | no |  | yes |  | direct |
| test_input_properties.html | pending | no |  | yes |  | direct |
| test_input_readonly.html | pending | no |  | yes |  | direct |
| test_input_type.html | pending | no |  | yes |  | direct |
| test_input_value.html | ready | yes |  | no | test_input_value | static-render |
| test_js_dom_bindings_p2.html | pending | no |  | yes |  | direct |
| test_keyboard_repeat.html | pending | no |  | yes |  | direct |
| test_label_for.html | pending | no |  | yes |  | direct |
| test_lang_attribute.html | pending | no |  | yes |  | direct |
| test_lang_turkish_i_dot.html | pending | no |  | yes |  | direct |
| test_light_dark_function.html | pending | no |  | yes |  | direct |
| test_list_markers_basic.html | ready | yes |  | no |  | static-render |
| test_list_nested.html | ready | no |  | no |  | static-render |
| test_list_position.html | ready | no |  | no |  | static-render |
| test_list_style_parsing.html | ready | no |  | no |  | static-render |
| test_list_style_shorthand.html | ready | no |  | no |  | static-render |
| test_list_style_types.html | ready | no |  | no |  | static-render |
| test_logical_properties.html | ready | no |  | no |  | static-render |
| test_logical_properties_basic.html | ready | no |  | no |  | static-render |
| test_marker_pseudo.html | ready | no |  | no |  | static-render |
| test_mask_conic_cooldown.html | ready | no |  | no |  | static-render |
| test_mask_general.html | ready | no |  | no |  | static-render |
| test_matchmedia.html | pending | no |  | yes |  | direct |
| test_max_height_overflow_auto.html | pending | no |  | yes |  | direct |
| test_max_height_overflow_auto_inline.html | pending | no |  | yes |  | direct |
| test_mouse_event_properties.html | pending | no |  | yes |  | direct |
| test_mutation_observer.html | pending | no |  | yes |  | direct |
| test_nineslice_basic.html | ready | no |  | no |  | static-render |
| test_node_sibling.html | pending | no |  | yes |  | direct |
| test_object_position.html | ready | no |  | no |  | static-render |
| test_offscreen_readback_smoke.html | ready | no |  | no |  | static-render |
| test_oklab_color.html | ready | no |  | no |  | static-render |
| test_oklch_color.html | ready | no |  | no |  | static-render |
| test_ol_start.html | ready | no |  | no |  | static-render |
| test_opacity_percent.html | ready | no |  | no |  | static-render |
| test_optgroup_basic.html | pending | no |  | yes |  | blocked(T12) |
| test_overflow_clip.html | pending | no |  | yes |  | direct |
| test_overscroll_behavior_axis.html | ready | no |  | no |  | static-render |
| test_overscroll_behavior_contain.html | ready | no |  | no |  | static-render |
| test_overscroll_behavior_none.html | ready | no |  | no |  | static-render |
| test_p0_comprehensive.html | pending | no |  | yes |  | pending-review |
| test_partial_repaint.html | pending | no |  | yes |  | direct |
| test_performance_now.html | pending | no |  | yes |  | blocked(T12) |
| test_place_shorthand.html | ready | no |  | no |  | static-render |
| test_placeholder_pseudo.html | ready | no |  | no |  | static-render |
| test_pointer_properties.html | pending | no |  | yes |  | blocked(T20) |
| test_porffor_clipboard_cn.html | pending | no |  | yes | hello_dom | blocked(T20) |
| test_porffor_greeting.html | ready | yes |  | no | test_porffor_greeting | static-render |
| test_porffor_mf_class.html | ready | yes |  | no | test_mf_class | static-render |
| test_porffor_mf_style.html | ready | yes |  | no | test_mf_style | static-render |
| test_porffor_mf_text.html | ready | yes |  | no | test_mf_text | static-render |
| test_position_fixed.html | ready | no |  | no |  | static-render |
| test_pseudo_classes_new.html | pending | no |  | yes |  | direct |
| test_quotes_basic.html | ready | no |  | no |  | static-render |
| test_quotes_lang.html | ready | no |  | no |  | static-render |
| test_quotes_nested.html | ready | no |  | no |  | static-render |
| test_scene_compiler.html | pending | no |  | yes |  | blocked(T20) |
| test_scene_graph.html | pending | no |  | yes |  | blocked(T20) |
| test_scroll_autobottom_blocks.html | pending | no |  | yes |  | blocked(T20) |
| test_scroll_behavior_auto_to_bottom.html | pending | no |  | yes |  | direct |
| test_scroll_behavior_smooth.html | pending | no |  | yes |  | blocked(T12) |
| test_scroll_behavior_smooth_autoscroll_auto_bottom.html | pending | no |  | yes |  | blocked(T12) |
| test_scroll_container_mid_scroll.html | pending | no |  | yes |  | direct |
| test_scroll_properties.html | pending | no |  | yes |  | blocked(T12) |
| test_scroll_scrollheight_margin_bottom.html | pending | no |  | yes |  | direct |
| test_scroll_to_section2_metrics.html | pending | no |  | yes |  | direct |
| test_scroll_top_clamp_scrollheight.html | pending | no |  | yes |  | blocked(T20) |
| test_select_basic.html | pending | no |  | yes |  | blocked(T12) |
| test_select_block_flow_layout.html | ready | no |  | no |  | static-render |
| test_select_dropdown_overlay.html | ready | no |  | no |  | static-render |
| test_select_dropdown_scroll.html | ready | no |  | no |  | static-render |
| test_select_event_log_growth.html | pending | no |  | yes |  | direct |
| test_select_keyboard.html | pending | no |  | yes |  | direct |
| test_select_option_click.html | pending | no |  | yes |  | blocked(T20) |
| test_select_programmatic_setters.html | pending | no |  | yes |  | direct |
| test_select_selected_attr_sync.html | pending | no |  | yes |  | direct |
| test_selection_api.html | pending | no |  | yes |  | blocked(T20) |
| test_selection_mouse.html | pending | no |  | yes |  | direct |
| test_selection_pseudo.html | pending | no |  | yes |  | direct |
| test_simple_bindings.html | pending | no |  | yes |  | direct |
| test_spatial_nav_explicit.html | pending | no |  | yes |  | direct |
| test_spatial_nav_grid.html | pending | no |  | yes |  | direct |
| test_sticky_nested.html | ready | no |  | no |  | static-render |
| test_sticky_parent_clamp.html | ready | no |  | no |  | static-render |
| test_sticky_scroll_bottom.html | ready | no |  | no |  | static-render |
| test_sticky_scroll_top.html | ready | no |  | no |  | static-render |
| test_sticky_scroll_top_autoscroll.html | pending | no |  | yes |  | blocked(T20) |
| test_structuredclone.html | pending | no |  | yes |  | direct |
| test_style_render.html | ready | no |  | no |  | static-render |
| test_submit_event.html | pending | no |  | yes |  | blocked(T12) |
| test_tab_size.html | pending | no |  | yes |  | direct |
| test_tabindex.html | pending | no |  | yes |  | blocked(T12) |
| test_table_layout.html | ready | no |  | no |  | static-render |
| test_text_overflow_ellipsis.html | ready | no |  | no |  | static-render |
| test_text_overlap_fix.html | ready | no |  | no |  | static-render |
| test_textarea_element_bindings.html | pending | no |  | yes |  | direct |
| test_textarea_multiline.html | pending | no |  | yes |  | direct |
| test_textarea_rows_cols.html | ready | no |  | no |  | static-render |
| test_textarea_rows_cols_override.html | ready | no |  | no |  | static-render |
| test_title_tooltip.html | ready | yes |  | no |  | static-render |
| test_visibility_collapse_non_table.html | ready | no |  | no |  | static-render |
| test_visibility_collapse_table_row.html | ready | no |  | no |  | static-render |
| test_visibility_collapse_table_tbody.html | ready | no |  | no |  | static-render |
| test_white_space_pre_line_break_spaces.html | pending | no |  | yes |  | direct |
| test_whitespace_prewrap.html | pending | no |  | yes |  | direct |
| text_decoration_test.html | ready | no |  | no |  | static-render |
| text_shadow_test.html | ready | no |  | no |  | static-render |
| text_wrap_test.html | ready | no |  | no |  | static-render |
| transform_test.html | ready | no |  | no |  | static-render |

## dropped 候选（待评审）

| 文件 | 建议理由 |
|------|---------|
| complex.html | 综合演示页，非回归粒度 |
| skin_test.html | 皮肤/主题实验，非引擎契约 |

## T14 snippet 交叉引用

见 [T14-snippet-inventory.md](./T14-snippet-inventory.md)（`porffor_snippet_inventory.mjs` 生成）。
