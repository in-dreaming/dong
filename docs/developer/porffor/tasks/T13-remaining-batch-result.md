# T13 剩余 blocked 批处理结果

> `node dong/scripts/porffor_batch_migrate_remaining.mjs` 生成

## 摘要

| 动作 | 数量 |
|------|------|
| ready | 31 |
| ready + T20 exec 模块 | 7 |
| ready + T10 fetch 模块 | 3 |
| blocked(T20) 保留 | 0 |
| dropped | 3 |
| 跳过 | 242 |

## 明细

| 文件 | 结果 | 说明 |
|------|------|------|
| innerhtml_auto_test.html | ready | layout smoke |
| innerhtml_test.html | ready | layout smoke |
| queryselector_complex_test.html | ready | selector layout (gray items) |
| stylesheets_deleterule_test.html | ready | stylesheet box layout |
| stylesheets_insertrule_test.html | ready | stylesheet box layout |
| tabindex_test.html | ready | tabindex focus order UI |
| test_bold_dynamic.html | ready+exec-module | t20_bold_dynamic |
| test_bold_layout_debug.html | ready+exec-module | t20_bold_layout_debug |
| test_ce_interactive_sim.html | ready+exec-module | t20_ce_interactive_sim |
| test_ce_simulate.html | ready+exec-module | t20_ce_simulate |
| test_clipboard_api.html | ready | clipboard UI layout |
| test_contenteditable_auto.html | ready+exec-module | t20_contenteditable_auto |
| test_contenteditable_bold_auto.html | ready+exec-module | t20_contenteditable_bold_auto |
| test_contenteditable_features.html | ready+exec-module | t20_contenteditable_features |
| test_cssom_matchmedia.html | ready | matchMedia page layout |
| test_cssom_supports.html | ready | CSS.supports page layout |
| test_dialog_modal.html | ready | dialog layout (closed) |
| test_dialog_show.html | ready | dialog layout (closed) |
| test_domparser_api.html | ready | static form layout |
| test_element_click_and_capture.html | ready | capture phase UI (initial) |
| test_event_clipboard.html | ready | clipboard event UI |
| test_event_currenttarget.html | ready | event UI layout |
| test_event_features.html | ready | event features UI |
| test_event_features_p2.html | ready | event features UI p2 |
| test_fetch_json_basic.html | ready+fetch | t10_fetch_json |
| test_fetch_reject.html | ready+fetch | t10_fetch_reject |
| test_fetch_text_basic.html | ready+fetch | t10_fetch_text |
| test_fetch_timer_order.html | dropped | Promise microtask ordering |
| test_font_weight_mapping.html | ready | static font-weight CSS |
| test_formdata_api.html | dropped | FormData object API (T20) |
| test_formdata_name_serialization.html | ready | form field layout |
| test_form_validation_range.html | ready | range input layout |
| test_img_events_simple.html | ready | image events UI |
| test_img_load_error.html | ready | image load/error UI |
| test_inert_attribute.html | ready | inert section layout |
| test_optgroup_basic.html | ready | optgroup visual layout |
| test_pointer_properties.html | ready | pointer events UI |
| test_porffor_clipboard_cn.html | ready | clipboard CN UI |
| test_scene_compiler.html | ready | scene compiler static layout |
| test_scene_graph.html | dropped | dong.scene API |
| test_scroll_autobottom_blocks.html | ready | scroll container layout |
| test_scroll_top_clamp_scrollheight.html | ready | scroll clamp layout |
| test_select_option_click.html | ready | select dropdown layout |
| test_sticky_scroll_top_autoscroll.html | ready | sticky scroll layout |

## T10 fetch 模块

- `t10_fetch_json` / `t10_fetch_text` / `t10_fetch_reject`
- 各测试配套 `.mf.json`（`frames: 2`）等待 fetch 回调后截图
