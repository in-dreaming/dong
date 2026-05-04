// dock_split.c - Pure C split tree data structure (no SDL dependency)
//
// Manages binary split tree layout for the docking system.
// Each node is either a LEAF (contains pane indices as tabs) or a SPLIT
// (horizontal/vertical with two children and a ratio).

#include "dock_internal.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// =============================================================================
// Node allocation
// =============================================================================

dock_node_t* dock_node_new_leaf(int pane_index) {
    dock_node_t* node = (dock_node_t*)calloc(1, sizeof(dock_node_t));
    if (!node) return NULL;

    node->type = DOCK_NODE_LEAF;
    node->ratio = 0.5f;
    node->parent = NULL;

    node->pane_cap = 4;
    node->pane_indices = (int*)malloc(sizeof(int) * node->pane_cap);
    if (!node->pane_indices) {
        free(node);
        return NULL;
    }

    node->pane_indices[0] = pane_index;
    node->pane_count = 1;
    node->active_tab = 0;

    return node;
}

void dock_node_free(dock_node_t* node) {
    if (!node) return;

    if (node->type == DOCK_NODE_LEAF) {
        free(node->pane_indices);
    } else {
        dock_node_free(node->children[0]);
        dock_node_free(node->children[1]);
    }

    free(node);
}

// =============================================================================
// Split operation
// =============================================================================

dock_node_t* dock_node_split(dock_node_t* leaf, dong_dock_edge_t edge,
                              int new_pane_index, float ratio) {
    if (!leaf || leaf->type != DOCK_NODE_LEAF) return NULL;
    if (edge == DONG_DOCK_TAB) return NULL; // Use dock_node_add_tab instead

    if (ratio <= 0.0f || ratio >= 1.0f) ratio = 0.5f;

    // Create new leaf for the new pane
    dock_node_t* new_leaf = dock_node_new_leaf(new_pane_index);
    if (!new_leaf) return NULL;

    // Create a new leaf to hold the existing pane data
    dock_node_t* existing_leaf = (dock_node_t*)calloc(1, sizeof(dock_node_t));
    if (!existing_leaf) {
        dock_node_free(new_leaf);
        return NULL;
    }

    // Move leaf's pane data to existing_leaf
    existing_leaf->type = DOCK_NODE_LEAF;
    existing_leaf->ratio = 0.5f;
    existing_leaf->pane_indices = leaf->pane_indices;
    existing_leaf->pane_count = leaf->pane_count;
    existing_leaf->pane_cap = leaf->pane_cap;
    existing_leaf->active_tab = leaf->active_tab;

    // Convert the original leaf into a split node
    // (preserving its position in the tree via parent pointer)
    switch (edge) {
        case DONG_DOCK_LEFT:
            leaf->type = DOCK_NODE_SPLIT_H;
            leaf->ratio = 1.0f - ratio; // new pane gets (1-ratio) on left
            leaf->children[0] = new_leaf;
            leaf->children[1] = existing_leaf;
            break;
        case DONG_DOCK_RIGHT:
            leaf->type = DOCK_NODE_SPLIT_H;
            leaf->ratio = ratio; // existing keeps ratio on left
            leaf->children[0] = existing_leaf;
            leaf->children[1] = new_leaf;
            break;
        case DONG_DOCK_TOP:
            leaf->type = DOCK_NODE_SPLIT_V;
            leaf->ratio = 1.0f - ratio;
            leaf->children[0] = new_leaf;
            leaf->children[1] = existing_leaf;
            break;
        case DONG_DOCK_BOTTOM:
            leaf->type = DOCK_NODE_SPLIT_V;
            leaf->ratio = ratio;
            leaf->children[0] = existing_leaf;
            leaf->children[1] = new_leaf;
            break;
        default:
            // Should not happen
            free(existing_leaf);
            dock_node_free(new_leaf);
            return NULL;
    }

    existing_leaf->parent = leaf;
    new_leaf->parent = leaf;

    return new_leaf;
}

// =============================================================================
// Split at root level (wraps entire subtree in a new split)
// =============================================================================

