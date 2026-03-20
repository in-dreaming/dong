

https://mccloskeybr.com/articles/font_rendering.html

https://github.com/EricLengyel/Slug


## Architecture
```
API (dong.h) → EngineView → Painter → DisplayList → GPU IR → SDL Backend
                  │                      │                        │
                  └─TextRendererMode     └─text_renderer_mode     └─TextRendererSelector
                                                                    ├─ MSDF (existing, default)
                                                                    └─ Slug (new, stub shader)
```