# 宿主图形层接入指南

## Tier-1 Standalone（dong_app）

应用层创建 `GpuGraph` → `dong_ui_graph_add_passes` → `gpuGraphExecute`。库内 `dong_gpu_driver_set_embedded_mode(driver, 0)` 可启用 standalone 执行路径。

## Tier-2 GPU 注入（推荐）

```c
GpuGraph graph;
gpuGraphCreate(device, &graph);

/* 宿主场景 pass */
scene_add_passes(graph);

DongUiGraphContext ui = {0};
ui.host_graph = graph;
ui.device = device;
ui.driver = dong_driver;
ui.composite_target = scene_color_rt;
ui.composite_mode = DONG_UI_COMPOSITE_OVERLAY;
dong_ui_graph_prepare(&ui, command_list);
dong_ui_graph_add_passes(&ui);

post_add_passes(graph, dong_ui_graph_get_output(&ui));
gpuGraphCompile(graph);
gpuGraphExecute(graph, graphics_queue);
```

## Tier-3 DrawList ABI

Unity/UE 等引擎 UI 管线：导出稳定 `DongDrawList` C ABI，无 GpuGraph。见 [host-embedding-design.md](host-embedding-design.md)。

## 纹理压缩

所有 ASTC/BCN 经 [`gpu_texture_compress`](https://github.com/in-dreaming/gpu_texture_compress)：

```c
DongGtcContext* gtc = dong_gtc_create(sdl_or_gpu_device, backend_kind);
dong_gtc_encode_rgba(gtc, &rgba_image, DONG_IMAGE_FORMAT_BC7, &opts, &out);
```

运行时烘焙：`dong_texture_bake_create` → `dong_texture_bake_add_layer` → `dong_texture_bake_submit`。

GTC compute pass 可通过 `dong_gtc_graph_add_passes` 插入宿主 graph（与 UI 解耦）。

## HostView 双向嵌入

```c
dong_ui_graph_set_host_view_callback(&ui, my_host_view_draw, userdata);
```

回调在 `Dong.UIMain` pass 内触发，scope 含 clip/transform/opacity。

## 示例

[`dong/examples/gpu_ui_inject/main.c`](../../dong/examples/gpu_ui_inject/main.c)（Tier-2 骨架）。