dock_node_t* dock_node_split_root(dock_node_t** root_ptr, dong_dock_edge_t edge,
                                   int new_pane_index, float ratio) {
    if (!root_ptr || !*root_ptr) return NULL;
    if (edge == DONG_DOCK_TAB) return NULL;
    if (ratio <= 0.0f || ratio >= 1.0f) ratio = 0.5f;

    dock_node_t* old_root = *root_ptr;

    // If root is a single leaf, just use normal split
    if (old_root->type == DOCK_NODE_LEAF) {
        return dock_node_split(old_root, edge, new_pane_index, ratio);
    }

    // Create new leaf for the new pane
    dock_node_t* new_leaf = dock_node_new_leaf(new_pane_index);
    if (!new_leaf) return NULL;

    // Create new split node to become the new root
    dock_node_t* new_split = (dock_node_t*)calloc(1, sizeof(dock_node_t));
    if (!new_split) {
        dock_node_free(new_leaf);
        return NULL;
    }

    switch (edge) {
        case DONG_DOCK_LEFT:
            new_split->type = DOCK_NODE_SPLIT_H;
            new_split->ratio = 1.0f - ratio;
            new_split->children[0] = new_leaf;
            new_split->children[1] = old_root;
            break;
        case DONG_DOCK_RIGHT:
            new_split->type = DOCK_NODE_SPLIT_H;
            new_split->ratio = ratio;
            new_split->children[0] = old_root;
            new_split->children[1] = new_leaf;
            break;
        case DONG_DOCK_TOP:
            new_split->type = DOCK_NODE_SPLIT_V;
            new_split->ratio = 1.0f - ratio;
            new_split->children[0] = new_leaf;
            new_split->children[1] = old_root;
            break;
        case DONG_DOCK_BOTTOM:
            new_split->type = DOCK_NODE_SPLIT_V;
            new_split->ratio = ratio;
            new_split->children[0] = old_root;
            new_split->children[1] = new_leaf;
            break;
        default:
            free(new_split);
            dock_node_free(new_leaf);
            return NULL;
    }

    new_split->parent = NULL;
    old_root->parent = new_split;
    new_leaf->parent = new_split;

    *root_ptr = new_split;
    return new_leaf;
}

// =============================================================================
// Remove pane from leaf
// =============================================================================

// Returns 1 if the node was collapsed (caller should handle), 0 otherwise.
int dock_node_remove_pane(dock_node_t* node, int pane_index) {
    if (!node) return 0;

    // Find the leaf containing this pane
    dock_node_t* leaf = dock_node_find_leaf(node, pane_index);
    if (!leaf) return 0;

    // Remove pane from the tab list
    int found = 0;
    for (int i = 0; i < leaf->pane_count; i++) {
        if (leaf->pane_indices[i] == pane_index) {
            // Shift remaining tabs down
            for (int j = i; j < leaf->pane_count - 1; j++) {
                leaf->pane_indices[j] = leaf->pane_indices[j + 1];
            }
            leaf->pane_count--;
            found = 1;
            break;
        }
    }

    if (!found) return 0;

    // Fix active_tab if needed
    if (leaf->active_tab >= leaf->pane_count && leaf->pane_count > 0) {
        leaf->active_tab = leaf->pane_count - 1;
    }

    // If leaf still has panes, nothing more to do
    if (leaf->pane_count > 0) return 0;

    // Leaf is empty - collapse: promote sibling to parent's position
    dock_node_t* parent = leaf->parent;
    if (!parent) {
        // This is the root and it's now empty - caller handles this
        return 1;
    }

    // Determine which child is the leaf and which is the sibling
    int leaf_idx = (parent->children[0] == leaf) ? 0 : 1;
    dock_node_t* sibling = parent->children[1 - leaf_idx];

    // Copy sibling's data into parent (promote sibling)
    parent->type = sibling->type;
    parent->ratio = sibling->ratio;

    if (sibling->type == DOCK_NODE_LEAF) {
        parent->pane_indices = sibling->pane_indices;
        parent->pane_count = sibling->pane_count;
        parent->pane_cap = sibling->pane_cap;
        parent->active_tab = sibling->active_tab;
    } else {
        parent->children[0] = sibling->children[0];
        parent->children[1] = sibling->children[1];
        if (parent->children[0]) parent->children[0]->parent = parent;
        if (parent->children[1]) parent->children[1]->parent = parent;
    }

    // Free the now-detached sibling shell (don't free its children/panes, they moved)
    // Reset sibling fields to avoid double-free
    if (sibling->type == DOCK_NODE_LEAF) {
        sibling->pane_indices = NULL;
    } else {
        sibling->children[0] = NULL;
        sibling->children[1] = NULL;
    }
    free(sibling);

    // Free the empty leaf
    free(leaf->pane_indices);
    free(leaf);

    return 0;
}

// =============================================================================
// Add tab to leaf
// =============================================================================

void dock_node_add_tab(dock_node_t* leaf, int pane_index) {
    if (!leaf || leaf->type != DOCK_NODE_LEAF) return;

    // Grow array if needed
    if (leaf->pane_count >= leaf->pane_cap) {
        int new_cap = leaf->pane_cap * 2;
        int* new_arr = (int*)realloc(leaf->pane_indices, sizeof(int) * new_cap);
        if (!new_arr) return;
        leaf->pane_indices = new_arr;
        leaf->pane_cap = new_cap;
    }

    leaf->pane_indices[leaf->pane_count] = pane_index;
    leaf->pane_count++;
    leaf->active_tab = leaf->pane_count - 1; // Activate newly added tab
}

