#include "app.h"
#include <stdio.h>
#include <string.h>

// ─────────────────────────────────────────────────────────────────────────────
//  screen_nodes.c  —  Liste des nœuds Meshtastic visibles
//
//  Layout 320×170 :
//  ┌────────────────────────────────────────────────────────┐  18px  header
//  │ 📡 Nœuds du réseau  (3)              [M]Messages [S]  │
//  ├────────────────────────────────────────────────────────┤
//  │  ●  Alice      -87 dBm   GPS ✓   vu il y a 2min       │  152px scrollable
//  │  ●  Bob       -102 dBm   GPS ✓   vu il y a 5min       │
//  │  ◌  Relay-1   -118 dBm   GPS ✗   hors ligne           │
//  └────────────────────────────────────────────────────────┘
//
//  Touches :
//   Fn+M ou Backspace → revenir à Messages
//   Fn+S             → Config
// ─────────────────────────────────────────────────────────────────────────────

static lv_obj_t *_root        = NULL;
static lv_obj_t *_header      = NULL;
static lv_obj_t *_node_list   = NULL;

static lv_style_t _style_root;
static lv_style_t _style_header;
static lv_style_t _style_list;
static lv_style_t _style_row;
static bool       _styles_init = false;

static void _init_styles(void) {
    if (_styles_init) return;
    _styles_init = true;

    lv_style_init(&_style_root);
    lv_style_set_bg_color(&_style_root, hex(COL_BG));
    lv_style_set_bg_opa(&_style_root, LV_OPA_COVER);
    lv_style_set_border_width(&_style_root, 0);
    lv_style_set_pad_all(&_style_root, 0);

    lv_style_init(&_style_header);
    lv_style_set_bg_color(&_style_header, hex(COL_SURFACE));
    lv_style_set_bg_opa(&_style_header, LV_OPA_COVER);
    lv_style_set_text_color(&_style_header, hex(COL_BLUE));
    lv_style_set_text_font(&_style_header, &lv_font_montserrat_10);
    lv_style_set_border_width(&_style_header, 0);
    lv_style_set_border_side(&_style_header, LV_BORDER_SIDE_BOTTOM);
    lv_style_set_border_color(&_style_header, hex(COL_BORDER));
    lv_style_set_pad_left(&_style_header, 6);
    lv_style_set_pad_top(&_style_header, 4);

    lv_style_init(&_style_list);
    lv_style_set_bg_color(&_style_list, hex(COL_BG));
    lv_style_set_bg_opa(&_style_list, LV_OPA_COVER);
    lv_style_set_border_width(&_style_list, 0);
    lv_style_set_pad_left(&_style_list, 6);
    lv_style_set_pad_top(&_style_list, 4);
    lv_style_set_pad_row(&_style_list, 3);

    lv_style_init(&_style_row);
    lv_style_set_text_font(&_style_row, &lv_font_montserrat_10);
    lv_style_set_bg_opa(&_style_row, LV_OPA_TRANSP);
    lv_style_set_border_width(&_style_row, 0);
    lv_style_set_pad_all(&_style_row, 0);
}

lv_obj_t *screen_nodes_create(lv_obj_t *parent) {
    _init_styles();

    _root = lv_obj_create(parent);
    lv_obj_set_size(_root, CZ_W, CZ_H);
    lv_obj_set_pos(_root, 0, 0);
    lv_obj_add_style(_root, &_style_root, 0);
    lv_obj_set_scrollbar_mode(_root, LV_SCROLLBAR_MODE_OFF);

    // Header
    _header = lv_label_create(_root);
    lv_obj_set_size(_header, CZ_W, STATUSBAR_H);
    lv_obj_set_pos(_header, 0, 0);
    lv_obj_add_style(_header, &_style_header, 0);

    // Liste scrollable
    _node_list = lv_obj_create(_root);
    lv_obj_set_size(_node_list, CZ_W, CZ_H - STATUSBAR_H);
    lv_obj_set_pos(_node_list, 0, STATUSBAR_H);
    lv_obj_add_style(_node_list, &_style_list, 0);
    lv_obj_set_flex_flow(_node_list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scroll_dir(_node_list, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(_node_list, LV_SCROLLBAR_MODE_OFF);

    screen_nodes_refresh();
    return _root;
}

void screen_nodes_refresh(void) {
    if (!_node_list) return;

    // Header
    char hdr[64];
    snprintf(hdr, sizeof(hdr),
             "Noeuds du reseau (%d)   [M] Messages  [S] Config",
             g_state.node_count);
    lv_label_set_text(_header, hdr);

    // Reconstruire les lignes
    lv_obj_clean(_node_list);

    for (int i = 0; i < g_state.node_count; i++) {
        const MeshNode *n = &g_state.nodes[i];

        char row[128];
        snprintf(row, sizeof(row),
                 "%s  %-12s  %4d dBm   GPS %s   %s",
                 n->online ? "●" : "◌",
                 n->name,
                 n->rssi,
                 n->has_gps ? "oui" : "non",
                 n->online  ? "en ligne" : "hors ligne");

        lv_obj_t *label = lv_label_create(_node_list);
        lv_label_set_text(label, row);
        lv_obj_add_style(label, &_style_row, 0);
        lv_obj_set_width(label, CZ_W - 12);

        // Couleur selon état
        lv_style_t *col_style = lv_malloc(sizeof(lv_style_t));
        lv_style_init(col_style);
        if (n->online) {
            lv_style_set_text_color(col_style,
                n->rssi >= -100 ? hex(COL_GREEN) : hex(COL_YELLOW));
        } else {
            lv_style_set_text_color(col_style, hex(COL_GRAY));
        }
        lv_obj_add_style(label, col_style, 0);
    }

    // Message si aucun nœud
    if (g_state.node_count == 0) {
        lv_obj_t *empty = lv_label_create(_node_list);
        lv_label_set_text(empty, "Aucun noeud visible...\nVérifiez le module LoRa.");
        lv_obj_add_style(empty, &_style_row, 0);
        lv_style_t *s = lv_malloc(sizeof(lv_style_t));
        lv_style_init(s);
        lv_style_set_text_color(s, hex(COL_GRAY));
        lv_obj_add_style(empty, s, 0);
    }
}

void screen_nodes_on_key(uint32_t key) {
    if (key == LV_KEY_UP) {
        lv_obj_scroll_by(_node_list, 0, -20, LV_ANIM_ON);
    } else if (key == LV_KEY_DOWN) {
        lv_obj_scroll_by(_node_list, 0, 20, LV_ANIM_ON);
    }
    // La navigation entre écrans est gérée dans main.c
}
