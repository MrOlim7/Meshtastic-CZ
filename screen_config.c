#include "app.h"
#include <stdio.h>
#include <string.h>

// ─────────────────────────────────────────────────────────────────────────────
//  screen_config.c  —  Paramètres du nœud Meshtastic
//
//  Layout 320×170 :
//  ┌────────────────────────────────────────────────────────┐  18px  header
//  │ ⚙ Parametres                          [M] [N] Retour  │
//  ├────────────────────────────────────────────────────────┤
//  │  Nom du nœud  :  MonNoeud                             │
//  │  Canal        :  LongFast                             │
//  │  Région       :  EU_868                               │
//  │  Daemon       :  ● Connecté  (localhost:4403)         │
//  │  Firmware     :  meshtasticd 2.5.x                    │
//  │                                                        │
//  │  [M] Messages   [N] Noeuds                            │
//  └────────────────────────────────────────────────────────┘
//
//  En attendant le vrai device, les champs sont en lecture seule.
//  Plus tard on pourra les rendre éditables avec le clavier physique.
// ─────────────────────────────────────────────────────────────────────────────

static lv_obj_t *_root   = NULL;
static lv_obj_t *_header = NULL;
static lv_obj_t *_body   = NULL;

static lv_style_t _style_root;
static lv_style_t _style_header;
static lv_style_t _style_body;
static lv_style_t _style_row_key;
static lv_style_t _style_row_val;
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
    lv_style_set_text_color(&_style_header, hex(COL_YELLOW));
    lv_style_set_text_font(&_style_header, &lv_font_montserrat_10);
    lv_style_set_border_width(&_style_header, 0);
    lv_style_set_border_side(&_style_header, LV_BORDER_SIDE_BOTTOM);
    lv_style_set_border_color(&_style_header, hex(COL_BORDER));
    lv_style_set_pad_left(&_style_header, 6);
    lv_style_set_pad_top(&_style_header, 4);

    lv_style_init(&_style_body);
    lv_style_set_bg_color(&_style_body, hex(COL_BG));
    lv_style_set_bg_opa(&_style_body, LV_OPA_COVER);
    lv_style_set_border_width(&_style_body, 0);
    lv_style_set_pad_left(&_style_body, 10);
    lv_style_set_pad_top(&_style_body, 6);
    lv_style_set_pad_row(&_style_body, 5);

    lv_style_init(&_style_row_key);
    lv_style_set_text_color(&_style_row_key, hex(COL_GRAY));
    lv_style_set_text_font(&_style_row_key, &lv_font_montserrat_10);
    lv_style_set_bg_opa(&_style_row_key, LV_OPA_TRANSP);
    lv_style_set_border_width(&_style_row_key, 0);
    lv_style_set_pad_all(&_style_row_key, 0);

    lv_style_init(&_style_row_val);
    lv_style_set_text_color(&_style_row_val, hex(COL_WHITE));
    lv_style_set_text_font(&_style_row_val, &lv_font_montserrat_10);
    lv_style_set_bg_opa(&_style_row_val, LV_OPA_TRANSP);
    lv_style_set_border_width(&_style_row_val, 0);
    lv_style_set_pad_all(&_style_row_val, 0);
}

// Ajoute une ligne "clé  :  valeur" dans le body
static void _add_row(const char *key, const char *value, uint32_t val_color) {
    char line[128];
    snprintf(line, sizeof(line), "%-16s  %s", key, value);

    lv_obj_t *label = lv_label_create(_body);
    lv_label_set_text(label, line);
    lv_obj_set_width(label, CZ_W - 20);
    lv_obj_add_style(label, &_style_row_val, 0);

    // Override couleur de la valeur si demandé
    if (val_color != COL_WHITE) {
        lv_style_t *s = lv_malloc(sizeof(lv_style_t));
        lv_style_init(s);
        lv_style_set_text_color(s, hex(val_color));
        lv_obj_add_style(label, s, 0);
    }
}

lv_obj_t *screen_config_create(lv_obj_t *parent) {
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
    lv_label_set_text(_header, "Parametres du noeud       [M] Messages  [N] Noeuds");

    // Body
    _body = lv_obj_create(_root);
    lv_obj_set_size(_body, CZ_W, CZ_H - STATUSBAR_H);
    lv_obj_set_pos(_body, 0, STATUSBAR_H);
    lv_obj_add_style(_body, &_style_body, 0);
    lv_obj_set_flex_flow(_body, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scrollbar_mode(_body, LV_SCROLLBAR_MODE_OFF);

    // Remplir les lignes
    _add_row("Nom du noeud :", g_state.my_name,  COL_WHITE);
    _add_row("Canal        :", g_state.channel,  COL_BLUE);
    _add_row("Region       :", g_state.region,   COL_BLUE);
    _add_row("Daemon       :",
             g_state.daemon_connected
               ? "● Connecte  (localhost:4403)"
               : "○ Non connecte",
             g_state.daemon_connected ? COL_GREEN : COL_RED);
    _add_row("Firmware     :", "meshtasticd 2.5.x", COL_GRAY);
    _add_row("Radio        :", "SX1262  EU_868  +22dBm", COL_GRAY);

    return _root;
}

void screen_config_on_key(uint32_t key) {
    // Pour l'instant : lecture seule
    // TODO : permettre d'éditer les champs avec le clavier physique
    (void)key;
}