// =============================================================================
// Layout computation
// =============================================================================

void dock_node_layout(dock_node_t* node, int32_t x, int32_t y, uint32_t w, uint32_t h) {
    if (!node) return;

    node->x = x;
    node->y = y;
    node->w = w;
    node->h = h;

    if (node->type == DOCK_NODE_LEAF) {
        return;
    }

    if (node->type == DOCK_NODE_SPLIT_H) {
        uint32_t left_w = (uint32_t)(w * node->ratio);
        // Enforce minimum pane size
        if (left_w < DOCK_MIN_PANE_SIZE && w > 2 * DOCK_MIN_PANE_SIZE)
            left_w = DOCK_MIN_PANE_SIZE;
        if (w > left_w && (w - left_w) < DOCK_MIN_PANE_SIZE && w > 2 * DOCK_MIN_PANE_SIZE)
            left_w = w - DOCK_MIN_PANE_SIZE;
        if (left_w < 1) left_w = 1;
        if (left_w > w - 1) left_w = w - 1;
        uint32_t right_w = w - left_w;

        dock_node_layout(node->children[0], x, y, left_w, h);
        dock_node_layout(node->children[1], x + (int32_t)left_w, y, right_w, h);
    } else { // SPLIT_V
        uint32_t top_h = (uint32_t)(h * node->ratio);
        // Enforce minimum pane size
        if (top_h < DOCK_MIN_PANE_SIZE && h > 2 * DOCK_MIN_PANE_SIZE)
            top_h = DOCK_MIN_PANE_SIZE;
        if (h > top_h && (h - top_h) < DOCK_MIN_PANE_SIZE && h > 2 * DOCK_MIN_PANE_SIZE)
            top_h = h - DOCK_MIN_PANE_SIZE;
        if (top_h < 1) top_h = 1;
        if (top_h > h - 1) top_h = h - 1;
        uint32_t bottom_h = h - top_h;

        dock_node_layout(node->children[0], x, y, w, top_h);
        dock_node_layout(node->children[1], x, y + (int32_t)top_h, w, bottom_h);
    }
}

// =============================================================================
// Find leaf containing a pane
// =============================================================================

dock_node_t* dock_node_find_leaf(dock_node_t* root, int pane_index) {
    if (!root) return NULL;

    if (root->type == DOCK_NODE_LEAF) {
        for (int i = 0; i < root->pane_count; i++) {
            if (root->pane_indices[i] == pane_index) {
                return root;
            }
        }
        return NULL;
    }

    dock_node_t* found = dock_node_find_leaf(root->children[0], pane_index);
    if (found) return found;
    return dock_node_find_leaf(root->children[1], pane_index);
}

// =============================================================================
// Hit test - find leaf node at mouse position
// =============================================================================

dock_node_t* dock_node_hit_test(dock_node_t* root, int32_t mx, int32_t my) {
    if (!root) return NULL;

    // Check if point is within this node's bounds
    if (mx < root->x || mx >= root->x + (int32_t)root->w ||
        my < root->y || my >= root->y + (int32_t)root->h) {
        return NULL;
    }

    if (root->type == DOCK_NODE_LEAF) {
        return root;
    }

    // Recurse into children
    dock_node_t* hit = dock_node_hit_test(root->children[0], mx, my);
    if (hit) return hit;
    return dock_node_hit_test(root->children[1], mx, my);
}

// =============================================================================
// Hit test divider - find split node whose divider is at mouse position
// =============================================================================

dock_node_t* dock_node_hit_test_divider(dock_node_t* root, int32_t mx, int32_t my) {
    if (!root) return NULL;
    if (root->type == DOCK_NODE_LEAF) return NULL;

    // Check children first (deeper dividers have priority)
    dock_node_t* hit = dock_node_hit_test_divider(root->children[0], mx, my);
    if (hit) return hit;
    hit = dock_node_hit_test_divider(root->children[1], mx, my);
    if (hit) return hit;

    // Check this node's divider
    if (root->type == DOCK_NODE_SPLIT_H) {
        // Vertical divider line at children[0]->x + children[0]->w
        int32_t divider_x = root->children[0]->x + (int32_t)root->children[0]->w;
        if (mx >= divider_x - DOCK_DIVIDER_HALF_W &&
            mx <= divider_x + DOCK_DIVIDER_HALF_W &&
            my >= root->y && my < root->y + (int32_t)root->h) {
            return root;
        }
    } else { // SPLIT_V
        // Horizontal divider line at children[0]->y + children[0]->h
        int32_t divider_y = root->children[0]->y + (int32_t)root->children[0]->h;
        if (my >= divider_y - DOCK_DIVIDER_HALF_W &&
            my <= divider_y + DOCK_DIVIDER_HALF_W &&
            mx >= root->x && mx < root->x + (int32_t)root->w) {
            return root;
        }
    }

    return NULL;
}
