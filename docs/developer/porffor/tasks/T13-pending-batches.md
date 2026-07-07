# T13 — Pending 分批处理（自动生成）

> `node dong/scripts/porffor_batch_tag_blocked.mjs` 后生成；与 `porffor_blocked_reasons.mjs` 同步。

## 摘要

| 指标 | 值 |
|------|-----|
| 本次处理 | 71 |
| 写入标记 | 71 |
| 跳过 | 215 |

## 波次（按阻塞任务）

处理顺序建议：**T12 内联提取** → **T14 多帧/snippet** → **T06 字符串/DOM 解析** → **T10 fetch 模块** → **T11 平台 API** → **T19 Promise** → **T20 长尾** → **dropped**

### blocked(T12)（17）

- focus_visible_test.html
- test_disabled.html
- test_document_methods.html
- test_document_props.html
- test_dom_manipulation.html
- test_event_istrusted.html
- test_form_validation_comprehensive.html
- test_html_element_props.html
- test_inert_attribute.html
- test_optgroup_basic.html
- test_performance_now.html
- test_scroll_behavior_smooth.html
- test_scroll_behavior_smooth_autoscroll_auto_bottom.html
- test_scroll_properties.html
- test_select_basic.html
- test_submit_event.html
- test_tabindex.html

### blocked(T14)（13）

- test_ce_debug.html
- test_ce_enter_offset.html
- test_ce_interactive_sim.html
- test_ce_mixed_multiline.html
- test_ce_simulate.html
- test_contenteditable_auto.html
- test_contenteditable_basic.html
- test_contenteditable_bold_auto.html
- test_contenteditable_features.html
- test_contenteditable_prebolded.html
- test_contenteditable_typing.html
- test_ime_composition.html
- test_selection_api.html

### blocked(T06)（5）

- innerhtml_auto_test.html
- innerhtml_test.html
- test_domparser_api.html
- test_formdata_api.html
- test_formdata_name_serialization.html

### blocked(T10)（3）

- test_fetch_json_basic.html
- test_fetch_reject.html
- test_fetch_text_basic.html

### blocked(T11)（7）

- test_clipboard_api.html
- test_cssom_matchmedia.html
- test_cssom_supports.html
- test_dialog_modal.html
- test_dialog_show.html
- test_event_clipboard.html
- test_porffor_clipboard_cn.html

### blocked(T19)（1）

- test_fetch_timer_order.html

### blocked(T20)（21）

- queryselector_complex_test.html
- stylesheets_deleterule_test.html
- stylesheets_insertrule_test.html
- tabindex_test.html
- test_bold_dynamic.html
- test_bold_layout_debug.html
- test_element_click_and_capture.html
- test_event_currenttarget.html
- test_event_features.html
- test_event_features_p2.html
- test_font_weight_mapping.html
- test_form_validation_range.html
- test_img_events_simple.html
- test_img_load_error.html
- test_pointer_properties.html
- test_scene_compiler.html
- test_scene_graph.html
- test_scroll_autobottom_blocks.html
- test_scroll_top_clamp_scrollheight.html
- test_select_option_click.html
- test_sticky_scroll_top_autoscroll.html

### dropped（3）

- skin_test.html
- test_gamepad_full_flow.html
- test_p0_comprehensive.html

### promoted-ready（1）

- test_img_alt_text.html

## 相关脚本

| 脚本 | 用途 |
|------|------|
| `porffor_batch_tag_blocked.mjs` | 本表：pending → blocked/dropped/ready |
| `porffor_batch_migrate_t12.mjs` | blocked(T12) → T12 内联 handler 提取 |
| `porffor_batch_migrate_t14.mjs` | blocked(T14) → ready 冒烟 / 回标 T20 / mf 占位 |
| `porffor_batch_migrate_direct.mjs` | direct 类 → ready（已完成） |
| `porffor_test_inventory.mjs` | 全量盘点 |
