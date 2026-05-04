#include "dock_internal.h"

DONG_APPCORE_API dong_dock_pane_t* dong_dock_add_pane(
    dong_dock_t* dock, const dong_dock_pane_config_t* config)
{
    return dock_add_pane_impl(dock, config);
}

DONG_APPCORE_API dong_dock_pane_t* dong_dock_split(
    dong_dock_t* dock, dong_dock_pane_t* neighbor,
    dong_dock_edge_t edge, float ratio,
    const dong_dock_pane_config_t* config)
{
    return dock_split_impl(dock, neighbor, edge, ratio, config);
}
