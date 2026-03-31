# Baseline Render Statistics

**Date**: 2026-03-31
**Build**: Release (clang-cl)
**Resolution**: 800x600
**Purpose**: Pre-optimization baseline for uber-quad pipeline refactoring

## Test Results

| Page | draws | rect | round | shadow | grad | img | text | clips | layers | total cmds | batches |
|------|-------|------|-------|--------|------|-----|------|-------|--------|-----------|---------|
| game_ui2 | 68 | 22 | 18 | 0 | 0 | 0 | 28 | 0 | 0 | 72 | 4 |
| test_style_render | 13 | 5 | 0 | 0 | 0 | 0 | 8 | 0 | 0 | 17 | 5 |
| test_list_markers_basic | 17 | 2 | 0 | 0 | 0 | 0 | 15 | 0 | 0 | 21 | 4 |
| transform_test | 22 | 2 | 10 | 0 | 0 | 0 | 10 | 0 | 7 | 40 | 3 |
| test_select_keyboard | 91 | 34 | 16 | 0 | 0 | 0 | 41 | 1 | 1 | 99 | 5 |
| test_aspect_ratio | 7 | 5 | 0 | 0 | 0 | 0 | 2 | 0 | 0 | 11 | 3 |
| test_flow_root_margin | 6 | 5 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 10 | 2 |
| text_shadow_test | 20 | 2 | 9 | 4 | 0 | 0 | 5 | 0 | 0 | 24 | 4 |

## Observations

1. **Dominant primitive types**: rect and text are the most common draws.
2. **Pipeline switches per frame**: Currently each draw command triggers a pipeline bind. With 68-91 draws for complex pages, that's 68-91 potential pipeline switches.
3. **Batch count** (sort-key based grouping): 2-5 batches, much lower than draw count, showing high potential for batching.
4. **Isolated layers**: `transform_test` has 7 layers (each forces render pass break).
5. **Clips**: Minimal clip usage in test pages. `test_select_keyboard` has 1 clip.
6. **No gradient or image draws** in these test pages — need separate tests for those.

## After Uber Quad Pipeline (2026-03-31)

With `DONG_USE_UBER_QUAD=1`, rect/round/shadow/gradient unified into single pipeline:

| Page | uber_draws | text | total | pipeline switches | pixel diff |
|------|-----------|------|-------|-------------------|------------|
| game_ui2 | 40 | 28 | 68 | 1 (uber) | 0 |
| text_shadow_test | 15 | 5 | 20 | 1 | 0 |
| transform_test | 12 | 10 | 22 | 1 | 0 |
| test_select_keyboard | 50 | 41 | 91 | 1 | 0 |
| test_style_render | 5 | 8 | 13 | 1 | 0 |
| test_list_markers_basic | 2 | 15 | 17 | 1 | 0 |

**Key improvement**: Pipeline switches for quad draws reduced from N (one per type change) to 1 (single uber_quad_pipeline).
Draw call count unchanged (still per-quad), but pipeline bind overhead eliminated.

## Key Metrics

- **Baseline**: Each draw = potentially different pipeline bind + 1 draw call
- **Uber quad**: All rect/round/shadow/gradient share 1 pipeline, 1 bind per frame
- **Next step**: True instanced rendering (pre-upload instance buffer before render pass)
